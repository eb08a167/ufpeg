#ifndef UFPEG_COMPILER_HPP
#define UFPEG_COMPILER_HPP

#include "expressions.hpp"

namespace ufpeg {
    class Compiler {
    public:
        std::vector<std::shared_ptr<Instruction>> compile(const std::shared_ptr<Expression> &root) {
            CompilerContext context;

            auto instructions = root->compile(context);

            for (auto it = instructions.begin(); it != instructions.end(); ++it) {
                auto &reference = (*it)->get_reference();
                auto offset = std::distance(instructions.begin(), it);

                reference->resolve(offset);
            }

            return instructions;
        }
    };
}

#endif
