#ifndef UFPEG_NODE_HPP
#define UFPEG_NODE_HPP

#include <string>
#include <vector>

namespace ufpeg {
    struct Node {
        std::u32string name;
        std::size_t start, stop;
        std::vector<Node> children;
    };
}

#endif
