#ifndef UFPEG_EXECUTOR_CONTEXT_HPP
#define UFPEG_EXECUTOR_CONTEXT_HPP

#include <stack>

#include "frame.hpp"
#include "node.hpp"

namespace ufpeg {
    struct ExecutorContext {
        std::u32string text;
        std::size_t pointer;
        std::stack<Frame> frames;
        std::stack<Node> nodes;
        std::stack<std::size_t> cursors;
        std::vector<const char32_t*> expectations;
        std::size_t offset;
    };
}

#endif
