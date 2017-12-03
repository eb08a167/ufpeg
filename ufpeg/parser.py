from hbcode.peg.parsercontext import ParserContext


class Parser:
    def __init__(self, rules):
        self.root = rules[-1]
        self.grammar = {rule.type: rule for rule in rules}

    def parse(self, text):
        context = ParserContext(text, self.grammar)
        return self.root.parse(0, context)
