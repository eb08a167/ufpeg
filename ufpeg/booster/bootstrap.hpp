#ifndef UFPEG_BOOTSTRAP_HPP
#define UFPEG_BOOTSTRAP_HPP

#include "expressions.hpp"

/*
grammar
    = space (rule space)+;

rule
    = identifier space "=" space expression space ";";

expression
    = choice-expression;

choice-expression
    = sequence-expression (space "|" space sequence-expression)*;

sequence-expression
    = prefixed-expression (space prefixed-expression)*;

prefixed-expression
    = and-expression
    | not-expression
    | suffixed-expression;

and-expression
    = "&" space suffixed-expression;

not-expression
    = "!" space suffixed-expression;

suffixed-expression
    = zero-or-one-expression
    | zero-or-more-expression
    | one-or-more-expression
    | primary-expression;

zero-or-one-expression
    = primary-expression space "?";

zero-or-more-expression
    = primary-expression space "*";

one-or-more-expression
    = primary-expression space "+";

primary-expression
    =

identifier
    = identifier-start identifier-part*;

identifier-start
    = 'a'..'z'
    | 'A'..'Z'
    | '-';

identifier-part
    = identifier-start
    | '0'..'9';

space
    = white-space*;

character-literal
    = single-quote single-quoted-character single-quote;

string-literal
    = single-quote single-quoted-characters single-quote
    | double-quote double-quoted-characters double-quote;

single-quoted-characters
    = single-quoted-character*;

double-quoted-characters
    = double-quoted-character*;

single-quoted-character
    = !(single-quote | escape) character
    | escaped-character;

double-quoted-character
    = !(double-quote | escape) character
    | escaped-character;

escaped-character
    = escape escape-sequence;

escape-sequence
    = builtin-escape-sequence
    | binary-escape-sequence
    | quaternary-escape-sequence
    | octal-escape-sequence
    | hexadecimal-escape-sequence;

builtin-escape-sequence
    = "\\"
    | "\""
    | "'"
    | "0"
    | "a"
    | "b"
    | "t"
    | "n"
    | "v"
    | "f"
    | "r"
    | "e";

binary-escape-sequence
    = "b" binary-digits;

quaternary-escape-sequence
    = "q" quaternary-digits;

octal-escape-sequence
    = "o" octal-digits;

hexadecimal-escape-sequence
    = "h" hexadecimal-digits;

binary-digits
    = binary-digit+;

quaternary-digits
    = quaternary-digit+;

octal-digits
    = octal-digit+;

hexadecimal-digits
    = hexadecimal-digit+;

binary-digit
    = '0'..'1';

quaternary-digit
    = '0'..'3';

octal-digit
    = '0'..'7';

hexadecimal-digit
    = '0'..'9'
    | 'A'..'F';

single-quote
    = "'";

double-quote
    = '"';

escape
    = '\\';

white-space
    = '\t'..'\r'
    | ' '
    | '\h85'
    | '\hA0'
    | '\h1680'
    | '\h2000'..'\h200A'
    | '\h2028'..'\h2029'
    | '\h202F'
    | '\h205F'
    | '\h3000';

character
    = '\0'..'\h10FFFF';
*/

