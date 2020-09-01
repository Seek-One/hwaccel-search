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

#ifndef LOCAL_CLOCK_H
#define LOCAL_CLOCK_H

#include <chrono>

template<typename ClockType = std::chrono::high_resolution_clock>
class Clock {
public:
    void start() {
        m_startTime = ClockType::now();
    }

    template<typename Duration = std::chrono::microseconds>
    Duration elapsed() {
        auto endTime = ClockType::now();
        return std::chrono::duration_cast<Duration>(endTime-m_startTime);
    }

    template<typename Duration = std::chrono::microseconds>
    Duration restart() {
        auto endTime = ClockType::now();
        auto diff = std::chrono::duration_cast<Duration>(endTime-m_startTime);
        m_startTime = endTime;
        return diff;
    }

private:
    std::chrono::time_point<ClockType> m_startTime;
};

#endif // LOCAL_CLOCK_H
