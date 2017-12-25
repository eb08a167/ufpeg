#ifndef UFPEG_COMPILER_HPP
#define UFPEG_COMPILER_HPP

#include "expressions.hpp"

namespace ufpeg {
    class Compiler {
    public:
        std::vector<RawInstruction> compile(const std::shared_ptr<Expression> &root) {
            CompilerContext context;

            auto smart_instructions = root->compile(context);

            for (auto it = smart_instructions.begin(); it != smart_instructions.end(); ++it) {
                auto &reference = (*it)->get_reference();
                auto offset = std::distance(smart_instructions.begin(), it);

                reference->resolve(offset);
            }

            std::vector<RawInstruction> raw_instructions;

            for (const auto &smart_instruction: smart_instructions) {
                raw_instructions.emplace_back(
                    smart_instruction->to_raw(context)
                );
            }

            return raw_instructions;
        }
    };
}

#endif
