#include <iostream>
#include <meta>
#include <string_view>

template <typename T, typename Annotation> consteval auto has_annotation() {
    constexpr auto typeInfo = ^^T;

    template for (constexpr auto ann :
                  std::define_static_array(std::meta::annotations_of(typeInfo))) {
        if (std::meta::type_of(ann) == ^^const Annotation)
            return true;
    }

    return false;
}

template <typename T, typename Annotation>
concept HasAnnotation = has_annotation<T, Annotation>();

struct OStreamable {};

template <std::size_t N> struct OStreamFieldCaption {
    char caption[N];

    constexpr OStreamFieldCaption(const char (&str)[N]) {
        for (std::size_t i = 0; i < N; ++i) {
            caption[i] = str[i];
        }
    }
};

struct Test {};

struct[[= OStreamable{}]][[= Test{}]] Person {
    std::string_view name[[= Test{}]];
    int age[[= Test{}]][[= OStreamFieldCaption("Age")]]{};
};

template <HasAnnotation<OStreamable> T> std::ostream &operator<<(std::ostream &os, const T &v) {
    template for (constexpr auto dm : std::define_static_array(std::meta::nonstatic_data_members_of(
                      ^^T, std::meta::access_context::current()))) {
        bool captionFound = false;

        template for (constexpr auto ann :
                      std::define_static_array(std::meta::annotations_of(dm))) {
            constexpr auto annType = std::meta::type_of(ann);

            if constexpr (std::meta::has_template_arguments(annType) &&
                          std::meta::template_of(annType) == ^^OStreamFieldCaption) {
                constexpr auto caption = std::meta::extract<typename [:annType:]>(ann);
                os << caption.caption;
                captionFound = true;
                break;
            }
        }

        if (!captionFound)
            os << std::meta::identifier_of(dm);

        os << ": " << v.[:dm:] << ", ";
    }
    return os;
}

int main() {
    std::cout << Person{.name = "Waggy", .age = 44} << std::endl;
    std::cout << has_annotation<Person, Test>() << std::endl;
}
