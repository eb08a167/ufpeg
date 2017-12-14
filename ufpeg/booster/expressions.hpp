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

            for (auto it = this->items.begin(); it != this->items.end(); ++it) {
                auto base = (*it)->compile();
                instructions.emplace_back(base);

                auto i = std::distance(this->items.begin(), it);
                auto branch = std::make_shared<BranchInstruction>(2 * i + 3, 2 * size + 3);
                instructions.emplace_back(branch);
            }

            instructions.insert(instructions.end(), {
                std::make_shared<CommitInstruction>(),
                std::make_shared<JumpInstruction>(2 * size + 4),
                std::make_shared<AbortInstruction>(),
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

            for (auto it = this->choices.begin(); it != this->choices.end(); ++it) {
                auto base = (*it)->compile();
                instructions.emplace_back(base);

                auto i = std::distance(this->choices.begin(), it);
                auto branch = std::make_shared<BranchInstruction>(2 * size, 2 * i + 2);
                instructions.emplace_back(branch);
            }

            return std::make_shared<CompoundInstruction>(instructions);
        }
    private:
        const std::vector<std::shared_ptr<Expression>> choices;
    };

    class LiteralExpression: public Expression {
    public:
        LiteralExpression(const std::u32string &literal):
            literal(literal) {}

        std::shared_ptr<Instruction> compile() const {
            return std::make_shared<MatchLiteralInstruction>(this->literal);
        }
    private:
        const std::u32string literal;
    };

    class RepeatExpression: public Expression {
    public:
        RepeatExpression(const std::shared_ptr<Expression> &item):
            item(item) {}

        std::shared_ptr<Instruction> compile() const {
            std::vector<std::shared_ptr<Instruction>> instructions = {
                this->item->compile(),
                std::make_shared<BranchInstruction>(0, 2),
            };

            return std::make_shared<CompoundInstruction>(instructions);
        }
    private:
        const std::shared_ptr<Expression> item;
    };
}

#endif
