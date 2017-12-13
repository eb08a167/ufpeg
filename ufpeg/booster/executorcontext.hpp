#ifndef UFPEG_EXECUTOR_CONTEXT_HPP
#define UFPEG_EXECUTOR_CONTEXT_HPP

#include <string>
#include <stack>

#include "node.hpp"

namespace ufpeg {
    template<typename T>
    using fast_stack = std::stack<T, std::vector<T>>;

    struct ExecutorContext {
        const std::u32string text;
        fast_stack<std::size_t> pointers, cursors;
        fast_stack<Node> nodes;
        std::vector<const char32_t*> expectations;
        std::size_t offset;
        bool has_matched;
    };
}

#endif
