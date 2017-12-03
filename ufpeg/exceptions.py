class SyntaxError(Exception):
    def __init__(self, context, rule, offset):
        self.context = context
        self.rule = rule
        self.offset = offset
