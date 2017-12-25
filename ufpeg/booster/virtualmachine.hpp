#ifndef UFPEG_VIRTUAL_MACHINE_HPP
#define UFPEG_VIRTUAL_MACHINE_HPP

#include <stack>
#include <vector>

#include "node.hpp"
#include "instructions.hpp"

namespace ufpeg {
    template<typename T>
    using fast_stack = std::stack<T, std::vector<T>>;

    class VirtualMachine {
    public:
        void execute(
            const std::vector<RawInstruction> instructions,
            const std::u32string &text
        ) const {
            fast_stack<std::size_t> pointers, cursors;
            fast_stack<Node> nodes;
            std::vector<const char32_t*> expectations;
            std::size_t offset = 0;
            bool has_matched = true;

            pointers.push(0);
            cursors.push(0);
            nodes.push({ nullptr });

            while (!pointers.empty()) {
                auto pointer = pointers.top();
                auto &instruction = instructions[pointer];

                switch (instruction.op_code) {
                case OpCode::INVOKE: {
                        pointers.top()++;
                        pointers.push(instruction.target);
                    }
                    break;
                case OpCode::REVOKE: {
                        pointers.pop();
                    }
                    break;
                case OpCode::PREPARE: {
                        nodes.push({ nullptr, cursors.top() });
                        pointers.top()++;
                    }
                    break;
                case OpCode::CONSUME: {
                        auto child = std::move(nodes.top());
                        nodes.pop();
                        child.name = instruction.name.c_str();
                        child.stop = cursors.top();
                        auto &parent = nodes.top();
                        parent.children.emplace_back(child);
                        pointers.top()++;
                    }
                    break;
                case OpCode::DISCARD: {
                        nodes.pop();
                        pointers.top()++;
                    }
                    break;
                case OpCode::BEGIN: {
                        auto cursor = cursors.top();
                        cursors.push(cursor);
                        pointers.top()++;
                    }
                    break;
                case OpCode::COMMIT: {
                        auto cursor = cursors.top();
                        cursors.pop();
                        cursors.top() = cursor;
                        pointers.top()++;
                    }
                    break;
                case OpCode::ABORT: {
                        cursors.pop();
                        pointers.top()++;
                    }
                    break;
                case OpCode::MATCH_LITERAL: {
                        auto &cursor = cursors.top();
                        const auto length = instruction.literal.length();
                        has_matched = !text.compare(cursor, length, instruction.literal);
                        cursor += has_matched ? length : 0;
                        pointers.top()++;
                    }
                    break;
                case OpCode::BRANCH: {
                        pointers.top() = has_matched ? instruction.success : instruction.failure;
                    }
                    break;
                case OpCode::JUMP: {
                        pointers.top() = instruction.target;
                    }
                    break;
                case OpCode::PASS: {
                        pointers.top()++;
                    }
                    break;
                case OpCode::FLIP: {
                        has_matched = !has_matched;
                        pointers.top()++;
                    }
                    break;
                case OpCode::EXPECT: {
                        auto cursor = cursors.top();
                        if (cursor > offset) {
                            expectations.clear();
                            offset = cursor;
                        }
                        expectations.push_back(instruction.name.c_str());
                        pointers.top()++;
                    }
                    break;
                }
            }
        }
    };
}

#endif
