#ifndef UFPEG_INSTRUCTIONS_HPP
#define UFPEG_INSTRUCTIONS_HPP

#include <memory>

#include "executorcontext.hpp"

namespace ufpeg {
    class AtomicInstruction;

    class Instruction {
    public:
        virtual ~Instruction() = default;

        virtual std::shared_ptr<Instruction> translate(std::size_t) const = 0;

        virtual std::vector<std::shared_ptr<AtomicInstruction>> flatten() const = 0;
    };

    class AtomicInstruction: public Instruction {
    public:
        AtomicInstruction(const std::u32string &label = {}):
            label(label) {}

        virtual void execute(ExecutorContext&) const = 0;

        const std::u32string &get_label() const {
            return this->label;
        }
    private:
        const std::u32string label;
    };

    class InvokeInstruction: public AtomicInstruction {
    public:
        InvokeInstruction(const std::u32string &target, const std::u32string &label = {}):
            AtomicInstruction(label), target(target) {}

        std::shared_ptr<Instruction> translate(std::size_t) const {
            return std::make_shared<InvokeInstruction>(*this);
        }

        std::vector<std::shared_ptr<AtomicInstruction>> flatten() const {
            return { std::make_shared<InvokeInstruction>(*this) };
        }

        void execute(ExecutorContext &context) const {
            context.pointers.top()++;

            //context.pointers.push(this->pointer);
        }
    private:
        const std::u32string target;
    };

    class RevokeInstruction: public AtomicInstruction {
    public:
        std::shared_ptr<Instruction> translate(std::size_t) const {
            return std::make_shared<RevokeInstruction>(*this);
        }

        std::vector<std::shared_ptr<AtomicInstruction>> flatten() const {
            return { std::make_shared<RevokeInstruction>(*this) };
        }

        void execute(ExecutorContext &context) const {
            context.pointers.pop();
        }
    };

    class PrepareInstruction: public AtomicInstruction {
    public:
        std::shared_ptr<Instruction> translate(std::size_t) const {
            return std::make_shared<PrepareInstruction>(*this);
        }

        std::vector<std::shared_ptr<AtomicInstruction>> flatten() const {
            return { std::make_shared<PrepareInstruction>(*this) };
        }

        void execute(ExecutorContext &context) const {
            context.nodes.push({ nullptr, context.cursors.top() });

            context.pointers.top()++;
        }
    };

    class ConsumeInstruction: public AtomicInstruction {
    public:
        ConsumeInstruction(const std::u32string &name, const std::u32string &label = {}):
            AtomicInstruction(label), name(name) {}

        std::shared_ptr<Instruction> translate(std::size_t) const {
            return std::make_shared<ConsumeInstruction>(*this);
        }

        std::vector<std::shared_ptr<AtomicInstruction>> flatten() const {
            return { std::make_shared<ConsumeInstruction>(*this) };
        }

        void execute(ExecutorContext &context) const {
            auto child = std::move(context.nodes.top());
            context.nodes.pop();
            child.name = this->name.c_str();
            child.stop = context.cursors.top();
            auto &parent = context.nodes.top();
            parent.children.push_back(child);

            context.pointers.top()++;
        }
    private:
        const std::u32string name;
    };

    class DiscardInstruction: public AtomicInstruction {
    public:
        std::shared_ptr<Instruction> translate(std::size_t) const {
            return std::make_shared<DiscardInstruction>(*this);
        }

        std::vector<std::shared_ptr<AtomicInstruction>> flatten() const {
            return { std::make_shared<DiscardInstruction>(*this) };
        }

        void execute(ExecutorContext &context) const {
            context.nodes.pop();

            context.pointers.top()++;
        }
    };

    class BeginInstruction: public AtomicInstruction {
    public:
        std::shared_ptr<Instruction> translate(std::size_t) const {
            return std::make_shared<BeginInstruction>(*this);
        }

        std::vector<std::shared_ptr<AtomicInstruction>> flatten() const {
            return { std::make_shared<BeginInstruction>(*this) };
        }

        void execute(ExecutorContext &context) const {
            auto cursor = context.cursors.top();
            context.cursors.push(cursor);

            context.pointers.top()++;
        }
    };

    class CommitInstruction: public AtomicInstruction {
    public:
        std::shared_ptr<Instruction> translate(std::size_t) const {
            return std::make_shared<CommitInstruction>(*this);
        }

        std::vector<std::shared_ptr<AtomicInstruction>> flatten() const {
            return { std::make_shared<CommitInstruction>(*this) };
        }

        void execute(ExecutorContext &context) const {
            auto cursor = context.cursors.top();
            context.cursors.pop();
            context.cursors.top() = cursor;

            context.pointers.top()++;
        }
    };

