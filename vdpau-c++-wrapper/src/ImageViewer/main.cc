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

#include <iostream>

#include <VdpWrapper/Device.h>
#include <VdpWrapper/Display.h>
#include <VdpWrapper/PresentationQueue.h>
#include <VdpWrapper/RenderSurface.h>
#include <VdpWrapper/DecodedSurface.h>
#include <VdpWrapper/VideoMixer.h>

namespace {
    void printUsage(const std::string& commandName, const std::string& message) {
        std::cerr << message << std::endl;
        std::cerr << "Usage:" << std::endl;
        std::cerr << "\t" << commandName << " [OPTION] --image-size <width>x<height> RAW_IMAGE_FILE" << std::endl;
        std::cerr << std::endl;
        std::cerr << "Options:" << std::endl;
        std::cerr << "\t--image-size <width>x<height>\t\tSet the source image size" << std::endl;
        std::cerr << "\t--initial-size <width>x<height>\t\tSet the initial screen size" << std::endl;
    }
}

int main(int argc, char *argv[]) {
    int iCurrentArg = 1;
    vw::SizeU screenSize(1280, 720);
    vw::SizeU sourceSize(0, 0);

    while (iCurrentArg < argc - 1) {
        std::string szArg = std::string(argv[iCurrentArg]);
        if (szArg == "--image-size") {
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

                    sourceSize = vw::SizeU(iWidth, iHeight);
                } catch (std::invalid_argument &e) {
                    bOptionParseFailed = true;
                }
            }

            if (bOptionParseFailed) {
                printUsage(argv[0], "Wrong size values");
                return 1;
            }

            std::cout << "[main] Set source image size: " << szWidth << "x" << szHeight << std::endl;
            iCurrentArg += 2;
        } else if (szArg == "--initial-size") {
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
        } else {
            printUsage(argv[0], "'" + szArg + "' unknown option");
            return 1;
        }
    }

    if (iCurrentArg != argc - 1) {
        printUsage(argv[0], "Missing parameter");
        return 1;
    }

    if (sourceSize == vw::SizeU(0u, 0u)) {
        printUsage(argv[0], "The source image size must be defined");
        return 1;
    }

    std::string szRawImageFile(argv[iCurrentArg]);

    vw::Display display(screenSize);
    vw::Device device(display);
    vw::PresentationQueue presentationQueue(display, device);
    vw::DecodedSurface inputSurface(device, szRawImageFile, sourceSize);
    vw::VideoMixer mixer(device, screenSize);

    while (display.isOpened()) {
        // Update the output surface
        mixer.setOutputSize(display.getScreenSize());
        vw::RenderSurface outputSurface = mixer.process(inputSurface);
        presentationQueue.enqueue(std::move(outputSurface));

        display.waitEvent();
    }

    return 0;
}
