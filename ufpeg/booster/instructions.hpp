#ifndef UFPEG_INSTRUCTIONS_HPP
#define UFPEG_INSTRUCTIONS_HPP

#include <memory>

#include "reference.hpp"
#include "executorcontext.hpp"

#include <iostream>
#include <iomanip>
#include <locale>
#include <codecvt>

std::string u32tou8(const std::u32string &text) {
    std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> cvt;
    return cvt.to_bytes(text);
}

namespace ufpeg {
    class Instruction {
    public:
        Instruction(const std::shared_ptr<Reference> &reference = {}):
            reference(reference ? reference : std::make_shared<Reference>()) {}

        virtual ~Instruction() = default;

        virtual void update(ExecutorContext &context) const = 0;

        virtual void print() const = 0;

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

        void print() const {
            std::cout << "INVOKE " << this->target->get_offset() << " " << this->success->get_offset() << " " << this->failure->get_offset() << std::endl;
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

        void print() const {
            std::cout << "REVOKE_SUCCESS" << std::endl;
        }
    };

    class RevokeFailureInstruction: public Instruction {
    public:
        void update(ExecutorContext &context) const {
            context.pointer = context.frames.top().failure;

            context.frames.pop();
        }

        void print() const {
            std::cout << "REVOKE_FAILURE" << std::endl;
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
            context.nodes.push({ {}, context.cursors.top() });

            context.pointer = this->target->get_offset();
        }

        void print() const {
            std::cout << "PREPARE " << this->target->get_offset() << std::endl;
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
            child.name = this->name;
            child.stop = context.cursors.top();
            auto &parent = context.nodes.top();
            parent.children.emplace_back(child);

            context.pointer = this->target->get_offset();
        }

        void print() const {
            std::cout << "CONSUME \"" << u32tou8(this->name) << "\" " << this->target->get_offset() << std::endl;
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

        void print() const {
            std::cout << "DISCARD " << this->target->get_offset() << std::endl;
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

        void print() const {
            std::cout << "BEGIN " << this->target->get_offset() << std::endl;
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

        void print() const {
            std::cout << "COMMIT " << this->target->get_offset() << std::endl;
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

        void print() const {
            std::cout << "ABORT " << this->target->get_offset() << std::endl;
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

            try {
                if (!context.text.compare(cursor, length, this->literal)) {
                    cursor += length;
                    context.pointer = this->success->get_offset();
                    return;
                }
            } catch (const std::out_of_range&) {
            }

            context.pointer = this->failure->get_offset();
        }

        void print() const {
            std::cout << "MATCH_LITERAL \"" << u32tou8(this->literal) << "\" " << this->success->get_offset() << " " << this->failure->get_offset() << std::endl;
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

            try {
                const auto code = context.text.at(cursor);

                if (this->min <= code && code <= this->max) {
                    cursor++;
                    context.pointer = this->success->get_offset();
                    return;
                }
            } catch (const std::out_of_range&) {
            }

            context.pointer = this->failure->get_offset();
        }

        void print() const {
            std::cout << "MATCH_RANGE " << this->min << " " << this->max << " " << this->success->get_offset() << " " << this->failure->get_offset() << std::endl;
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

        void print() const {
            std::cout << "JUMP " << this->target->get_offset() << std::endl;
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
            context.expectations.push_back(this->name);

            context.pointer = this->target->get_offset();
        }

        void print() const {
            std::cout << "EXPECT \"" << u32tou8(this->name) << "\" " << this->target->get_offset() << std::endl;
        }
    private:
        const std::u32string name;
        const std::shared_ptr<Reference> target;
    };
}

#endif