    class AbortInstruction: public AtomicInstruction {
    public:
        std::shared_ptr<Instruction> translate(std::size_t) const {
            return std::make_shared<AbortInstruction>(*this);
        }

        std::vector<std::shared_ptr<AtomicInstruction>> flatten() const {
            return { std::make_shared<AbortInstruction>(*this) };
        }

        void execute(ExecutorContext &context) const {
            context.cursors.pop();

            context.pointers.top()++;
        }
    };

    class MatchLiteralInstruction: public AtomicInstruction {
    public:
        MatchLiteralInstruction(const std::u32string &literal, const std::u32string &label = {}):
            AtomicInstruction(label), literal(literal) {}

        std::shared_ptr<Instruction> translate(std::size_t) const {
            return std::make_shared<MatchLiteralInstruction>(*this);
        }

        std::vector<std::shared_ptr<AtomicInstruction>> flatten() const {
            return { std::make_shared<MatchLiteralInstruction>(*this) };
        }

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

    class BranchInstruction: public AtomicInstruction {
    public:
        BranchInstruction(
            std::size_t success,
            std::size_t failure,
            const std::u32string &label = {}
        ):
            AtomicInstruction(label), success(success), failure(failure) {}

        std::shared_ptr<Instruction> translate(std::size_t offset) const {
            return std::make_shared<BranchInstruction>(
                this->success + offset,
                this->failure + offset,
                this->get_label()
            );
        }

        std::vector<std::shared_ptr<AtomicInstruction>> flatten() const {
            return { std::make_shared<BranchInstruction>(*this) };
        }

        void execute(ExecutorContext &context) const {
            auto &pointer = context.pointers.top();
            pointer = context.has_matched ? this->success : this->failure;
        }
    private:
        const std::size_t success, failure;
    };

    class JumpInstruction: public AtomicInstruction {
    public:
        JumpInstruction(std::size_t pointer, const std::u32string &label = {}):
            AtomicInstruction(label), pointer(pointer) {}

        std::shared_ptr<Instruction> translate(std::size_t offset) const {
            return std::make_shared<JumpInstruction>(
                this->pointer + offset,
                this->get_label()
            );
        }

        std::vector<std::shared_ptr<AtomicInstruction>> flatten() const {
            return { std::make_shared<JumpInstruction>(*this) };
        }

        void execute(ExecutorContext &context) const {
            context.pointers.top() = this->pointer;
        }
    private:
        const std::size_t pointer;
    };

    class PassInstruction: public AtomicInstruction {
    public:
        std::shared_ptr<Instruction> translate(std::size_t) const {
            return std::make_shared<PassInstruction>(*this);
        }

        std::vector<std::shared_ptr<AtomicInstruction>> flatten() const {
            return { std::make_shared<PassInstruction>(*this) };
        }

        void execute(ExecutorContext &context) const {
            context.pointers.top()++;
        }
    };

    class FlipInstruction: public AtomicInstruction {
    public:
        std::shared_ptr<Instruction> translate(std::size_t) const {
            return std::make_shared<FlipInstruction>(*this);
        }

        std::vector<std::shared_ptr<AtomicInstruction>> flatten() const {
            return { std::make_shared<FlipInstruction>(*this) };
        }

        void execute(ExecutorContext &context) const {
            context.has_matched = !context.has_matched;

            context.pointers.top()++;
        }
    };

    class ExpectInstruction: public AtomicInstruction {
    public:
        ExpectInstruction(const std::u32string &name, const std::u32string &label = {}):
            AtomicInstruction(label), name(name) {}

        std::shared_ptr<Instruction> translate(std::size_t) const {
            return std::make_shared<ExpectInstruction>(*this);
        }

        std::vector<std::shared_ptr<AtomicInstruction>> flatten() const {
            return { std::make_shared<ExpectInstruction>(*this) };
        }

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

    class CompoundInstruction: public Instruction {
    public:
        CompoundInstruction(const std::vector<std::shared_ptr<Instruction>> &instructions):
            instructions(instructions) {}

        std::shared_ptr<Instruction> translate(std::size_t offset) const {
            std::vector<std::shared_ptr<Instruction>> instructions;

            for (auto instruction: this->instructions) {
                instructions.emplace_back(
                    instruction->translate(offset)
                );
            }

            return std::make_shared<CompoundInstruction>(instructions);
        }

        std::vector<std::shared_ptr<AtomicInstruction>> flatten() const {
            std::vector<std::shared_ptr<AtomicInstruction>> result;
            std::size_t offset = 0;

            for (auto instruction: this->instructions) {
                auto parent = instruction->translate(offset);
                auto children = parent->flatten();

                for (auto child: children) {
                    result.emplace_back(child);
                }

                offset += children.size();
            }

            return result;
        }
    private:
        const std::vector<std::shared_ptr<Instruction>> instructions;
    };
}

#endif
