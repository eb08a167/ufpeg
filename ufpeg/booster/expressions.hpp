#ifndef UFPEG_EXPRESSIONS_HPP
#define UFPEG_EXPRESSIONS_HPP

#include <vector>
#include <algorithm>

#include "instructions.hpp"

namespace ufpeg {
    class Expression {
    public:
        virtual ~Expression() = default;

        virtual std::vector<std::shared_ptr<SmartInstruction>> compile(CompilerContext&) const = 0;
    };

    class SequenceExpression: public Expression {
    public:
        SequenceExpression(const std::vector<std::shared_ptr<Expression>> &items):
            items(items) {
                if (this->items.empty()) {
                    throw std::logic_error(
                        "SequenceExpression with no items does not make any sense"
                    );
                }
            }

        std::vector<std::shared_ptr<SmartInstruction>> compile(CompilerContext &context) const {
            auto pass = std::make_shared<PassInstruction>();
            auto abort = std::make_shared<AbortInstruction>();
            auto jump = std::make_shared<JumpInstruction>(pass->get_reference());
            auto commit = std::make_shared<CommitInstruction>();

            std::vector<std::shared_ptr<SmartInstruction>> instructions = { pass, abort, jump, commit };

            std::shared_ptr<SmartInstruction> success = commit, failure = abort;

            for (auto it = this->items.rbegin(); it != this->items.rend(); ++it) {
                auto branch = std::make_shared<BranchInstruction>(
                    success->get_reference(),
                    failure->get_reference()
                );
                instructions.emplace_back(branch);

                auto item_instructions = (*it)->compile(context);
                instructions.insert(
                    instructions.end(),
                    std::make_move_iterator(item_instructions.rbegin()),
                    std::make_move_iterator(item_instructions.rend())
                );

                success = instructions.back();
            }

            instructions.emplace_back(std::make_shared<BeginInstruction>());

            std::reverse(instructions.begin(), instructions.end());

            return instructions;
        }
    private:
        const std::vector<std::shared_ptr<Expression>> items;
    };

    class ChoiceExpression: public Expression {
    public:
        ChoiceExpression(const std::vector<std::shared_ptr<Expression>> &choices):
            choices(choices) {
                if (this->choices.empty()) {
                    throw std::logic_error(
                        "ChoiceExpression with no choices does not make any sense"
                    );
                }
            }

        std::vector<std::shared_ptr<SmartInstruction>> compile(CompilerContext &context) const {
            auto pass = std::make_shared<PassInstruction>();

            std::vector<std::shared_ptr<SmartInstruction>> instructions = { pass };

            std::shared_ptr<SmartInstruction> success = pass, failure = pass;

            for (auto it = this->choices.rbegin(); it != this->choices.rend(); ++it) {
                if (success != failure) {
                    auto branch = std::make_shared<BranchInstruction>(
                        success->get_reference(),
                        failure->get_reference()
                    );
                    instructions.emplace_back(branch);
                }

                auto choice_instructions = (*it)->compile(context);
                instructions.insert(
                    instructions.end(),
                    std::make_move_iterator(choice_instructions.rbegin()),
                    std::make_move_iterator(choice_instructions.rend())
                );

                failure = instructions.back();
            }

            std::reverse(instructions.begin(), instructions.end());

            return instructions;
        }
    private:
        const std::vector<std::shared_ptr<Expression>> choices;
    };

    class LiteralExpression: public Expression {
    public:
        LiteralExpression(const std::u32string &literal):
            literal(literal) {}

        std::vector<std::shared_ptr<SmartInstruction>> compile(CompilerContext&) const {
            return { std::make_shared<MatchLiteralInstruction>(this->literal) };
        }
    private:
        const std::u32string literal;
    };

    class RepeatExpression: public Expression {
    public:
        RepeatExpression(const std::shared_ptr<Expression> &item):
            item(item) {}

        std::vector<std::shared_ptr<SmartInstruction>> compile(CompilerContext &context) const {
            auto pass = std::make_shared<PassInstruction>();
            auto instructions = this->item->compile(context);
            auto &success = pass;
            auto &failure = instructions.front();
            auto branch = std::make_shared<BranchInstruction>(
                success->get_reference(),
                failure->get_reference()
            );

            instructions.emplace_back(branch);
            instructions.emplace_back(pass);

            return instructions;
        }
    private:
        const std::shared_ptr<Expression> item;
    };
}

#endif
