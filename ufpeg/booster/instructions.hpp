#ifndef UFPEG_INSTRUCTIONS_HPP
#define UFPEG_INSTRUCTIONS_HPP

#include <memory>

#include "reference.hpp"
#include "executorcontext.hpp"

namespace ufpeg {
    class Instruction {
    public:
        Instruction(const std::shared_ptr<Reference> &reference = {}):
            reference(reference ? reference : std::make_shared<Reference>()) {}

        virtual ~Instruction() = default;

        virtual void update(ExecutorContext &context) const = 0;

        const std::shared_ptr<Reference> &get_reference() const {
            return this->reference;
        }
    private:
        const std::shared_ptr<Reference> reference;
    };

    class InvokeInstruction: public Instruction {
    public:
        InvokeInstruction(
            const std::shared_ptr<Reference> &target,
            const std::shared_ptr<Reference> &success,
            const std::shared_ptr<Reference> &failure,
            const std::shared_ptr<Reference> &reference = {}
        ):
            Instruction(reference), target(target), success(success), failure(failure) {}

        void update(ExecutorContext &context) const {
            context.pointer = this->target->get_offset();

            context.frames.push({
                this->success->get_offset(),
                this->failure->get_offset(),
            });
        }
    private:
        const std::shared_ptr<Reference> target, success, failure;
    };

    class RevokeSuccessInstruction: public Instruction {
    public:
        void update(ExecutorContext &context) const {
            context.pointer = context.frames.top().success;

            context.frames.pop();
        }
    };

    class RevokeFailureInstruction: public Instruction {
    public:
        void update(ExecutorContext &context) const {
            context.pointer = context.frames.top().failure;

            context.frames.pop();
        }
    };

    class PrepareInstruction: public Instruction {
    public:
        PrepareInstruction(
            const std::shared_ptr<Reference> &target,
            const std::shared_ptr<Reference> &reference = {}
        ):
            Instruction(reference), target(target) {}

        void update(ExecutorContext &context) const {
            context.nodes.push({ nullptr, context.cursors.top() });

            context.pointer = this->target->get_offset();
        }
    private:
        const std::shared_ptr<Reference> target;
    };

    class ConsumeInstruction: public Instruction {
    public:
        ConsumeInstruction(
            const std::u32string &name,
            const std::shared_ptr<Reference> &target,
            const std::shared_ptr<Reference> &reference = {}
        ):
            Instruction(reference), name(name), target(target) {}

        void update(ExecutorContext &context) const {
            auto child = std::move(context.nodes.top());
            context.nodes.pop();
            child.name = this->name.c_str();
            child.stop = context.cursors.top();
            auto &parent = context.nodes.top();
            parent.children.emplace_back(child);

            context.pointer = this->target->get_offset();
        }
    private:
        const std::u32string name;
        const std::shared_ptr<Reference> target;
    };

    class DiscardInstruction: public Instruction {
    public:
        DiscardInstruction(
            const std::shared_ptr<Reference> &target,
            const std::shared_ptr<Reference> &reference = {}
        ):
            Instruction(reference), target(target) {}

        void update(ExecutorContext &context) const {
            context.nodes.pop();

            context.pointer = this->target->get_offset();
        }
    private:
        const std::shared_ptr<Reference> target;
    };

    class BeginInstruction: public Instruction {
    public:
        BeginInstruction(
            const std::shared_ptr<Reference> &target,
            const std::shared_ptr<Reference> &reference = {}
        ):
            Instruction(reference), target(target) {}

        void update(ExecutorContext &context) const {
            auto cursor = context.cursors.top();
            context.cursors.push(cursor);

            context.pointer = this->target->get_offset();
        }
    private:
        const std::shared_ptr<Reference> target;
    };

    class CommitInstruction: public Instruction {
    public:
        CommitInstruction(
            const std::shared_ptr<Reference> &target,
            const std::shared_ptr<Reference> &reference = {}
        ):
            Instruction(reference), target(target) {}

        void update(ExecutorContext &context) const {
            auto cursor = context.cursors.top();
            context.cursors.pop();
            context.cursors.top() = cursor;

            context.pointer = this->target->get_offset();
        }
    private:
        const std::shared_ptr<Reference> target;
    };

    class AbortInstruction: public Instruction {
    public:
        AbortInstruction(
            const std::shared_ptr<Reference> &target,
            const std::shared_ptr<Reference> &reference = {}
        ):
            Instruction(reference), target(target) {}

        void update(ExecutorContext &context) const {
            context.cursors.pop();

            context.pointer = this->target->get_offset();
        }
    private:
        const std::shared_ptr<Reference> target;
    };

    class MatchLiteralInstruction: public Instruction {
    public:
        MatchLiteralInstruction(
            const std::u32string &literal,
            const std::shared_ptr<Reference> &success,
            const std::shared_ptr<Reference> &failure,
            const std::shared_ptr<Reference> &reference = {}
        ):
            Instruction(reference), literal(literal), success(success), failure(failure) {}

        void update(ExecutorContext &context) const {
            auto &cursor = context.cursors.top();
            const auto length = this->literal.length();

            if (context.text.compare(cursor, length, this->literal)) {
                context.pointer = this->failure->get_offset();
            } else {
                cursor += length;
                context.pointer = this->success->get_offset();
            }
        }
    private:
        const std::u32string literal;
        const std::shared_ptr<Reference> success, failure;
    };

    class MatchRangeInstruction: public Instruction {
    public:
        MatchRangeInstruction(
            char32_t min, char32_t max,
            const std::shared_ptr<Reference> &success,
            const std::shared_ptr<Reference> &failure,
            const std::shared_ptr<Reference> &reference = {}
        ):
            Instruction(reference), min(min), max(max), success(success), failure(failure) {}

        void update(ExecutorContext &context) const {
            auto &cursor = context.cursors.top();
            const auto code = context.text.at(cursor);

            if (this->min <= code && code <= this->max) {
                cursor++;
                context.pointer = this->success->get_offset();
            } else {
                context.pointer = this->failure->get_offset();
            }
        }
    private:
        const char32_t min, max;
        const std::shared_ptr<Reference> success, failure;
    };

    class JumpInstruction: public Instruction {
    public:
        JumpInstruction(
            const std::shared_ptr<Reference> &target,
            const std::shared_ptr<Reference> &reference = {}
        ):
            Instruction(reference), target(target) {}

        void update(ExecutorContext &context) const {
            context.pointer = this->target->get_offset();
        }
    private:
        const std::shared_ptr<Reference> target;
    };

    class ExpectInstruction: public Instruction {
    public:
        ExpectInstruction(
            const std::u32string &name,
            const std::shared_ptr<Reference> &target,
            const std::shared_ptr<Reference> &reference = {}
        ):
            Instruction(reference), name(name), target(target) {}

        void update(ExecutorContext &context) const {
            auto cursor = context.cursors.top();
            if (cursor > context.offset) {
                context.expectations.clear();
                context.offset = cursor;
            }
            context.expectations.push_back(this->name.c_str());

            context.pointer = this->target->get_offset();
        }
    private:
        const std::u32string name;
        const std::shared_ptr<Reference> target;
    };
}

#endif
