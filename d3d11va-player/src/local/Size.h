/* Copyright (c) 2021 Jet1oeil
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

#ifndef LOCAL_SIZE_H_
#define LOCAL_SIZE_H_

#include <type_traits>

namespace dp {
  template<typename IntegralType, typename = std::enable_if_t<std::is_integral_v<IntegralType> > >
  struct Size {
    IntegralType width;
    IntegralType height;

    Size() = default;
    Size(IntegralType w, IntegralType h)
    : width(w)
    , height(h) {
    }

    bool operator==(const Size<IntegralType> &other) {
      return width == other.width && height == other.height;
    }

    bool operator!=(const Size<IntegralType> &other) {
      return !(*this == other);
    }
  };

  using SizeI = Size<int>;
  using SizeU = Size<unsigned>;
}

#endif // LOCAL_SIZE_H_