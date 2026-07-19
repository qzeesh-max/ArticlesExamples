#include <iostream>
#include <meta>
#include <optional>
#include <sqlite3.h> // SQLite3 C API
#include <string>
#include <string_view>
#include <type_traits>
#include <vector>

namespace meta = std::meta;

// ============================================================================
// 1. Compile-Time SQL Intake via #embed
// ============================================================================
constexpr char schema_data[] = {
#embed "schema.sql"
    , '\0'};

constexpr std::string_view schema_sql{schema_data};

template <typename S> consteval auto getMembers() {
    constexpr auto info = ^^S;
    auto members = std::meta::nonstatic_data_members_of(info, std::meta::access_context::current());
    std::array<
        std::meta::info,
        std::meta::nonstatic_data_members_of(info, std::meta::access_context::current()).size()>
        rr{};

    for (size_t i = 0; i < members.size(); ++i)
        rr[i] = members[i];

    return rr;
}

template <typename S, typename Lambda> void overMembers(const S &v, Lambda &&lambda) {
    static constexpr auto members = getMembers<S>();

    template for (constexpr auto member : members) {
        lambda(std::meta::identifier_of(member), v.[:member:]);
    }
}

template <typename S, typename Lambda> void overMembers(S &v, Lambda &&lambda) {
    static constexpr auto members = getMembers<S>();

    template for (constexpr auto member : members) {
        lambda(std::meta::identifier_of(member), v.[:member:]);
    }
}

// ============================================================================
// 2. Compile-Time Schema Parser & Meta-Generator
// ============================================================================
namespace orm {
// Helper to determine C++ type reflection info from SQL type
consteval meta::info map_sql_type_to_cpp(std::string_view type_str, bool is_not_null) {
    meta::info base_type = ^^std::string; // Default to string for VARCHAR, etc.

    if (type_str.find("INT") != std::string_view::npos) {
        base_type = ^^int;
    }

    if (is_not_null) {
        return base_type;
    } else {
        // If nullable, wrap the type in std::optional at compile time
        if (base_type == ^^int)
            return ^^std::optional<int>;
        return ^^std::optional<std::string>;
    }
}

template <auto sql_, auto start, auto end, auto pos, size_t ResultSize>
consteval auto parse_schema_to_members_helper(const std::array<meta::info, ResultSize> &members) {
    constexpr std::string_view sql(sql_);
    constexpr std::string_view columns = sql.substr(start + 1, end - start - 1);

    constexpr size_t comma_pos_pre = columns.find(',', pos);
    constexpr size_t comma_pos =
        comma_pos_pre == std::string::npos ? columns.length() : comma_pos_pre;

    constexpr auto col_def_full = columns.substr(pos, comma_pos - pos);

    // Trim leading whitespace
    constexpr auto col_def = col_def_full.substr(col_def_full.find_first_not_of(" \n\r"));

    if constexpr (!col_def.empty()) {
        constexpr size_t space1 = col_def.find(' ');
        constexpr size_t space2 = col_def.find(' ', space1 + 1);

        constexpr std::string_view type_str = col_def.substr(space1 + 1, space2 - space1 - 1);
        constexpr bool is_not_null = (col_def.find("NOT NULL") != std::string_view::npos);

        constexpr meta::info cpp_type = map_sql_type_to_cpp(type_str, is_not_null);

        std::array<meta::info, ResultSize + 1> results;

        std::copy(members.begin(), members.end(), results.begin());

        results.back() = meta::data_member_spec(
            cpp_type, {.name = std::define_static_string(col_def.substr(0, space1))});

        if constexpr (comma_pos + 1 <= columns.size())
            return parse_schema_to_members_helper<sql_, start, end, comma_pos + 1>(results);
        else
            return results;
    } else
        return members;
}

// Simplified constexpr parser extracting column names and types from CREATE
// TABLE
template <auto sql_> consteval auto parse_schema_to_members() {
    constexpr std::string_view sql(sql_);

    // Find the block between '(' and ')'
    constexpr auto start = sql.find('(');
    constexpr auto end = sql.rfind(')');

    if constexpr (start == std::string_view::npos || end == std::string_view::npos)
        return std::array<meta::info, 0>();

    std::array<meta::info, 0> members;
    return parse_schema_to_members_helper<sql_, start, end, 0>(members);
}
} // namespace orm

