from math import inf

from hbcode.peg.exceptions import SyntaxError
from hbcode.peg.node import Node
from hbcode.peg.range import Range


class BaseRule:
    def __init__(self, type, to_ast=None):
        self.type = type
        self.to_ast = to_ast

    def parse(self, offset, context):
        raise NotImplementedError


class EndOfInputRule(BaseRule):
    def get_children(self):
        return []

    def parse(self, offset, context):
        if len(context.text) == offset:
            return Node(context, self, Range(offset, offset))
        else:
            raise SyntaxError(context, self, offset)


class CharSetRule(BaseRule):
    def get_children(self):
        return []

    def __init__(self, type, ranges, **kwargs):
        super().__init__(type, **kwargs)
        self.ranges = []
        for range in ranges:
            try:
                min, max = range
            except ValueError:
                min = max = range
            self.ranges.append((min, max))

    def parse(self, offset, context):
        try:
            char = context.text[offset]
            for min, max in self.ranges:
                if min <= char <= max:
                    return Node(context, self, Range(offset, offset + 1))
        except IndexError:
            pass
        raise SyntaxError(context, self, offset)


class LiteralRule(BaseRule):
    def get_children(self):
        return []

    def __init__(self, type, literal, **kwargs):
        super().__init__(type, **kwargs)
        self.literal = literal

    def parse(self, offset, context):
        start = offset
        stop = offset + len(self.literal)
        if context.text[start:stop] == self.literal:
            return Node(context, self, Range(start, stop))
        else:
            raise SyntaxError(context, self, offset)


class RepeatRule(BaseRule):
    def get_children(self):
        return [self.item_type]

    def __init__(self, type, item_type, min=0, max=inf, **kwargs):
        super().__init__(type, **kwargs)
        self.item_type = item_type
        self.min = min
        self.max = max

    def parse(self, offset, context):
        start, stop = offset, offset
        children = []
        i = 0
        while i < self.max:
            try:
                item = context.grammar[self.item_type]
                node = item.parse(stop, context)
                stop = node.range.stop
                children.append(node)
            except SyntaxError:
                if i >= self.min:
                    break
                else:
                    raise
            i += 1
        return Node(context, self, Range(start, stop), children)


class SequenceRule(BaseRule):
    def get_children(self):
        return self.items_types

    def __init__(self, type, items_types, **kwargs):
        super().__init__(type, **kwargs)
        self.items_types = items_types

    def parse(self, offset, context):
        start = offset
        stop = offset
        children = []
        for item_type in self.items_types:
            item = context.grammar[item_type]
            node = item.parse(stop, context)
            stop = node.range.stop
            children.append(node)
        return Node(context, self, Range(start, stop), children)


class ChoiceRule(BaseRule):
    def get_children(self):
        return self.choices_types

    def __init__(self, type, choices_types, **kwargs):
        super().__init__(type, **kwargs)
        self.choices_types = choices_types

    def parse(self, offset, context):
        for choice_type in self.choices_types:
            choice = context.grammar[choice_type]
            try:
                return choice.parse(offset, context)
            except SyntaxError:
                continue
        raise SyntaxError(context, self, offset)


class AndRule(BaseRule):
    def get_children(self):
        return [self.rule_type]

    def __init__(self, type, rule_type, **kwargs):
        super().__init__(type, **kwargs)
        self.rule_type = rule_type

    def parse(self, offset, context):
        rule = context.grammar[self.rule_type]
        rule.parse(offset, context)
        return Node(context, self, Range(offset, offset))


class NotRule(BaseRule):
    def get_children(self):
        return [self.rule_type]

    def __init__(self, type, rule_type, **kwargs):
        super().__init__(type, **kwargs)
        self.rule_type = rule_type

    def parse(self, offset, context):
        rule = context.grammar[self.rule_type]
        try:
            rule.parse(offset, context)
        except SyntaxError:
            return Node(context, self, Range(offset, offset))
        else:
            raise SyntaxError(context, self, offset)
