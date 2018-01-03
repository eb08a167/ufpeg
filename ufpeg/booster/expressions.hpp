#ifndef UFPEG_EXPRESSIONS_HPP
#define UFPEG_EXPRESSIONS_HPP

#include <algorithm>

#include "instructions.hpp"
#include "compilercontext.hpp"
#include "compileoptions.hpp"

namespace ufpeg {
    class Expression {
    public:
        virtual ~Expression() = default;

        virtual std::vector<std::shared_ptr<Instruction>> compile(
            CompilerContext &context,
            const CompileOptions &options
        ) const = 0;
    };

    class SequenceExpression: public Expression {
    public:
        SequenceExpression(const std::vector<std::shared_ptr<Expression>> &items):
            items(items) {}

        std::vector<std::shared_ptr<Instruction>> compile(
            CompilerContext &context,
            const CompileOptions &options
        ) const {
            auto commit = std::make_shared<CommitInstruction>(options.success);
            auto abort = std::make_shared<AbortInstruction>(options.failure);

            std::vector<std::shared_ptr<Instruction>> instructions = { abort, commit };

            auto success = commit->get_reference();
            auto failure = abort->get_reference();

            for (auto it = this->items.rbegin(); it != this->items.rend(); ++it) {
                auto entry = std::make_shared<Reference>();
                auto item_instructions = (*it)->compile(context, { entry, success, failure });

                success = entry;

                instructions.insert(
                    instructions.end(),
                    item_instructions.rbegin(),
                    item_instructions.rend()
                );
            }

            instructions.emplace_back(
                std::make_shared<BeginInstruction>(success, options.entry)
            );

            std::reverse(instructions.begin(), instructions.end());

            return instructions;
        }
    private:
        const std::vector<std::shared_ptr<Expression>> items;
    };

    class ChoiceExpression: public Expression {
    public:
        ChoiceExpression(const std::vector<std::shared_ptr<Expression>> &items):
            items(items) {}

        std::vector<std::shared_ptr<Instruction>> compile(
            CompilerContext &context,
            const CompileOptions &options
        ) const {
            if (this->items.empty()) {
                return {
                    std::make_shared<JumpInstruction>(options.failure, options.entry),
                };
            }

            std::vector<std::shared_ptr<Instruction>> instructions;

            auto success = options.success;
            auto failure = options.failure;

            for (auto it = this->items.rbegin(); it != this->items.rend(); ++it) {
                auto entry = std::next(it) == this->items.rend() ?
                    options.entry : std::make_shared<Reference>();
                auto item_instructions = (*it)->compile(context, { entry, success, failure });

                failure = entry;

                instructions.insert(
                    instructions.end(),
                    item_instructions.rbegin(),
                    item_instructions.rend()
                );
            }

            std::reverse(instructions.begin(), instructions.end());

            return instructions;
        }
    private:
        const std::vector<std::shared_ptr<Expression>> items;
    };

    class LiteralExpression: public Expression {
    public:
        LiteralExpression(const std::u32string &literal):
            literal(literal) {}

        std::vector<std::shared_ptr<Instruction>> compile(
            CompilerContext &context,
            const CompileOptions &options
        ) const {
            return {
                std::make_shared<MatchLiteralInstruction>(
                    this->literal, options.success, options.failure, options.entry
                ),
            };
        }
    private:
        const std::u32string literal;
    };

    class RangeExpression: public Expression {
    public:
        RangeExpression(char32_t min, char32_t max):
            min(min), max(max) {}

        std::vector<std::shared_ptr<Instruction>> compile(
            CompilerContext &context,
            const CompileOptions &options
        ) const {
            return {
                std::make_shared<MatchRangeInstruction>(
                    this->min, this->max, options.success, options.failure, options.entry
                ),
            };
        }
    private:
        const char32_t min, max;
    };

    class ZeroOrOneExpression: public Expression {
    public:
        ZeroOrOneExpression(const std::shared_ptr<Expression> &item):
            item(item) {}

        std::vector<std::shared_ptr<Instruction>> compile(
            CompilerContext &context,
            const CompileOptions &options
        ) const {
            return this->item->compile(context, {
                options.entry, options.success, options.success,
            });
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
            const CompileOptions &options
        ) const {
            return this->item->compile(context, { options.entry, options.entry, options.success });
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
            const CompileOptions &options
        ) const {
            SequenceExpression expression({
                this->item,
                std::make_shared<ZeroOrMoreExpression>(this->item),
            });

            return expression.compile(context, options);
        }
    private:
        const std::shared_ptr<Expression> item;
    };