// ============================================================================
// 3. Aggregate Injection via std::meta::define_aggregate
// ============================================================================
struct Person; // Declare incomplete type

consteval void synthesize_person() {
    static constexpr auto members = orm::parse_schema_to_members<schema_data>();

    meta::define_aggregate(^^Person, members);
}

// Execute the synthesis at namespace scope
consteval {
    synthesize_person();
}

// ============================================================================
// 4. Type Traits & SQLite Marshalling Helpers
// ============================================================================
template <typename T> struct is_optional : std::false_type {};

template <typename T> struct is_optional<std::optional<T>> : std::true_type {};

template <typename T> struct is_optional<const std::optional<T>> : std::true_type {};

template <typename T> constexpr bool is_optional_v = is_optional<T>::value;

// Direct extraction from SQLite prepared statements into reflected C++ types
template <typename T> void assign_from_sqlite(T &field, sqlite3_stmt *stmt, int col_idx) {
    if (sqlite3_column_type(stmt, col_idx) == SQLITE_NULL) {
        if constexpr (is_optional_v<T>)
            field = std::nullopt;
        return;
    }

    if constexpr (std::is_same_v<T, int>) {
        field = sqlite3_column_int(stmt, col_idx);
    } else if constexpr (std::is_same_v<T, std::string>) {
        const unsigned char *text = sqlite3_column_text(stmt, col_idx);
        if (text)
            field = reinterpret_cast<const char *>(text);
    } else if constexpr (is_optional_v<T>) {
        using InnerType = typename T::value_type;
        InnerType inner;
        assign_from_sqlite(inner, stmt, col_idx);
        field = inner;
    }
}

template <typename T> std::string to_sql_string(const T &field) {
    if constexpr (is_optional_v<T>) {
        if (!field.has_value())
            return "NULL";
        return to_sql_string(field.value());
    } else if constexpr (std::is_same_v<T, int>) {
        return std::to_string(field);
    } else {
        return "'" + field + "'"; // In production, use sqlite3_bind_* instead of
                                  // string concatenation
    }
}

// ============================================================================
// 5. Reflection-Based Database ORM Functions
// ============================================================================

template <typename T> void print_struct(const T &obj) {
    std::cout << "{ ";

    overMembers(obj, [&](const auto &name, const auto &value) {
        std::cout << name << ": ";
        auto const &val = value;

        if constexpr (is_optional_v<std::remove_reference_t<decltype(val)>>) {
            if (val)
                std::cout << *val;
            else
                std::cout << "NULL";
        } else {
            std::cout << val;
        }
        std::cout << ", ";
    });
    std::cout << "}\n";
}

template <typename T> bool insert_row(sqlite3 *db, const std::string &table_name, const T &obj) {
    std::string cols, vals;
    bool first = true;

    overMembers(obj, [&](const auto &name, const auto &value) {
        if (!first) {
            cols += ", ";
            vals += ", ";
        }
        cols += name;
        vals += to_sql_string(value);
        first = false;
    });

    std::string query = "INSERT INTO " + table_name + " (" + cols + ") VALUES (" + vals + ");";
    char *err_msg = nullptr;
    if (sqlite3_exec(db, query.c_str(), nullptr, nullptr, &err_msg) != SQLITE_OK) {
        std::cerr << "SQL Error: " << err_msg << "\n";
        sqlite3_free(err_msg);
        return false;
    }
    return true;
}

