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

#include <algorithm>
#include <iostream>
#include <numeric>

#include <VdpWrapper/Display.h>
#include <VdpWrapper/Decoder.h>
#include <VdpWrapper/Device.h>
#include <VdpWrapper/NalUnit.h>
#include <VdpWrapper/PresentationQueue.h>
#include <VdpWrapper/Size.h>
#include <VdpWrapper/RenderSurface.h>
#include <VdpWrapper/DecodedSurface.h>
#include <VdpWrapper/VideoMixer.h>

#include "local/Clock.h"
#include "local/H264Parser.h"

namespace {
    void printUsage(const std::string& commandName, const std::string& message) {
        std::cerr << message << std::endl;
        std::cerr << "Usage:" << std::endl;
        std::cerr << "\t" << commandName << " [OPTION...] BITSTREAM_FILE" << std::endl;
        std::cerr << std::endl;
        std::cerr << "Options:" << std::endl;
        std::cerr << "\t--initial-size <width>x<height>\t\tSet the initial screen size" << std::endl;
        std::cerr << "\t--disable-pts\t\t\t\tDisplay images in decode order" << std::endl;
        std::cerr << "\t--enable-pts\t\t\t\tDisplay images in presentation order" << std::endl;
        std::cerr << "\t--fps <FPS>\t\t\t\tSet the video FPS" << std::endl;
        std::cerr << "\t--benchmark\t\t\t\tEnable time benchmark" << std::endl;
    }
}

