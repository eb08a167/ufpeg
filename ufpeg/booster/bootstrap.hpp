#ifndef UFPEG_BOOTSTRAP_HPP
#define UFPEG_BOOTSTRAP_HPP

#include "expressions.hpp"

namespace ufpeg {
    std::shared_ptr<Expression> bootstrap() {
        using namespace std;

        auto escape_sequence = []() {
            vector<shared_ptr<Expression>> items = {
                make_shared<LiteralExpression>(U"u"),
                make_shared<OneOrMoreExpression>(
                    make_shared<RuleReferenceExpression>(U"hexadecimal-digit")
                ),
            };

            return make_shared<RuleDefinitionExpression>(
                U"escape-sequence",
                make_shared<SequenceExpression>(items)
            );
        };

        auto binary_digit = []() {
            return make_shared<RuleDefinitionExpression>(
                U"binary-digit",
                make_shared<RangeExpression>(U'0', U'1')
            );
        };

        auto quaternary_digit = []() {
            return make_shared<RuleDefinitionExpression>(
                U"quaternary-digit",
                make_shared<RangeExpression>(U'0', U'3')
            );
        };

        auto octal_digit = []() {
            return make_shared<RuleDefinitionExpression>(
                U"octal-digit",
                make_shared<RangeExpression>(U'0', U'7')
            );
        };

        auto hexadecimal_digit = []() {
            vector<shared_ptr<Expression>> items = {
                make_shared<RangeExpression>(U'0', U'9'),
                make_shared<RangeExpression>(U'A', U'F'),
            };

            return make_shared<RuleDefinitionExpression>(
                U"hexadecimal-digit",
                make_shared<ChoiceExpression>(items)
            );
        };

        auto white_space = []() {
            vector<shared_ptr<Expression>> items = {
                make_shared<RangeExpression>(U'\t', U'\r'),
                make_shared<LiteralExpression>(U" "),
                make_shared<LiteralExpression>(U"\u0085"),
                make_shared<LiteralExpression>(U"\u00A0"),
                make_shared<LiteralExpression>(U"\u1680"),
                make_shared<RangeExpression>(U'\u2000', U'\u200A'),
                make_shared<RangeExpression>(U'\u2028', U'\u2029'),
                make_shared<LiteralExpression>(U"\u202F"),
                make_shared<LiteralExpression>(U"\u205F"),
                make_shared<LiteralExpression>(U"\u3000"),
            };

            return make_shared<RuleDefinitionExpression>(
                U"white-space",
                make_shared<ChoiceExpression>(items)
            );
        };

        auto character = []() {
            return make_shared<RuleDefinitionExpression>(
                U"character",
                make_shared<RangeExpression>(U'\0', U'\U0010FFFF')
            );
        };

        vector<shared_ptr<Expression>> items = {
            escape_sequence(),
            binary_digit(),
            quaternary_digit(),
            octal_digit(),
            hexadecimal_digit(),
            white_space(),
            character(),
        };

        return make_shared<GrammarExpression>(items);
    }
}

#endif
