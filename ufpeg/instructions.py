class BaseInstruction:
    pass


class InvokeInstruction(BaseInstruction):
    def __init__(self, pointer):
        self.pointer = pointer


class RevokeInstruction(BaseInstruction):
    pass


class PrepareInstruction(BaseInstruction):
    pass


class ConsumeInstruction(BaseInstruction):
    def __init__(self, name):
        self.name = name


class DiscardInstruction(BaseInstruction):
    pass


class BeginInstruction(BaseInstruction):
    pass


class CommitInstruction(BaseInstruction):
    pass


class AbortInstruction(BaseInstruction):
    pass


class MatchLiteralInstruction(BaseInstruction):
    def __init__(self, literal):
        self.literal = literal


class BranchInstruction(BaseInstruction):
    def __init__(self, success, failure):
        self.success = success
        self.failure = failure


class JumpInstruction(BaseInstruction):
    def __init__(self, pointer):
        self.pointer = pointer


class PassInstruction(BaseInstruction):
    pass


class FlipInstruction(BaseInstruction):
    pass


class ExpectInstruction(BaseInstruction):
    def __init__(self, name):
        self.name = name