template <typename T>
std::vector<T> lookup_by_name(sqlite3 *db, const std::string &table_name,
                              const std::string &first_name) {
    std::string query = "SELECT * FROM " + table_name + " WHERE first_name = '" + first_name + "';";
    std::vector<T> results;
    sqlite3_stmt *stmt;

    if (sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            T obj;
            int col_idx = 0;

            overMembers(obj, [&](const auto &, auto &value) {
                assign_from_sqlite(value, stmt, col_idx);
                col_idx++;
            });
            results.push_back(obj);
        }
        sqlite3_finalize(stmt);
    }
    return results;
}

// ============================================================================
// 6. Application CLI Entry Point
// ============================================================================
int main() {
    sqlite3 *db;
    // Open an embedded database file (or create it if it doesn't exist)
    if (sqlite3_open("app_database.db", &db)) {
        std::cerr << "Can't open database: " << sqlite3_errmsg(db) << "\n";
        return 1;
    }

    // Deploy Table if it does not exist using the #embed SQL
    char *err_msg = nullptr;
    sqlite3_exec(db, schema_data, nullptr, nullptr, &err_msg);
    if (err_msg) {
        // Safe to ignore if table already exists, but clean up memory
        sqlite3_free(err_msg);
    }

    std::cout << "C++26 Reflection ORM Interface (SQLite Edition)\n";
    std::string cmd;

    while (true) {
        try {
            std::cout << "\n> (list, add, lookup, remove, quit): ";
            std::getline(std::cin, cmd);

            if (cmd == "quit")
                break;

            if (cmd == "list") {
                sqlite3_stmt *stmt;
                if (sqlite3_prepare_v2(db, "SELECT * FROM Person;", -1, &stmt, nullptr) ==
                    SQLITE_OK) {
                    while (sqlite3_step(stmt) == SQLITE_ROW) {
                        Person p;
                        int col_idx = 0;
                        overMembers(p, [&](const auto &, auto &value) {
                            assign_from_sqlite(value, stmt, col_idx++);
                        });
                        print_struct(p);
                    }
                    sqlite3_finalize(stmt);
                }
            } else if (cmd == "add") {
                Person p;

                overMembers(p, [&](const auto &name, auto &value) {
                    std::cout << name;

                    if constexpr (is_optional_v<std::remove_reference_t<decltype(value)>>) {
                        if constexpr (std::is_same_v<int, typename std::remove_reference_t<
                                                              decltype(value)>::value_type>) {
                            std::cout << " (0 for null): ";

                            std::string tempStr;

                            std::getline(std::cin, tempStr);

                            int temp = std::stoi(tempStr);

                            if (temp != 0)
                                value = temp;
                            else
                                value = std::nullopt;
                        } else {
                            std::cout << " (or none): ";
                            std::string temp;

                            std::getline(std::cin, temp);

                            if (temp != "none")
                                value = temp;
                            else
                                value = std::nullopt;
                        }
                    } else {
                        std::cout << ": ";

                        std::getline(std::cin, value);
                    }
                });

                if (insert_row(db, "Person", p)) {
                    std::cout << "Row inserted via reflection.\n";
                }
            } else if (cmd == "lookup") {
                std::string name;
                std::cout << "First name to search: ";
                std::getline(std::cin, name);
                auto results = lookup_by_name<Person>(db, "Person", name);
                if (results.empty()) {
                    std::cout << "No matching records found.\n";
                } else {
                    for (const auto &r : results)
                        print_struct(r);
                }
            } else if (cmd == "remove") {
                std::string name;
                std::cout << "First name to remove: ";
                std::getline(std::cin, name);
                std::string q = "DELETE FROM Person WHERE first_name = '" + name + "';";
                if (sqlite3_exec(db, q.c_str(), nullptr, nullptr, nullptr) == SQLITE_OK) {
                    std::cout << "Removal query executed.\n";
                }
            }
        } catch (const std::invalid_argument &ex) {
            std::cerr << "**ERROR** Invalid Argument:" << ex.what() << std::endl;
        } catch (const std::exception &ex) {
            std::cerr << "**ERROR** " << ex.what() << std::endl;
        }
    }

    sqlite3_close(db);
    return 0;
}
