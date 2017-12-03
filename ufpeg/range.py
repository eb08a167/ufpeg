class Range:
    def __init__(self, start, stop):
        self.start, self.stop = start, stop

    def to_slice(self):
        return slice(self.start, self.stop)
