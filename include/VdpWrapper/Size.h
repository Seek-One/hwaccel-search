#ifndef VW_SIZE_H
#define VW_SIZE_H

#include <cstdint>

namespace vw {
    template<typename T>
    struct Size {
        Size(T width = T(), T height = T())
        : width(width)
        , height(height) {
        }

        template<typename OtherT>
        Size(Size<OtherT> other)
        : width(other.width)
        , height(other.height) {
        }

        T width;
        T height;

        bool operator==(const Size<T>& other) {
            return width == other.width && height == other.height;
        }

        bool operator!=(const Size<T>& other) {
            return !(*this == other);
        }
    };

    template<typename T1, typename T2>
    bool operator==(const Size<T1>&lhs, const Size<T2>&rhs) {
        return lhs.width == rhs.width && lhs.height == rhs.height;
    }

    template<typename T1, typename T2>
    bool operator!=(const Size<T1>&lhs, const Size<T2>&rhs) {
        return !(lhs == rhs);
    }

    using SizeI = Size<int32_t>;
    using SizeU = Size<uint32_t>;
    using SizeF = Size<float>;
}

#endif // VW_SIZE_H
