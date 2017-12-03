#ifndef UFPEG_INSTRUCTIONS_HPP
#define UFPEG_INSTRUCTIONS_HPP

#include "context.hpp"

namespace ufpeg {
    class BaseInstruction {
    public:
        virtual void update(Context&) const = 0;

        virtual ~BaseInstruction() = default;
    };

    class InvokeInstruction: public BaseInstruction {
    public:
        InvokeInstruction(std::size_t pointer):
            pointer(pointer) {}

        void update(Context &context) const {
            context.pointers.top()++;

            context.pointers.push(this->pointer);
        }
    private:
        const std::size_t pointer;
    };

    class RevokeInstruction: public BaseInstruction {
    public:
        void update(Context &context) const {
            context.pointers.pop();
        }
    };

    class PrepareInstruction: public BaseInstruction {
    public:
        void update(Context &context) const {
            context.nodes.push({ nullptr, context.cursors.top() });

            context.pointers.top()++;
        }
    };

    class ConsumeInstruction: public BaseInstruction {
    public:
        ConsumeInstruction(const std::u32string &name):
            name(name) {}

        void update(Context &context) const {
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

    class DiscardInstruction: public BaseInstruction {
    public:
        void update(Context &context) const {
            context.nodes.pop();

            context.pointers.top()++;
        }
    };

    class BeginInstruction: public BaseInstruction {
    public:
        void update(Context &context) const {
            auto cursor = context.cursors.top();
            context.cursors.push(cursor);

            context.pointers.top()++;
        }
    };

    class CommitInstruction: public BaseInstruction {
    public:
        void update(Context &context) const {
            auto cursor = context.cursors.top();
            context.cursors.pop();
            context.cursors.top() = cursor;

            context.pointers.top()++;
        }
    };

    class AbortInstruction: public BaseInstruction {
    public:
        void update(Context &context) const {
            context.cursors.pop();

            context.pointers.top()++;
        }
    };

    class MatchLiteralInstruction: public BaseInstruction {
    public:
        MatchLiteralInstruction(const std::u32string &literal):
            literal(literal) {}

        void update(Context &context) const {
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

    class BranchInstruction: public BaseInstruction {
    public:
        BranchInstruction(std::size_t success, std::size_t failure):
            success(success), failure(failure) {}

        void update(Context &context) const {
            auto &pointer = context.pointers.top();
            pointer = context.has_matched ? this->success : this->failure;
        }
    private:
        const std::size_t success, failure;
    };

    class JumpInstruction: public BaseInstruction {
    public:
        JumpInstruction(std::size_t pointer):
            pointer(pointer) {}

        void update(Context &context) const {
            context.pointers.top() = this->pointer;
        }
    private:
        const std::size_t pointer;
    };

    class PassInstruction: public BaseInstruction {
    public:
        void update(Context &context) const {
            context.pointers.top()++;
        }
    };

    class FlipInstruction: public BaseInstruction {
    public:
        void update(Context &context) const {
            context.has_matched = !context.has_matched;

            context.pointers.top()++;
        }
    };

    class ExpectInstruction: public BaseInstruction {
    public:
        ExpectInstruction(const std::u32string &name):
            name(name) {}

        void update(Context &context) const {
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
