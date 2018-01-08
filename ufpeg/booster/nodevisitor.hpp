#ifndef UFPEG_NODE_VISITOR_HPP
#define UFPEG_NODE_VISITOR_HPP

#include <functional>
#include <map>

#include "node.hpp"

namespace ufpeg {
    template <typename T>
    class NodeVisitor {
        typedef std::function<T()> handler_type;
    public:
        void add_handler(const std::u32string &name, const handler_type &handler) {
            this->handlers.emplace(name, handler);
        }

        T visit(const Node &node) const {
            auto &handler = this->handlers.at(node.name);

            return handler();
        }
    private:
        std::map<std::u32string, handler_type> handlers;
    };
}

#endif
