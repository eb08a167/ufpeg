#ifndef UFPEG_EXPRESSIONS_HPP
#define UFPEG_EXPRESSIONS_HPP

#include <algorithm>

#include "instructions.hpp"
#include "compilercontext.hpp"

namespace ufpeg {
    class Expression {
    public:
        virtual ~Expression() = default;

        virtual std::vector<std::shared_ptr<Instruction>> compile(
            CompilerContext &context,
            std::shared_ptr<Reference> success,
            std::shared_ptr<Reference> failure
        ) const = 0;
    };

    class SequenceExpression: public Expression {
    public:
        SequenceExpression(const std::vector<std::shared_ptr<Expression>> &items):
            items(items) {}

        std::vector<std::shared_ptr<Instruction>> compile(
            CompilerContext &context,
            std::shared_ptr<Reference> success,
            std::shared_ptr<Reference> failure
        ) const {
            auto commit = std::make_shared<CommitInstruction>(success);
            auto abort = std::make_shared<AbortInstruction>(failure);

            std::vector<std::shared_ptr<Instruction>> instructions = { abort, commit };

            success = commit->get_reference();
            failure = abort->get_reference();

            for (auto it = this->items.rbegin(); it != this->items.rend(); ++it) {
                auto item_instructions = (*it)->compile(context, success, failure);

                success = item_instructions.back()->get_reference();

                instructions.insert(
                    instructions.end(),
                    std::make_move_iterator(item_instructions.rbegin()),
                    std::make_move_iterator(item_instructions.rend())
                );
            }

            instructions.emplace_back(std::make_shared<BeginInstruction>(success));

            std::reverse(instructions.begin(), instructions.end());

            return instructions;
        }
    private:
        const std::vector<std::shared_ptr<Expression>> items;
    };

    class ChoiceExpression: public Expression {
    public:
        ChoiceExpression(const std::vector<std::shared_ptr<Expression>> &choices):
            choices(choices) {}

        std::vector<std::shared_ptr<Instruction>> compile(
            CompilerContext &context,
            std::shared_ptr<Reference> success,
            std::shared_ptr<Reference> failure
        ) const {
            auto jump = std::make_shared<JumpInstruction>(failure);

            std::vector<std::shared_ptr<Instruction>> instructions = { jump };

            failure = jump->get_reference();

            for (auto it = this->choices.rbegin(); it != this->choices.rend(); ++it) {
                auto choice_instructions = (*it)->compile(context, success, failure);

                failure = choice_instructions.back()->get_reference();

                instructions.insert(
                    instructions.end(),
                    std::make_move_iterator(choice_instructions.rbegin()),
                    std::make_move_iterator(choice_instructions.rend())
                );
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

        std::vector<std::shared_ptr<Instruction>> compile(
            CompilerContext &context,
            std::shared_ptr<Reference> success,
            std::shared_ptr<Reference> failure
        ) const {
            return { std::make_shared<MatchLiteralInstruction>(this->literal, success, failure) };
        }
    private:
        const std::u32string literal;
    };

    class ZeroOrOneExpression: public Expression {
    public:
        ZeroOrOneExpression(const std::shared_ptr<Expression> &item):
            item(item) {}

        std::vector<std::shared_ptr<Instruction>> compile(
            CompilerContext &context,
            std::shared_ptr<Reference> success,
            std::shared_ptr<Reference> failure
        ) const {
            return this->item->compile(context, success, success);
        }
    private:
        const std::shared_ptr<Expression> item;
    };

    class ZeroOrMoreExpression: public Expression {
    public:
        ZeroOrMoreExpression(const std::shared_ptr<Expression> &item):
            item(item) {}

        std::vector<std::shared_ptr<Instruction>> compile(
            CompilerContext &context,
            std::shared_ptr<Reference> success,
            std::shared_ptr<Reference> failure
        ) const {
            auto reference = std::make_shared<Reference>();
            auto instructions = this->item->compile(context, reference, success);
            auto target = instructions.front()->get_reference();
            auto jump = std::make_shared<JumpInstruction>(target, reference);

            instructions.emplace_back(jump);

            return instructions;
        }
    private:
        const std::shared_ptr<Expression> item;
    };

    class OneOrMoreExpression: public Expression {
    public:
        OneOrMoreExpression(const std::shared_ptr<Expression> &item):
            item(item) {}

        std::vector<std::shared_ptr<Instruction>> compile(
            CompilerContext &context,
            std::shared_ptr<Reference> success,
            std::shared_ptr<Reference> failure
        ) const {
            SequenceExpression expression({
                this->item,
                std::make_shared<ZeroOrMoreExpression>(this->item),
            });

            return expression.compile(context, success, failure);
        }
    private:
        const std::shared_ptr<Expression> item;
    };
}

#endif

// L1: BEGIN L2
// L2: MATCH_LITERAL "foo" L3 L5
// L3: MATCH_LITERAL "bar" L4 L5
// L4: COMMIT @success
// L5: ABORT @failure

// L1: MATCH_LITERAL "foo" @success L2
// L2: MATCH_LITERAL "bar" @success L3
// L3: JUMP @failure

// L1: PREPARE L2
// L2: MATCH_LITERAL "foobar" L3 L5
// L3: CONSUME "node" L5
// L4: DISCARD L6
// L5: REVOKE_SUCCESS
// L6: REVOKE_FAILURE

// L1: MATCH_LITERAL "foo" L2 @success
// L2: JUMP L1
