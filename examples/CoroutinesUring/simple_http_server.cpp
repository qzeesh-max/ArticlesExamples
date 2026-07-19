#include "async_io_uring_task.hpp"
#include "io_uring_coroutine_helpers.hpp"
#include <arpa/inet.h>
#include <getopt.h>
#include <limits.h>
#include <netinet/in.h>
#include <sstream>
#include <sys/socket.h>

struct option long_options[] = {{"basedir", required_argument, 0, 0},
                                {"port", required_argument, 0, 0},
                                {0, 0, 0, 0}};

std::string basedir = "./";

const char *get_content_type(const std::string &name) {
  auto dotPos = name.rfind('.');

  if (dotPos == std::string::npos)
    return "text/plain";

  static const std::unordered_map<std::string, const char *> content_types{
      {".html", "text/html"},     {".jpeg", "image/jpeg"},
      {".gif", "image/gif"},      {".png", "image/png"},
      {".svg", "image/svg+xml"},  {".pdf", "application/pdf"},
      {".xml", "text/xml"},       {".css", "text/css"},
      {".js", "text/javascript"}, {".json", "application/json"},
  };

  if (auto it = content_types.find(name.substr(dotPos));
      it != content_types.end())
    return it->second;

  return "text/plain";
}

std::stringstream create_header(int replyCode, const char *replyName,
                                const char *contentType, size_t contentLength) {
  std::stringstream stm;

  stm << "HTTP/1.1 " << replyCode << " " << replyName
      << "\r\nContent-Type: " << contentType
      << "\r\nContent-Length:" << contentLength << "\r\n\r\n";

  return stm;
}

async_io_uring_task sendResponse(async_io_uring_task_allocator &,
                                 async_file_descriptor &desc, int replyCode,
                                 const char *replyName, const char *contentType,
                                 const std::string &content) {
  auto stm = create_header(replyCode, replyName, contentType, content.size());

  stm << content;

  auto response = stm.str();

  int bytes_sent =
      co_await desc.async_send(response.c_str(), response.size(), 0);

  if (bytes_sent < 0) {
    std::cerr << "Send error: " << strerror(-bytes_sent) << std::endl;
    co_await desc.async_close();
    co_return;
  } else {
    std::cout << "Sent " << bytes_sent << " bytes to client " << desc.handle()
              << std::endl;
  }

  // signal we are done writing, so the otherside also acknowledges that they
  // are also done writing
  co_await desc.async_shutdown(SHUT_WR);

  char buffer[1024];

  // wait for proper close, discarding data, normally unexpected data can be
  // used to log an error
  while (co_await desc.async_recv(buffer, sizeof(buffer) - 1, 0) > 0)
    ;

  co_await desc.async_close();
  std::cout << "Client " << desc.handle() << " connection closed." << std::endl;
  co_return;
}

