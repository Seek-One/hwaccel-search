#ifndef VW_SIZE_H
#define VW_SIZE_H

#include <cstdint>

namespace vw {
    /**
     * @brief Size is an utility class to handle a size
     *
     * @tparam T Type of size unit (int, unsigned, float...)
     */
    template<typename T>
    struct Size {
        /**
         * @brief Construct a new Size object
         *
         * @param width Width value
         * @param height Height value
         */
        Size(T width = T(), T height = T())
        : width(width)
        , height(height) {
        }

        /**
         * @brief Copy constructor
         *
         * @tparam OtherT Type of other size unit
         * @param other Other Size to be copied
         */
        template<typename OtherT>
        Size(Size<OtherT> other)
        : width(other.width)
        , height(other.height) {
        }

        T width; ///< Width value
        T height; ///< Height value

        /**
         * @brief Equality operator
         *
         * @param other Other size
         * @return bool Check equality
         */
        inline bool operator==(const Size<T>& other) {
            return width == other.width && height == other.height;
        }

        /**
         * @brief Inequality operator
         *
         * @param other Other size
         * @return bool Check inequality
         */
        inline bool operator!=(const Size<T>& other) {
            return !(*this == other);
        }
    };

    /**
     * @brief Equality operator
     *
     * @param lhs Size
     * @param rhs Another size
     * @return bool Check equality
     */
    template<typename T1, typename T2>
    inline bool operator==(const Size<T1>&lhs, const Size<T2>&rhs) {
        return lhs.width == rhs.width && lhs.height == rhs.height;
    }

    /**
     * @brief Equality operator
     *
     * @param lhs Size
     * @param rhs Another size
     * @return bool Check equality
     */
    template<typename T1, typename T2>
    inline bool operator!=(const Size<T1>&lhs, const Size<T2>&rhs) {
        return !(lhs == rhs);
    }

    using SizeI = Size<int32_t>;
    using SizeU = Size<uint32_t>;
    using SizeF = Size<float>;
}

#endif // VW_SIZE_H
