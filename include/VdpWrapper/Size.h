#ifndef VW_SIZE_H
#define VW_SIZE_H

#include <cstdint>

namespace vw {
    template<typename T>
    struct Size {
        Size(T width, T height)
        : width(width)
        , height(height) {
        }

        T width;
        T height;
    };

    using SizeI = Size<int32_t>;
    using SizeU = Size<uint32_t>;
    using SizeF = Size<float>;
}

#endif // VW_SIZE_H
