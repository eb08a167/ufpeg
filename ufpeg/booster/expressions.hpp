#ifndef UFPEG_EXPRESSIONS_HPP
#define UFPEG_EXPRESSIONS_HPP

#include "compilercontext.hpp"

namespace ufpeg {
    class BaseExpression {
    public:
        virtual ~BaseExpression() = default;

        virtual void compile(CompilerContext&) const = 0;
    };

    class LiteralExpression: public BaseExpression {
    public:
        LiteralExpression(const std::u32string &literal):
            literal(literal) {}

        void compile(CompilerContext &context) const {
        }
    private:
        const std::u32string literal;
    };
}

#endif
