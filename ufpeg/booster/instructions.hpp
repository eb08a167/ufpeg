#ifndef UFPEG_INSTRUCTIONS_HPP
#define UFPEG_INSTRUCTIONS_HPP

#include "opcode.hpp"
#include "compilercontext.hpp"

namespace ufpeg {
    struct RawInstruction {
        OpCode op_code;
        std::u32string name, literal;
        std::size_t target, success, failure;
    };

    class SmartInstruction {
    public:
        SmartInstruction(const std::shared_ptr<Reference> &reference = {}):
            reference(reference ? reference : std::make_shared<Reference>()) {}

        virtual ~SmartInstruction() = default;

        virtual RawInstruction to_raw(CompilerContext&) const = 0;

        const std::shared_ptr<Reference> &get_reference() const {
            return this->reference;
        }
    private:
        const std::shared_ptr<Reference> reference;
    };

    class InvokeInstruction: public SmartInstruction {
    public:
        InvokeInstruction(
            const std::shared_ptr<Reference> &target,
            const std::shared_ptr<Reference> &reference = {}
        ):
            SmartInstruction(reference), target(target) {}

        RawInstruction to_raw(CompilerContext &context) const {
            RawInstruction instruction = { OpCode::INVOKE };

            instruction.target = this->target->get_offset();

            return instruction;
        }
    private:
        const std::shared_ptr<Reference> target;
    };

    class RevokeInstruction: public SmartInstruction {
    public:
        RawInstruction to_raw(CompilerContext&) const {
            return { OpCode::REVOKE };
        }
    };

    class PrepareInstruction: public SmartInstruction {
    public:
        RawInstruction to_raw(CompilerContext&) const {
            return { OpCode::PREPARE };
        }
    };

    class ConsumeInstruction: public SmartInstruction {
    public:
        ConsumeInstruction(
            const std::u32string &name,
            const std::shared_ptr<Reference> &reference = {}
        ):
            SmartInstruction(reference), name(name) {}

        RawInstruction to_raw(CompilerContext&) const {
            RawInstruction instruction = { OpCode::CONSUME };

            instruction.name = this->name;

            return instruction;
        }
    private:
        const std::u32string name;
    };

    class DiscardInstruction: public SmartInstruction {
    public:
        RawInstruction to_raw(CompilerContext&) const {
            return { OpCode::DISCARD };
        }
    };

    class BeginInstruction: public SmartInstruction {
    public:
        RawInstruction to_raw(CompilerContext&) const {
            return { OpCode::BEGIN };
        }
    };

    class CommitInstruction: public SmartInstruction {
    public:
        RawInstruction to_raw(CompilerContext&) const {
            return { OpCode::COMMIT };
        }
    };

    class AbortInstruction: public SmartInstruction {
    public:
        RawInstruction to_raw(CompilerContext&) const {
            return { OpCode::ABORT };
        }
    };

    class MatchLiteralInstruction: public SmartInstruction {
    public:
        MatchLiteralInstruction(
            const std::u32string &literal,
            const std::shared_ptr<Reference> &reference = {}
        ):
            SmartInstruction(reference), literal(literal) {}

        RawInstruction to_raw(CompilerContext&) const {
            RawInstruction instruction = { OpCode::MATCH_LITERAL };

            instruction.literal = this->literal;

            return instruction;
        }
    private:
        const std::u32string literal;
    };

    class BranchInstruction: public SmartInstruction {
    public:
        BranchInstruction(
            const std::shared_ptr<Reference> &success,
            const std::shared_ptr<Reference> &failure,
            const std::shared_ptr<Reference> &reference = {}
        ):
            SmartInstruction(reference), success(success), failure(failure) {}

        RawInstruction to_raw(CompilerContext &context) const {
            RawInstruction instruction = { OpCode::BRANCH };

            instruction.success = this->success->get_offset();
            instruction.failure = this->failure->get_offset();

            return instruction;
        }
    private:
        const std::shared_ptr<Reference> success, failure;
    };

    class JumpInstruction: public SmartInstruction {
    public:
        JumpInstruction(
            const std::shared_ptr<Reference> &target,
            const std::shared_ptr<Reference> &reference = {}
        ):
            SmartInstruction(reference), target(target) {}

        RawInstruction to_raw(CompilerContext &context) const {
            RawInstruction instruction = { OpCode::JUMP };

            instruction.target = this->target->get_offset();

            return instruction;
        }
    private:
        const std::shared_ptr<Reference> &target;
    };

    class PassInstruction: public SmartInstruction {
    public:
        RawInstruction to_raw(CompilerContext&) const {
            return { OpCode::PASS };
        }
    };

    class FlipInstruction: public SmartInstruction {
    public:
        RawInstruction to_raw(CompilerContext&) const {
            return { OpCode::FLIP };
        }
    };

    class ExpectInstruction: public SmartInstruction {
    public:
        ExpectInstruction(
            const std::u32string &name,
            const std::shared_ptr<Reference> &reference = {}
        ):
            SmartInstruction(reference), name(name) {}

        RawInstruction to_raw(CompilerContext&) const {
            RawInstruction instruction = { OpCode::EXPECT };

            instruction.name = this->name;

            return instruction;
        }
    private:
        const std::u32string name;
    };
}

#endif
