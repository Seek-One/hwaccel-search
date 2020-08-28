/* Copyright (c) 2020 Jet1oeil
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

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
