#ifndef UFPEG_VM_HPP
#define UFPEG_VM_HPP

#include <memory>

#include "instructions.hpp"

namespace ufpeg {
    class VM {
    public:
        VM(const std::vector<std::shared_ptr<BaseInstruction>> &instructions):
            instructions(instructions) {}

        void run(const std::u32string &text) const {
            Context context(text);

            while (!context.pointers.empty()) {
                auto pointer = context.pointers.top();
                auto &instruction = this->instructions[pointer];

                instruction->update(context);
            }
        }
    private:
        const std::vector<std::shared_ptr<BaseInstruction>> instructions;
    };
}

#endif