namespace ufpeg {
    std::shared_ptr<Expression> bootstrap() {
        using namespace std;

        auto string_literal = []() {
            auto pairs = {
                make_pair(U"single-quote", U"single-quoted-characters"),
                make_pair(U"double-quote", U"double-quoted-characters"),
            };

            vector<shared_ptr<Expression>> choices;

            for (const auto &pair: pairs) {
                vector<shared_ptr<Expression>> items = {
                    make_shared<RuleReferenceExpression>(pair.first),
                    make_shared<RuleReferenceExpression>(pair.second),
                    make_shared<RuleReferenceExpression>(pair.first),
                };

                choices.emplace_back(
                    make_shared<SequenceExpression>(items)
                );
            }

            return make_shared<RuleDefinitionExpression>(
                U"string-literal",
                make_shared<ChoiceExpression>(choices)
            );
        };

        auto quoted_characters = [](auto name, auto quote_name) {
            return make_shared<RuleDefinitionExpression>(
                name,
                make_shared<ZeroOrMoreExpression>(
                    make_shared<RuleReferenceExpression>(quote_name)
                )
            );
        };

        auto quoted_character = [](auto name, auto quote_name) {
            vector<shared_ptr<Expression>> not_items = {
                make_shared<RuleReferenceExpression>(quote_name),
                make_shared<RuleReferenceExpression>(U"escape"),
            };

            vector<shared_ptr<Expression>> items = {
                make_shared<NotExpression>(
                    make_shared<ChoiceExpression>(not_items)
                ),
                make_shared<RuleReferenceExpression>(U"character"),
            };

            vector<shared_ptr<Expression>> choices = {
                make_shared<SequenceExpression>(items),
                make_shared<RuleReferenceExpression>(U"escaped-character"),
            };

            return make_shared<RuleDefinitionExpression>(
                name,
                make_shared<ChoiceExpression>(choices)
            );
        };

        auto escaped_character = []() {
            vector<shared_ptr<Expression>> items = {
                make_shared<RuleReferenceExpression>(U"escape"),
                make_shared<RuleReferenceExpression>(U"escape-sequence"),
            };

            return make_shared<RuleDefinitionExpression>(
                U"escaped-character",
                make_shared<SequenceExpression>(items)
            );
        };

        auto escape_sequence = []() {
            auto names = {
                U"binary-escape-sequence",
                U"quaternary-escape-sequence",
                U"octal-escape-sequence",
                U"hexadecimal-escape-sequence",
                U"builtin-escape-sequence",
            };

            vector<shared_ptr<Expression>> items;

            for (auto name: names) {
                items.emplace_back(
                    make_shared<RuleReferenceExpression>(name)
                );
            }

            return make_shared<RuleDefinitionExpression>(
                U"escape-sequence",
                make_shared<ChoiceExpression>(items)
            );
        };

        auto numeric_escape_sequence = [](auto name, auto prefix, auto item) {
            vector<shared_ptr<Expression>> items = {
                make_shared<LiteralExpression>(prefix),
                make_shared<LiteralExpression>(U"{"),
                make_shared<RuleReferenceExpression>(item),
                make_shared<LiteralExpression>(U"}"),
            };

            return make_shared<RuleDefinitionExpression>(
                name,
                make_shared<SequenceExpression>(items)
            );
        };

        auto builtin_escape_sequence = []() {
            auto literals = {
                U"\\", U"\"", U"'",
                U"0", U"a", U"b", U"t", U"n", U"v", U"f", U"r", U"e",
            };

            vector<shared_ptr<Expression>> items;

            for (auto literal: literals) {
                items.emplace_back(
                    make_shared<LiteralExpression>(literal)
                );
            }

            return make_shared<RuleDefinitionExpression>(
                U"builtin-escape-sequence",
                make_shared<ChoiceExpression>(items)
            );
        };

        auto digits = [](auto name, auto item) {
            return make_shared<RuleDefinitionExpression>(
                name,
                make_shared<OneOrMoreExpression>(
                    make_shared<RuleReferenceExpression>(item)
                )
            );
        };

        auto digit = [](auto name, initializer_list<pair<char32_t, char32_t>> ranges) {
            vector<shared_ptr<Expression>> items;

            for (const auto &range: ranges) {
                items.emplace_back(
                    make_shared<RangeExpression>(range.first, range.second)
                );
            }

            return make_shared<RuleDefinitionExpression>(
                name,
                make_shared<ChoiceExpression>(items)
            );
        };

        auto single_quote = []() {
            return make_shared<RuleDefinitionExpression>(
                U"single-quote",
                make_shared<LiteralExpression>(U"'")
            );
        };

        auto double_quote = []() {
            return make_shared<RuleDefinitionExpression>(
                U"double-quote",
                make_shared<LiteralExpression>(U"\"")
            );
        };

        auto escape = []() {
            return make_shared<RuleDefinitionExpression>(
                U"escape",
                make_shared<LiteralExpression>(U"\\")
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
            string_literal(),
            quoted_characters(U"single-quoted-characters", U"single-quoted-character"),
            quoted_characters(U"double-quoted-characters", U"double-quoted-character"),
            quoted_character(U"single-quoted-character", U"single-quote"),
            quoted_character(U"double-quoted-character", U"double-quote"),
            escaped_character(),
            escape_sequence(),
            numeric_escape_sequence(U"binary-escape-sequence", U"b", U"binary-digits"),
            numeric_escape_sequence(U"quaternary-escape-sequence", U"q", U"quaternary-digits"),
            numeric_escape_sequence(U"octal-escape-sequence", U"o", U"octal-digits"),
            numeric_escape_sequence(U"hexadecimal-escape-sequence", U"h", U"hexadecimal-digits"),
            builtin_escape_sequence(),
            digits(U"binary-digits", U"binary-digit"),
            digits(U"quaternary-digits", U"quaternary-digit"),
            digits(U"octal-digits", U"octal-digit"),
            digits(U"hexadecimal-digits", U"hexadecimal-digit"),
            digit(U"binary-digit", { { U'0', U'1' } }),
            digit(U"quaternary-digit", { { U'0', U'3' } }),
            digit(U"octal-digit", { { U'0', U'7' } }),
            digit(U"hexadecimal-digit", { { U'0', U'9' }, { U'A', U'F' } }),
            double_quote(),
            single_quote(),
            escape(),
            white_space(),
            character(),
        };

        return make_shared<GrammarExpression>(items);
    }
}

#endif
