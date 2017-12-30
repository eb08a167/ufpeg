#ifndef UFPEG_COMPILE_OPTIONS_HPP
#define UFPEG_COMPILE_OPTIONS_HPP

#include <memory>

#include "reference.hpp"

namespace ufpeg {
    struct CompileOptions {
        std::shared_ptr<Reference> success;
        std::shared_ptr<Reference> failure;
        std::shared_ptr<Reference> entry;
    };
}

#endif
