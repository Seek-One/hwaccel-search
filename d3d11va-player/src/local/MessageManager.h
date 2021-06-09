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

#ifndef LOCAL_MESAGE_MANAGER_H_
#define LOCAL_MESAGE_MANAGER_H_

#include "WindowsHeaders.h"

#include "Size.h"

namespace dp {
  /**
   * @brief Represent different message type
   */
  enum class MessageType {
    Resize, //<! Rise event
    Quit, //<! Quit event
  };

  /**
   * @brief Encapsulate a message
   */
  struct Message {
    MessageType type; //<! Message type

    union {
      struct {
        SizeI windowSize; //<! New window size
      } resize; //<! Useful value for resize event
    };
  };

  constexpr LPCWSTR szWindowClass = L"D3D11VAPLAYER"; //<! Window class name for WIN32 API

  /**
   * @brief Manage message form WIN32 API
   */
  class MessageManager {
  public:
    /**
     * @brief Construct a new Message Manager object
     */
    MessageManager();
    ~MessageManager() = default;

    MessageManager(const MessageManager&) = delete;
    MessageManager(MessageManager&&) = delete;

    MessageManager& operator=(const MessageManager&) = delete;
    MessageManager& operator=(MessageManager&&) = delete;

    /**
     * @brief Retrieve a message
     *
     * @param message The message retrived
     *
     * @return true if there are a pending message otherwise false
     */
    bool poolMessage(Message& message);
  };
}

#endif // LOCAL_MESAGE_MANAGER_H_
