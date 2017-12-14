#ifndef UFPEG_EXPRESSIONS_HPP
#define UFPEG_EXPRESSIONS_HPP

#include "instructions.hpp"

namespace ufpeg {
    class Expression {
    public:
        virtual ~Expression() = default;

        virtual std::shared_ptr<Instruction> compile() const = 0;
    };

    class SequenceExpression: public Expression {
    public:
        SequenceExpression(const std::vector<std::shared_ptr<Expression>> &expressions):
            expressions(expressions) {}

        std::shared_ptr<Instruction> compile() const {
            auto size = this->expressions.size();
            std::vector<std::shared_ptr<Instruction>> instructions;

            instructions.emplace_back(std::make_shared<BeginInstruction>());

            for (std::size_t i = 0; i < size; i++) {
                auto instruction = this->expressions[i]->compile();
                auto branch = std::make_shared<BranchInstruction>(2 * i + 1, 2 * size + 3);

                instructions.emplace_back(instruction);
                instructions.emplace_back(branch);
            }

            instructions.insert(instructions.end(), {
                std::make_shared<CommitInstruction>(),
                std::make_shared<JumpInstruction>(2 * size + 4),
                std::make_shared<AbortInstruction>(),
                std::make_shared<PassInstruction>(),
            });

            return std::make_shared<CompoundInstruction>(instructions);
        }
    private:
        const std::vector<std::shared_ptr<Expression>> expressions;
    };
}

#endif