    class NotExpression: public Expression {
    public:
        NotExpression(const std::shared_ptr<Expression> &item):
            item(item) {}

        std::vector<std::shared_ptr<Instruction>> compile(
            CompilerContext &context,
            const CompileOptions &options
        ) const {
            auto entry = std::make_shared<Reference>();

            auto begin = std::make_shared<BeginInstruction>(entry, options.entry);
            auto abort_success = std::make_shared<AbortInstruction>(options.success);
            auto abort_failure = std::make_shared<AbortInstruction>(options.failure);

            auto instructions = this->item->compile(
                context, {
                    entry,
                    abort_failure->get_reference(),
                    abort_success->get_reference(),
                }
            );

            instructions.emplace(instructions.begin(), begin);
            instructions.emplace_back(abort_failure);
            instructions.emplace_back(abort_success);

            return instructions;
        }
    private:
        const std::shared_ptr<Expression> item;
    };

    class AndExpression: public Expression {
    public:
        AndExpression(const std::shared_ptr<Expression> &item):
            item(item) {}

        std::vector<std::shared_ptr<Instruction>> compile(
            CompilerContext &context,
            const CompileOptions &options
        ) const {
            auto entry = std::make_shared<Reference>();

            auto begin = std::make_shared<BeginInstruction>(entry, options.entry);
            auto abort_success = std::make_shared<AbortInstruction>(options.success);
            auto abort_failure = std::make_shared<AbortInstruction>(options.failure);

            auto instructions = this->item->compile(
                context, {
                    entry,
                    abort_success->get_reference(),
                    abort_failure->get_reference(),
                }
            );

            instructions.emplace(instructions.begin(), begin);
            instructions.emplace_back(abort_success);
            instructions.emplace_back(abort_failure);

            return instructions;
        }
    private:
        const std::shared_ptr<Expression> item;
    };

    class RuleReferenceExpression: public Expression {
    public:
        RuleReferenceExpression(const std::u32string &name):
            name(name) {}

        std::vector<std::shared_ptr<Instruction>> compile(
            CompilerContext &context,
            const CompileOptions &options
        ) const {
            std::shared_ptr<Reference> target;

            try {
                target = context.references.at(this->name);
            } catch (const std::out_of_range&) {
                target = std::make_shared<Reference>();
                context.references.emplace(this->name, target);
            }

            return {
                std::make_shared<InvokeInstruction>(
                    target, options.success, options.failure, options.entry
                ),
            };
        }
    private:
        const std::u32string name;
    };

    class RuleDefinitionExpression: public Expression {
    public:
        RuleDefinitionExpression(
            const std::u32string &name,
            const std::shared_ptr<Expression> &item
        ):
            name(name), item(item) {}

        std::vector<std::shared_ptr<Instruction>> compile(
            CompilerContext &context,
            const CompileOptions &options
        ) const {
            std::shared_ptr<Reference> entry;

            try {
                entry = context.references.at(this->name);
            } catch (const std::out_of_range&) {
                entry = std::make_shared<Reference>();
                context.references.emplace(this->name, entry);
            }

            auto target = std::make_shared<Reference>();

            auto prepare = std::make_shared<PrepareInstruction>(target, entry);
            auto revoke_success = std::make_shared<RevokeSuccessInstruction>();
            auto revoke_failure = std::make_shared<RevokeFailureInstruction>();
            auto consume = std::make_shared<ConsumeInstruction>(
                this->name, revoke_success->get_reference()
            );
            auto discard = std::make_shared<DiscardInstruction>(revoke_failure->get_reference());

            auto instructions = this->item->compile(
                context, {
                    target,
                    consume->get_reference(),
                    discard->get_reference(),
                }
            );

            instructions.emplace(instructions.begin(), prepare);
            instructions.insert(instructions.end(), {
                consume, discard, revoke_success, revoke_failure,
            });

            return instructions;
        }
    private:
        const std::u32string name;
        const std::shared_ptr<Expression> item;
    };
}

#endif
