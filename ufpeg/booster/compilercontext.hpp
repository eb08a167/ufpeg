#ifndef UFPEG_COMPILER_CONTEXT_HPP
#define UFPEG_COMPILER_CONTEXT_HPP

#include <string>
#include <memory>
#include <map>

#include "reference.hpp"

namespace ufpeg {
    struct CompilerContext {
        std::map<std::u32string, std::shared_ptr<Reference>> references;
    };
}

#endif
