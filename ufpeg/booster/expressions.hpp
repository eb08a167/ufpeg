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
        SequenceExpression(const std::vector<std::shared_ptr<Expression>> &items):
            items(items) {}

        std::shared_ptr<Instruction> compile() const {
            auto size = this->items.size();
            std::vector<std::shared_ptr<Instruction>> instructions;

            instructions.emplace_back(std::make_shared<BeginInstruction>());

            for (std::size_t i = 0; i < size; i++) {
                auto base = this->items[i]->compile();
                auto branch = std::make_shared<BranchInstruction>(2 * i + 3, 2 * size + 3);

                instructions.emplace_back(base);
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
        const std::vector<std::shared_ptr<Expression>> items;
    };

    class ChoiceExpression: public Expression {
    public:
        ChoiceExpression(const std::vector<std::shared_ptr<Expression>> &choices):
            choices(choices) {}

        std::shared_ptr<Instruction> compile() const {
            auto size = this->choices.size();
            std::vector<std::shared_ptr<Instruction>> instructions;

            for (auto it = this->choices.begin(); it != this->choices.end(); it++) {
                auto i = std::distance(this->choices.begin(), it);

                auto base = this->choices[i]->compile();
                instructions.emplace_back(base);

                if (it != this->choices.end()) {
                    auto branch = std::make_shared<BranchInstruction>(2 * size + 1, 2 * i + 2);
                    instructions.emplace_back(branch);
                }
            }

            instructions.emplace_back(std::make_shared<PassInstruction>());

            return std::make_shared<CompoundInstruction>(instructions);
        }
    private:
        const std::vector<std::shared_ptr<Expression>> choices;
    };
}

#endif