async_io_uring_task handle_client(async_io_uring_task_allocator &allocator,
                                  async_io_uring_tasks_list &tasks,
                                  io_uring_context &ring_context,
                                  int client_fd) {
  async_file_descriptor desc(ring_context, client_fd);
  bool validHttp = false;
  std::stringstream readstream;
  std::string first_line;

  std::cout << "Handling client: " << client_fd << std::endl;

  while (!validHttp) {
    char buffer[1024];
    int bytes_read = co_await desc.async_recv(buffer, sizeof(buffer) - 1, 0);

    if (bytes_read < 0) {
      std::cerr << "Recv error or client disconnected: "
                << strerror(-bytes_read) << std::endl;
      co_await desc.async_close();
      co_return;
    }
    if (bytes_read == 0) {
      std::cout << "Client " << client_fd << " disconnected." << std::endl;
      co_await desc.async_close();
      co_return;
    }

    readstream.write(buffer, bytes_read);

    readstream.clear();
    readstream.seekg(0, std::ios_base::beg);

    std::string line;

    std::getline(readstream, line);

    if (line.size() < 4)
      continue;

    if (!line.starts_with("GET ")) {
      static const std::string badRequestText =
          "<html><head><title>Bad Request</title></head><body>Bad "
          "Request</body></html>";

      auto responseSender = sendResponse(allocator, desc, 400, "BadRequest",
                                         "text/html", badRequestText);

      tasks.emplace_back(std::move(responseSender));

      co_return;
    }

    first_line = line;

    while (std::getline(readstream, line)) {
      if (line.empty() || (line.size() == 1 && line[0] == '\r'))
        validHttp = true;
    }
  }

  std::string uri = first_line.substr(4);

  if (auto method_version_offset = uri.rfind(" HTTP/");
      method_version_offset != std::string::npos) {
    uri.erase(method_version_offset);
  } else {
    static const std::string badRequestText =
        "<html><head><title>Bad Request</title></head><body>Bad "
        "Request</body></html>";

    auto responseSender = sendResponse(allocator, desc, 400, "BadRequest",
                                       "text/html", badRequestText);

    tasks.emplace_back(std::move(responseSender));

    co_return;
  }

  if (auto method_data_offset = uri.find("?");
      method_data_offset != std::string::npos)
    uri.erase(method_data_offset);

  if (uri.ends_with("/"))
    uri += "index.html";

  // shutdown access to allow us to test clean shutdown
  if (uri == "/shutdown") {
    ring_context.terminate();
    co_return;
  }

  auto actual_uri = uri;

  // make it a relative path (not safe)
  uri = basedir + "/" + uri;

  char realuri[PATH_MAX];

  realpath(uri.c_str(), realuri);

  uri = realuri;

  if (!uri.starts_with(basedir)) {
    std::string errorHtml =
        "<html><head><title>403 Forbidden</title></head><body>Forbidden "
        "Access outside the tree " +
        actual_uri + "</body></html>";

    auto errorResponder =
        sendResponse(allocator, desc, 403, "Forbidden", "text/html", errorHtml);

    tasks.emplace_back(std::move(errorResponder));

    co_return;
  }

  struct statx statxbuf;

  int statResult = co_await async_statx(ring_context, uri.c_str(), 0,
                                        STATX_BASIC_STATS, &statxbuf);

  if (statResult == 0) {
    std::cout << "Found ... " << uri << std::endl;
  } else {
    std::string errorHtml =
        "<html><head><title>404 Not Found</title></head><body>Not found " +
        actual_uri + "</body></html>";

    auto errorResponder =
        sendResponse(allocator, desc, 404, "Not Found", "text/html", errorHtml);

    tasks.emplace_back(std::move(errorResponder));

    co_return;
  }

  if (S_ISDIR(statxbuf.stx_mode)) {
    std::string errorHtml =
        "<html><head><title>403 Forbidden</title></head><body>Forbidden " +
        actual_uri + "</body></html>";

    auto errorResponder =
        sendResponse(allocator, desc, 403, "Forbidden", "text/html", errorHtml);

    tasks.emplace_back(std::move(errorResponder));

    co_return;
  }

  int file = co_await async_open(ring_context, uri.c_str(), 0, 0);

  if (file < 0) {
    std::string errorHtml =
        "<html><head><title>404 Not Found</title></head><body>Error opening " +
        actual_uri + ": " + strerror(-file) + "</body></html>";

    auto errorResponder =
        sendResponse(allocator, desc, 404, "Not Found", "text/html", errorHtml);

    tasks.emplace_back(std::move(errorResponder));

    co_return;
  }

  async_file_descriptor rd(ring_context, file);

  auto stm = create_header(200, "OK", get_content_type(uri), statxbuf.stx_size);

  std::string http_response = stm.str();

  int bytes_sent = co_await desc.async_send(http_response.c_str(),
                                            http_response.length(), 0);

  if (bytes_sent < 0) {
    std::cerr << "Send error: " << strerror(-bytes_sent) << std::endl;
    co_await desc.async_close();
    co_await rd.async_close();
    co_return;
  } else {
    std::cout << "Sent " << bytes_sent << " bytes to client " << client_fd
              << std::endl;
  }

  char fileReadBuffer[4096];

  off_t position = 0;
  int readSize;

  while ((readSize = co_await rd.async_pread(
              fileReadBuffer, sizeof(fileReadBuffer), position)) > 0) {
    position += readSize;
    int bytes_sent = co_await desc.async_send(fileReadBuffer, readSize, 0);

    if (bytes_sent < 0) {
      std::cerr << "Send error: " << strerror(-bytes_sent) << std::endl;
      co_await desc.async_close();
      co_return;
    } else {
      std::cout << "Sent " << bytes_sent << " bytes to client " << client_fd
                << std::endl;
    }
  }

  co_await rd.async_close();

  // signal we are done writing, so the otherside also acknowledges that we are
  // done writing
  co_await desc.async_shutdown(SHUT_WR);

  char buffer[1024];
  // wait for proper close, discarding data
  while (co_await desc.async_recv(buffer, sizeof(buffer) - 1, 0) > 0)
    ;

  co_await desc.async_close();
  std::cout << "Client " << client_fd << " connection closed." << std::endl;
  co_return;
}