int main(int argc, char *argv[]) {
    int iCurrentArg = 1;
    vw::SizeU screenSize(1280, 720);
    bool bEnablePTS = true;
    int iFPS = 25;
    bool bBenchmarkEnabled = false;
    Clock clock;

    while (iCurrentArg < argc - 1) {
        std::string szArg = std::string(argv[iCurrentArg]);
        if (szArg == "--initial-size") {
            bool bOptionParseFailed = false;
            std::string szWidth;
            std::string szHeight;

            if (iCurrentArg >= argc - 1) {
                bOptionParseFailed = true;
            }
            else {
                std::string szValue = std::string(argv[iCurrentArg + 1]);
                int iDelimiterIndex = szValue.find_first_of("x");
                szWidth = szValue.substr(0, iDelimiterIndex);
                szHeight = szValue.substr(iDelimiterIndex + 1);

                try {
                    int iWidth = std::stoi(szWidth);
                    int iHeight = std::stoi(szHeight);

                    screenSize = vw::SizeU(iWidth, iHeight);
                } catch (std::invalid_argument &e) {
                    bOptionParseFailed = true;
                }
            }

            if (bOptionParseFailed) {
                printUsage(argv[0], "Wrong size values");
                return 1;
            }

            std::cout << "[main] Set initial screen size: " << szWidth << "x" << szHeight << std::endl;
            iCurrentArg += 2;
        } else if (szArg == "--fps") {
            bool bOptionParseFailed = false;
            std::string szFPS;
            std::string szValue;

            if (iCurrentArg >= argc - 1) {
                bOptionParseFailed = true;
            }
            else {
                szValue = std::string(argv[iCurrentArg + 1]);

                try {
                    iFPS = std::stoi(szValue);
                } catch (std::invalid_argument &e) {
                    bOptionParseFailed = true;
                }
            }

            if (bOptionParseFailed) {
                printUsage(argv[0], "Wrong FPS value");
                return 1;
            }

            std::cout << "[main] Set FPS to: " << szValue << std::endl;

            iCurrentArg += 2;
        } else if (szArg == "--disable-pts") {
            bEnablePTS = false;
            std::cout << "[main] Display images in decode order" << std::endl;
            ++iCurrentArg;
        } else if (szArg == "--enable-pts") {
            bEnablePTS = true;
            std::cout << "[main] Display images in presentation order" << std::endl;
            ++iCurrentArg;
        } else if (szArg == "--benchmark") {
            bBenchmarkEnabled = true;
            std::cout << "[main] Enable benchmark" << std::endl;
            ++iCurrentArg;
        } else {
            printUsage(argv[0], "'" + szArg + "' unknown option");
            return 1;
        }
    }

    if (iCurrentArg != argc - 1) {
        printUsage(argv[0], "Missing parameter");
        return 1;
    }

    std::string szBitstreamFile(argv[iCurrentArg]);
    vw::Display display(screenSize);
    vw::Device device(display);
    vw::Decoder decoder(device);
    vw::PresentationQueue presentationQueue(display, device);
    presentationQueue.setFramerate(iFPS);
    presentationQueue.enablePresentationOrderDisplay(bEnablePTS);
    vw::VideoMixer mixer(device, screenSize);

    H264Parser parser(szBitstreamFile);
    vw::NalUnit nalUnit;

    // Benchmark variables
    std::chrono::microseconds totalTime;
    std::vector<std::chrono::microseconds> listDecodeTimes;
    std::vector<std::chrono::microseconds> listPostProcessTimes;
    std::vector<std::chrono::microseconds> listDisplayTimes;

    while (parser.readNextNAL(nalUnit) && display.isOpened()) {
        // Handle XEvent
        display.processEvent();
        mixer.setOutputSize(display.getScreenSize());

        switch (nalUnit.getType()) {
        case vw::NalType::SPS:
        case vw::NalType::PPS:
        case vw::NalType::SEI:
            // Nothing to do
            break;

        case vw::NalType::CodedSliceNonIDR:
        case vw::NalType::CodedSliceIDR: {
            if (bBenchmarkEnabled) {
                clock.start();
            }
            vw::DecodedSurface& decodedSurface = decoder.decode(nalUnit);
            if (bBenchmarkEnabled) {
                auto elapsedTime = clock.restart();
                listDecodeTimes.push_back(elapsedTime);
                totalTime = elapsedTime;
                std::cout << "[main] Decode time: " << elapsedTime.count() << " µs" << std::endl;
            }

            if (bBenchmarkEnabled) {
                clock.start();
            }
            vw::RenderSurface outputSurface = mixer.process(decodedSurface);
            if (bBenchmarkEnabled) {
                auto elapsedTime = clock.restart();
                listPostProcessTimes.push_back(elapsedTime);
                totalTime += elapsedTime;
                std::cout << "[main] Post-process time: " << elapsedTime.count() << " µs" << std::endl;
            }

            if (bBenchmarkEnabled) {
                clock.start();
            }
            presentationQueue.enqueue(std::move(outputSurface));
            if (bBenchmarkEnabled) {
                auto elapsedTime = clock.restart();
                listDisplayTimes.push_back(elapsedTime);
                totalTime += elapsedTime;
                std::cout << "[main] Display time: " << std::chrono::duration_cast<std::chrono::milliseconds>(elapsedTime).count() << " ms" << std::endl;
                std::cout << "[main] Total time: " << std::chrono::duration_cast<std::chrono::milliseconds>(totalTime).count() << " ms" << std::endl;
                std::cout << std::endl;
            }
            break;
        }

        case vw::NalType::Unspecified:
            std::cerr << "[main] Unknown NAL type" << std::endl;
            break;

        default:
            std::cout << "[main] Unhandled NAL type: " << static_cast<int>(nalUnit.getType()) << std::endl;
        }
    }

    std::cout << "[main] End of parsing" << std::endl;
    if (bBenchmarkEnabled) {
        std::cout << std::endl;
        std::cout << "[main] Benchmarks stats:" << std::endl;
        auto computeState = [](const std::vector<std::chrono::microseconds>& listTimes, const std::string& outputPrefix) {
            double sum = std::accumulate(listTimes.begin(), listTimes.end(), 0, [](const auto& accumulator, const auto& element) {
                return element.count() + accumulator;
            });
            double mean = sum / listTimes.size();
            auto [min, max] = std::minmax_element(listTimes.begin(), listTimes.end());

            std::cout << "[main] " << outputPrefix << ": min = " << min->count() << " µs ; max = " << max->count() << " µs ; mean = " << mean << " µs" << std::endl;
        };

        computeState(listDecodeTimes, "Decode time");
        computeState(listPostProcessTimes, "Post-process time");
        computeState(listDisplayTimes, "Display time");
    }

    while (display.isOpened()) {
        display.waitEvent();
    }

    return 0;
}
