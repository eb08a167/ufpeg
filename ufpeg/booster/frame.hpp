#ifndef UFPEG_FRAME_HPP
#define UFPEG_FRAME_HPP

#include <cstddef>

namespace ufpeg {
    struct Frame {
        std::size_t success, failure;
    };
}

#endif
