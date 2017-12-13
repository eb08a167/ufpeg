#ifndef UFPEG_INSTRUCTIONS_HPP
#define UFPEG_INSTRUCTIONS_HPP

#include "executorcontext.hpp"

namespace ufpeg {
    class BaseSimpleInstruction;

    class BaseInstruction {
    public:
        virtual ~BaseInstruction() = default;

        virtual BaseInstruction *translate(int) const = 0;

        virtual std::vector<std::shared_ptr<BaseSimpleInstruction>> flatten() const = 0;
    };

    class BaseSimpleInstruction: public BaseInstruction {
    public:
        BaseSimpleInstruction(const std::u32string &label = {}):
            label(label) {}

        virtual void execute(ExecutorContext&) const = 0;

        const std::u32string &get_label() const {
            return this->label;
        }
    private:
        const std::u32string label;
    };

    class InvokeInstruction: public BaseSimpleInstruction {
    public:
        InvokeInstruction(std::size_t pointer):
            pointer(pointer) {}

        BaseInstruction *translate(int offset) const {
            return new InvokeInstruction(this->pointer);
        }

        std::vector<std::shared_ptr<BaseSimpleInstruction>> flatten() const {
            return { std::make_shared<InvokeInstruction>(this->pointer) };
        }

        void execute(ExecutorContext &context) const {
            context.pointers.top()++;

            context.pointers.push(this->pointer);
        }
    private:
        const std::size_t pointer;
    };

    class RevokeInstruction: public BaseSimpleInstruction {
    public:
        BaseInstruction *translate(int offset) const {
            return new RevokeInstruction();
        }

        std::vector<std::shared_ptr<BaseSimpleInstruction>> flatten() const {
            return { std::make_shared<RevokeInstruction>() };
        }

        void execute(ExecutorContext &context) const {
            context.pointers.pop();
        }
    };

    class PrepareInstruction: public BaseSimpleInstruction {
    public:
        BaseInstruction *translate(int offset) const {
            return new PrepareInstruction();
        }

        std::vector<std::shared_ptr<BaseSimpleInstruction>> flatten() const {
            return { std::make_shared<PrepareInstruction>() };
        }

        void execute(ExecutorContext &context) const {
            context.nodes.push({ nullptr, context.cursors.top() });

            context.pointers.top()++;
        }
    };

    class ConsumeInstruction: public BaseSimpleInstruction {
    public:
        ConsumeInstruction(const std::u32string &name):
            name(name) {}

        BaseInstruction *translate(int offset) const {
            return new ConsumeInstruction(this->name);
        }