async_io_uring_task accept_loop(async_io_uring_task_allocator &allocator,
                                io_uring_context &ring_context, int server_fd) {
  async_io_uring_tasks_list tasks;
  async_file_descriptor desc(ring_context, server_fd);
  while (true) {
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);

    std::cout << "Awaiting new connection..." << std::endl;
    int client_fd = co_await desc.async_accept((struct sockaddr *)&client_addr,
                                               &client_len, 0);

    if (client_fd >= 0) {
      std::cout << "Accepted new connection with fd: " << client_fd << " from "
                << inet_ntoa(client_addr.sin_addr) << ":"
                << ntohs(client_addr.sin_port) << std::endl;

      auto handler = handle_client(allocator, tasks, ring_context, client_fd);

      tasks.emplace_back(std::move(handler));
    } else {
      std::cerr << "Accept error: " << strerror(-client_fd) << std::endl;

      if (client_fd != -EINTR && client_fd != -EAGAIN) {
        std::cerr << "Fatal accept error, stopping accept loop." << std::endl;
        break;
      }
    }

    // clean up all the tasks on each new connection
    for (auto it = tasks.begin(); it != tasks.end();) {
      if ((*it).done()) {
        it = tasks.erase(it);
      } else {
        ++it;
      }
    }
  }
  co_return;
}

int main(int argc, char **argv) {
  int port = 8080;
  int option_index;
  bool optionsProcessed = false;

  while (!optionsProcessed) {
    int this_option_optind = optind ? optind : 1;
    int option_index = 0;

    auto optResult = getopt_long(argc, argv, "", long_options, &option_index);

    switch (optResult) {
    case 0:
      switch (option_index) {
      case 0:
        if (optarg)
          basedir = optarg;
        break;
      case 1:
        if (optarg)
          port = atoi(optarg);
        break;
      }
      break;
    case -1:
      optionsProcessed = true;
      continue;
    default:
      std::cerr << argv[0] << "[--basedir <basedir>] [--port <port>]"
                << std::endl;
      std::cerr << std::endl << std::endl;
      std::cerr << "--basedir <basedir>\t  directory where the files are found"
                << std::endl;
      std::cerr << "--port <port>\t port on which the server listens on"
                << std::endl;
      return EXIT_SUCCESS;
    }
  }

  char realbase[PATH_MAX];

  realpath(basedir.c_str(), realbase);

  basedir = realbase;

  int server_fd;
  struct sockaddr_in serv_addr;

  // Create socket
  server_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (server_fd < 0) {
    perror("socket failed");
    return EXIT_FAILURE;
  }

  int opt = 1;
  if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt,
                 sizeof(opt))) {
    perror("setsockopt");
    close(server_fd);
    return EXIT_FAILURE;
  }

  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = INADDR_ANY;
  serv_addr.sin_port = htons(port);

  if (bind(server_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
    perror("bind failed");
    close(server_fd);
    return EXIT_FAILURE;
  }

  if (listen(server_fd, 128) < 0) { // backlog of 128
    perror("listen failed");
    close(server_fd);
    return EXIT_FAILURE;
  }

  std::cout << "Server listening on port " << port << std::endl;

  io_uring_context ring_context;

  async_io_uring_task_allocator allocator(1024);

  // Start the accepting coroutine
  async_io_uring_task acceptor_task =
      accept_loop(allocator, ring_context, server_fd);

  ring_context.dispatch_events();

  close(server_fd);
}
