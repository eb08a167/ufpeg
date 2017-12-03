#ifndef UFPEG_NODE_HPP
#define UFPEG_NODE_HPP

#include <vector>

namespace ufpeg {
    struct Node {
        const char32_t *name;
        std::size_t start, stop;
        std::vector<Node> children;
    };
}

#endif
