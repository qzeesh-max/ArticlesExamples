#include "ordered_static_initializer.hpp"

OrderedStaticInitializer& GetStaticInitializer() {
    static OrderedStaticInitializer initializer;
    return initializer;
}
