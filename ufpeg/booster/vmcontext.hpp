#ifndef UFPEG_CONTEXT_HPP
#define UFPEG_CONTEXT_HPP

#include <string>
#include <stack>

#include "node.hpp"

namespace ufpeg {
    template<typename T>
    using fast_stack = std::stack<T, std::vector<T>>;

    struct VmContext {
        VmContext(const std::u32string &text):
            text(text) {
            this->pointers.push(0);
            this->cursors.push(0);
            this->nodes.push({ nullptr });
            this->offset = 0;
            this->has_matched = true;
        }

        const std::u32string text;
        fast_stack<std::size_t> pointers, cursors;
        fast_stack<Node> nodes;
        std::vector<const char32_t*> expectations;
        std::size_t offset;
        bool has_matched;
    };
}

#endif
