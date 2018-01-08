#ifndef UFPEG_EXECUTOR_HPP
#define UFPEG_EXECUTOR_HPP

#include "instructions.hpp"

namespace ufpeg {
    class Executor {
    public:
        Executor(const std::vector<std::shared_ptr<Instruction>> &instructions):
            instructions(instructions) {}

        Node execute(const std::u32string &text) const {
            ExecutorContext context = { text };
            context.pointer = 0;
            context.frames.push({ 0, 0 });
            context.nodes.push({ {} });
            context.cursors.push(0);
            context.offset = 0;

            while (!context.frames.empty()) {
                auto &instruction = this->instructions[context.pointer];

                instruction->update(context);
            }

            return std::move(context.nodes.top());
        }
    private:
        const std::vector<std::shared_ptr<Instruction>> instructions;
    };
}

#endif
