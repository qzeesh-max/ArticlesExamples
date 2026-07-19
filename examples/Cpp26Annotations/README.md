# C++26: Annotations and Reflection: Custom Attributes

*July 4^th, 2026 By Zeeshan Qazi*

C++26 reflection introduces the ability to add and query custom annotations (attributes) on types and fields. This allows developers to attach rich metadata to structures at compile-time and effortlessly process it using the new reflection capabilities.

In this demonstration, we define an `[[= OStreamable{}]]` custom annotation alongside an `[[= OStreamFieldCaption("...")]]` annotation. We then utilize these annotations combined with compile-time reflection to automatically generate an `operator&lt;&lt;` for any structure annotated as `OStreamable`.

The code implementation introduces a `has_annotation` consteval function:

```cpp
template  consteval auto has_annotation() {
    constexpr auto typeInfo = ^^T;

    template for (constexpr auto ann :
                  std::define_static_array(std::meta::annotations_of(typeInfo))) {
        if (std::meta::type_of(ann) == ^^const Annotation)
            return true;
    }

    return false;
}
```

By extracting `std::meta::annotations_of` on a type, we can loop over all attached annotations and compare them using `std::meta::type_of` method. This lets us build a simple `HasAnnotation` concept.

The real magic happens in our generic `operator&lt;&lt;` serialization method. It loops through `std::meta::nonstatic_data_members_of` to extract all fields in a struct. For each field, we check if it has the `OStreamFieldCaption` annotation. If it does, we extract the string literal to use as a custom label; otherwise, we gracefully fallback to using the original variable name via `std::meta::identifier_of(dm)`.

Here is an example of an annotated struct utilizing these features:

```cpp
struct[[= OStreamable{}]][[= Test{}]] Person {
    std::string_view name[[= Test{}]];
    int age[[= Test{}]][[= OStreamFieldCaption("Age")]]{};
};
```

## Sample Execution

```bash
$ ./cpp26_annotations_demo
name: Waggy, Age: 44, 
1
```

[ annotations_example.cpp](javascript:DoLink('/download-sourcecode.php?Example=annotations_example');)

- **G++ C++26 Compilation Instructions:** `g++ ./annotations_example.cpp -o cpp26_annotations_demo --std=c++26 -freflection`
- **Clang++ C++26 Compilation Instructions:** `clang++ ./annotations_example.cpp -o cpp26_annotations_demo --std=c++26 -freflection -fexpansion-statements`

---
