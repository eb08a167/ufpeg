#ifndef UFPEG_EXECUTOR_HPP
#define UFPEG_EXECUTOR_HPP

#include "instructions.hpp"

namespace ufpeg {
    class Executor {
    public:
        Executor(const std::vector<std::shared_ptr<Instruction>> &instructions):
            instructions(instructions) {}

        void execute(const std::u32string &text) const {
            ExecutorContext context = { text };
            context.pointers.push(0);
            context.cursors.push(0);
            context.nodes.push({ nullptr });
            context.offset = 0;
            context.has_matched = true;

            while (!context.pointers.empty()) {
                auto pointer = context.pointers.top();
                auto &instruction = this->instructions[pointer];

                instruction->execute(context);
            }
        }
    private:
        const std::vector<std::shared_ptr<Instruction>> instructions;
    };
}

#endif
