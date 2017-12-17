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

        virtual void execute(ExecutorContext&) const = 0;

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
            const std::shared_ptr<Reference> &reference = {}
        ):
            Instruction(reference), target(target) {}

        void execute(ExecutorContext &context) const {
            context.pointers.top()++;

            //context.pointers.push(this->target);
        }
    private:
        const std::shared_ptr<Reference> &target;
    };

    class RevokeInstruction: public Instruction {
    public:
        void execute(ExecutorContext &context) const {
            context.pointers.pop();
        }
    };

    class PrepareInstruction: public Instruction {
    public:
        void execute(ExecutorContext &context) const {
            context.nodes.push({ context.cursors.top() });

            context.pointers.top()++;
        }
    };

    class ConsumeInstruction: public Instruction {
    public:
        ConsumeInstruction(
            const std::u32string &name,
            const std::shared_ptr<Reference> &reference = {}
        ):
            Instruction(reference), name(name) {}

        void execute(ExecutorContext &context) const {
            auto child = std::move(context.nodes.top());
            context.nodes.pop();
            child.stop = context.cursors.top();
            auto &parent = context.nodes.top();
            parent.children.push_back(child);

            context.pointers.top()++;
        }
    private:
        const std::u32string name;
    };

    class DiscardInstruction: public Instruction {
    public:
        void execute(ExecutorContext &context) const {
            context.nodes.pop();

            context.pointers.top()++;
        }
    };

    class BeginInstruction: public Instruction {
    public:
        void execute(ExecutorContext &context) const {
            auto cursor = context.cursors.top();
            context.cursors.push(cursor);

            context.pointers.top()++;
        }
    };

    class CommitInstruction: public Instruction {
    public:
        void execute(ExecutorContext &context) const {
            auto cursor = context.cursors.top();
            context.cursors.pop();
            context.cursors.top() = cursor;

            context.pointers.top()++;
        }
    };

    class AbortInstruction: public Instruction {
    public:
        void execute(ExecutorContext &context) const {
            context.cursors.pop();

            context.pointers.top()++;
        }
    };

    class MatchLiteralInstruction: public Instruction {
    public:
        MatchLiteralInstruction(
            const std::u32string &literal,
            const std::shared_ptr<Reference> &reference = {}
        ):
            Instruction(reference), literal(literal) {}

        void execute(ExecutorContext &context) const {
            auto &cursor = context.cursors.top();
            const auto length = this->literal.length();

            context.has_matched = !context.text.compare(
                cursor, length, this->literal
            );

            if (context.has_matched) {
                cursor += length;
            }

            context.pointers.top()++;
        }
    private:
        const std::u32string literal;
    };

    class BranchInstruction: public Instruction {
    public:
        BranchInstruction(
            const std::shared_ptr<Reference> &success,
            const std::shared_ptr<Reference> &failure,
            const std::shared_ptr<Reference> &reference = {}
        ):
            Instruction(reference), success(success), failure(failure) {}

        void execute(ExecutorContext &context) const {
            //auto &pointer = context.pointers.top();
            //pointer = context.has_matched ? this->success : this->failure;
        }
    private:
        const std::shared_ptr<Reference> &success, &failure;
    };

    class JumpInstruction: public Instruction {
    public:
        JumpInstruction(
            const std::shared_ptr<Reference> &target,
            const std::shared_ptr<Reference> &reference = {}
        ):
            Instruction(reference), target(target) {}

        void execute(ExecutorContext &context) const {
            //context.pointers.top() = this->pointer;
        }
    private:
        const std::shared_ptr<Reference> &target;
    };

    class PassInstruction: public Instruction {
    public:
        void execute(ExecutorContext &context) const {
            context.pointers.top()++;
        }
    };

    class FlipInstruction: public Instruction {
    public:
        void execute(ExecutorContext &context) const {
            context.has_matched = !context.has_matched;

            context.pointers.top()++;
        }
    };

    class ExpectInstruction: public Instruction {
    public:
        ExpectInstruction(
            const std::u32string &name,
            const std::shared_ptr<Reference> &reference = {}
        ):
            Instruction(reference), name(name) {}

        void execute(ExecutorContext &context) const {
            auto cursor = context.cursors.top();
            if (cursor > context.offset) {
                context.expectations.clear();
                context.offset = cursor;
            }

            context.expectations.push_back(this->name.c_str());

            context.pointers.top()++;
        }
    private:
        const std::u32string name;
    };
}

#endif