        std::vector<std::shared_ptr<BaseSimpleInstruction>> flatten() const {
            return { std::make_shared<ConsumeInstruction>(this->name) };
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

    class DiscardInstruction: public BaseSimpleInstruction {
    public:
        BaseInstruction *translate(int offset) const {
            return new DiscardInstruction();
        }

        std::vector<std::shared_ptr<BaseSimpleInstruction>> flatten() const {
            return { std::make_shared<DiscardInstruction>() };
        }

        void execute(ExecutorContext &context) const {
            context.nodes.pop();

            context.pointers.top()++;
        }
    };

    class BeginInstruction: public BaseSimpleInstruction {
    public:
        BaseInstruction *translate(int offset) const {
            return new BeginInstruction();
        }

        std::vector<std::shared_ptr<BaseSimpleInstruction>> flatten() const {
            return { std::make_shared<BeginInstruction>() };
        }

        void execute(ExecutorContext &context) const {
            auto cursor = context.cursors.top();
            context.cursors.push(cursor);

            context.pointers.top()++;
        }
    };

    class CommitInstruction: public BaseSimpleInstruction {
    public:
        BaseInstruction *translate(int offset) const {
            return new CommitInstruction();
        }

        std::vector<std::shared_ptr<BaseSimpleInstruction>> flatten() const {
            return { std::make_shared<CommitInstruction>() };
        }

        void execute(ExecutorContext &context) const {
            auto cursor = context.cursors.top();
            context.cursors.pop();
            context.cursors.top() = cursor;

            context.pointers.top()++;
        }
    };

    class AbortInstruction: public BaseSimpleInstruction {
    public:
        BaseInstruction *translate(int offset) const {
            return new AbortInstruction();
        }

        std::vector<std::shared_ptr<BaseSimpleInstruction>> flatten() const {
            return { std::make_shared<AbortInstruction>() };
        }

        void execute(ExecutorContext &context) const {
            context.cursors.pop();

            context.pointers.top()++;
        }
    };

    class MatchLiteralInstruction: public BaseSimpleInstruction {
    public:
        MatchLiteralInstruction(const std::u32string &literal):
            literal(literal) {}

        BaseInstruction *translate(int offset) const {
            return new MatchLiteralInstruction(this->literal);
        }

        std::vector<std::shared_ptr<BaseSimpleInstruction>> flatten() const {
            return { std::make_shared<MatchLiteralInstruction>(this->literal) };
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

    class BranchInstruction: public BaseSimpleInstruction {
    public:
        BranchInstruction(std::size_t success, std::size_t failure):
            success(success), failure(failure) {}

        BaseInstruction *translate(int offset) const {
            return new BranchInstruction(
                this->success + offset,
                this->failure + offset
            );
        }

        std::vector<std::shared_ptr<BaseSimpleInstruction>> flatten() const {
            return { std::make_shared<BranchInstruction>(this->success, this->failure) };
        }

        void execute(ExecutorContext &context) const {
            auto &pointer = context.pointers.top();
            pointer = context.has_matched ? this->success : this->failure;
        }
    private:
        const std::size_t success, failure;
    };

    class JumpInstruction: public BaseSimpleInstruction {
    public:
        JumpInstruction(std::size_t pointer):
            pointer(pointer) {}

        BaseInstruction *translate(int offset) const {
            return new JumpInstruction(this->pointer + offset);
        }

        std::vector<std::shared_ptr<BaseSimpleInstruction>> flatten() const {
            return { std::make_shared<JumpInstruction>(this->pointer) };
        }

        void execute(ExecutorContext &context) const {
            context.pointers.top() = this->pointer;
        }
    private:
        const std::size_t pointer;
    };

    class PassInstruction: public BaseSimpleInstruction {
    public:
        BaseInstruction *translate(int offset) const {
            return new PassInstruction();
        }

        std::vector<std::shared_ptr<BaseSimpleInstruction>> flatten() const {
            return { std::make_shared<PassInstruction>() };
        }

        void execute(ExecutorContext &context) const {
            context.pointers.top()++;
        }
    };

    class FlipInstruction: public BaseSimpleInstruction {
    public:
        BaseInstruction *translate(int offset) const {
            return new FlipInstruction();
        }

        std::vector<std::shared_ptr<BaseSimpleInstruction>> flatten() const {
            return { std::make_shared<FlipInstruction>() };
        }

        void execute(ExecutorContext &context) const {
            context.has_matched = !context.has_matched;

            context.pointers.top()++;
        }
    };

    class ExpectInstruction: public BaseSimpleInstruction {
    public:
        ExpectInstruction(const std::u32string &name):
            name(name) {}

        BaseInstruction *translate(int offset) const {
            return new ExpectInstruction(this->name);
        }

        std::vector<std::shared_ptr<BaseSimpleInstruction>> flatten() const {
            return { std::make_shared<ExpectInstruction>(this->name) };
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

    class CompoundInstruction: public BaseInstruction {
    public:
        CompoundInstruction(const std::vector<std::shared_ptr<BaseInstruction>> &instructions):
            instructions(instructions) {}

        BaseInstruction *translate(int offset) const {
            std::vector<std::shared_ptr<BaseInstruction>> instructions;

            for (auto instruction: this->instructions) {
                instructions.emplace_back(
                    instruction->translate(offset)
                );
            }

            return new CompoundInstruction(instructions);
        }

        std::vector<std::shared_ptr<BaseSimpleInstruction>> flatten() const {
            std::vector<std::shared_ptr<BaseSimpleInstruction>> result;

            for (auto parent: this->instructions) {
                for (auto child: parent->flatten()) {
                    result.emplace_back(child);
                }
            }

            return result;
        }
    private:
        const std::vector<std::shared_ptr<BaseInstruction>> instructions;
    };
}

#endif
