#ifndef UFPEG_OPCODE_HPP
#define UFPEG_OPCODE_HPP

namespace ufpeg {
    enum class OpCode {
        INVOKE,
        REVOKE,
        PREPARE,
        CONSUME,
        DISCARD,
        BEGIN,
        COMMIT,
        ABORT,
        MATCH_LITERAL,
        BRANCH,
        JUMP,
        PASS,
        FLIP,
        EXPECT,
    };
}

#endif
