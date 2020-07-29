#include <iostream>

#include "local/H264Parser.h"

int main(int argc, char *argv[]) {
    if (argc != 2) {
        std::cerr << "Missing parameter" << std::endl;
        std::cerr << "Usage:" << std::endl;
        std::cerr << "\t" << std::string(argv[0]) << " BITSTREAM_FILE" << std::endl;

        return 1;
    }

    H264Parser parser(argv[1]);

    while (parser.readNextNAL());

    return 0;
}
