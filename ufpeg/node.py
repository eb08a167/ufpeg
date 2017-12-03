class Node:
    def __init__(self, context, rule, range, children=None):
        self.context = context
        self.rule = rule
        self.range = range
        self.children = children or []

    def __iter__(self):
        return iter(self.children)

    def get_text(self):
        return self.context.text[self.range.to_slice()]

    def to_ast(self):
        return self.rule.to_ast(self)
