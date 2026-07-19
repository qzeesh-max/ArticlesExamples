#include <iostream>
#include <meta>

template <typename T, std::meta::info method, typename PreType, typename PostType,
          ptrdiff_t objectOffset, ptrdiff_t preOffset, ptrdiff_t postOffset>
struct ProxyMethod {
    template <typename... ArgumentType> auto operator()(ArgumentType &&...args) {
        auto &object = *reinterpret_cast<T *>(reinterpret_cast<char *>(this) + objectOffset);
        auto &pre = *reinterpret_cast<PreType *>(reinterpret_cast<char *>(this) + preOffset);
        auto &post = *reinterpret_cast<PostType *>(reinterpret_cast<char *>(this) + postOffset);

        if constexpr (!std::is_same_v<void, decltype(object.[:method:](
                                                std::forward<ArgumentType>(args)...))>) {
            pre(std::meta::identifier_of(method), std::forward<ArgumentType>(args)...);
            auto result = object.[:method:](std::forward<ArgumentType>(args)...);
            post(std::meta::identifier_of(method), std::forward<ArgumentType>(args)...);

            return result;
        }

        pre(std::meta::identifier_of(method), std::forward<ArgumentType>(args)...);
        object.[:method:](std::forward<ArgumentType>(args)...);
        post(std::meta::identifier_of(method), std::forward<ArgumentType>(args)...);
    }
};

template <typename T, typename PreType, typename PostType, typename... ArgumentType>
auto generateProxy(PreType &&pre, PostType &&post, ArgumentType &&...args) {
    struct GeneratedProxy;

    consteval {
        static constexpr auto members = std::define_static_array(
            std::meta::members_of(^^T, std::meta::access_context::current()));
        std::vector<std::meta::info> membersOfInterest;

        membersOfInterest.push_back(std::meta::data_member_spec(^^T, {
                                                                         .name = "__object__"}));

        membersOfInterest.push_back(std::meta::data_member_spec(^^PreType, {
                                                                               .name = "__pre__"}));

        membersOfInterest.push_back(
            std::meta::data_member_spec(^^PostType, {
                                                        .name = "__post__"}));

        struct dummy {
            alignas(alignof(T)) char __object__[sizeof(T)];
            alignas(alignof(PreType)) char __pre__[sizeof(PreType)];
            alignas(alignof(PostType)) char __post__[sizeof(PostType)];
        };

        constexpr const auto PrefixSize = sizeof(dummy);
        constexpr const auto PreOffset = offsetof(dummy, __pre__);
        constexpr const auto PostOffset = offsetof(dummy, __post__);

        template for (constexpr std::meta::info member : members) {
            if constexpr (std::meta::is_function(member) && std::meta::has_identifier(member)) {
                membersOfInterest.push_back(std::meta::data_member_spec(
                    ^^ProxyMethod<T, member, PreType, PostType, 0, PreOffset, PostOffset>,
                    {
                        .name = std::meta::identifier_of(member), .no_unique_address = true}));
            }
        }

        std::meta::define_aggregate(^^GeneratedProxy, membersOfInterest);
    }

    GeneratedProxy gp{T(std::forward<ArgumentType>(args)...), pre, post};

    return gp;
}

struct Test {
    std::string_view someName = "Yoda";

    void say() {
        std::cout << "Hello " << someName << std::endl;
    }

    int get_calc(int x, int y) {
        return x + y;
    }
};

int main() {
    std::cout << "starting" << std::endl;
    auto prxy = generateProxy<Test>(
        [](std::string_view method, auto &&...args) {
            std::cout << "Method call start : " << method << " args: [";
            const char *sep = "";
            (((std::cout << sep << args), sep = ", "), ...);
            std::cout << "]" << std::endl;
        },
        [](std::string_view method, auto &&...) {
            std::cout << "Method call end : " << method << std::endl;
        });

    prxy.say();

    const auto calc = prxy.get_calc(12, 14);

    std::cout << "calc = " << calc << std::endl;
}
