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

#pragma comment(lib, "d3d11")
#pragma comment(lib, "dxgi")

#include <iostream>

#include "local/Clock.h"
#include "local/D3D11Manager.h"
#include "local/Decoder.h"
#include "local/FileParser.h"
#include "local/Filter.h"
#include "local/Window.h"

int main(int argc, char* argv[]) {
  if (argc != 2) {
    std::cerr << "Missing parameter" << std::endl;
    std::cerr << "Usage: " << argv[0] << " BITSTREAM_FILE" << std::endl;

    return 1;
  }

  dp::FileParser fileParser(argv[1]);
  fileParser.extractPictureSizes();
  auto rawPictureSize = fileParser.getRawPictureSize();
  auto realPictureSize = fileParser.getRealPictureSize();

  dp::D3D11Manager manager;
  dp::Decoder decoder(manager, rawPictureSize, realPictureSize);
  dp::Window window(manager);
  dp::Filter filter(manager);

  while (window.isActive()) {
    window.procMessage();

    // If it's the end of stream, we only keep the window
    const auto& stream = fileParser.getStream();
    if (!fileParser.parseNextNAL() || (stream.nal->nal_unit_type != NAL_UNIT_TYPE_CODED_SLICE_IDR && stream.nal->nal_unit_type != NAL_UNIT_TYPE_CODED_SLICE_NON_IDR)) {
      continue;
    }

    dp::Clock totalProcessClock;

    window.clear();

    const auto& decodedTexture = decoder.decodeSlice(fileParser);
    filter.process(decodedTexture, window.getCurrentBackbuffer());

    window.render();

    auto elapsedTime = totalProcessClock.elapsed();
    std::cout << "[Main] Total proccessing in " << elapsedTime.count() << "ms" << std::endl;
  }

  return 0;
}
