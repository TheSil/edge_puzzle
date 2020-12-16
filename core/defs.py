E = 0
S = 1
W = 2
N = 3

TYPE_CORNER = 0
TYPE_EDGE = 1
TYPE_INNER = 2


class PieceDef:
    def __init__(self, id, E=0, S=0, W=0, N=0):
        self.id = id
        self.colors = [E, S, W, N]
        gray_count = self.colors.count(0)
        if gray_count == 2:
            self.type = TYPE_CORNER
        elif gray_count == 1:
            self.type = TYPE_EDGE
        else:
            self.type = TYPE_INNER

    def get_color(self, idx):
        return self.colors[idx]

    def set_color(self, idx, color):
        self.colors[idx] = color

    def get_type(self):
        return self.type

    def __repr__(self):
        return str(self.colors)


class PuzzleDefinition:
    def __init__(self):
        self.height = 0
        self.width = 0
        self.edge_colors = 0
        self.inner_color = 0
        self.corners = []
        self.edges = []
        self.inner = []
        self.hints = []
        self.all = {}

    def load(self, filename, hints=None):
        self.pieces = []
        self.hints = []
        with open(filename, "r") as f:
            header = f.readline().strip().split(",")
            height, width, edge_colors, inner_colors = header[0], header[1], header[2], header[3]
            self.height = int(height)
            self.width = int(width)
            self.edge_colors = int(edge_colors)
            self.inner_color = int(inner_colors)

            for i in range(self.height * self.width):
                line = f.readline().strip()
                items = line.split(",")
                items = [int(x) if x else 0 for x in items]
                items.extend([0] * (5 - len(items)))  # 1 id and 4 colors
                new_piece = PieceDef(*items)
                self.all[new_piece.id] = new_piece

                if items.count(0) == 2:
                    self.corners.append(new_piece)
                elif items.count(0) == 1:
                    self.edges.append(new_piece)
                else:
                    self.inner.append(new_piece)

        if hints:
            with open(hints, "r") as f:
                for line in f.readlines():
                    i, j, piece_id, piece_orientation = line.strip().split(",")
                    i = int(i)
                    j = int(j)
                    piece_id = int(piece_id)
                    piece_orientation = int(piece_orientation)
                    self.hints.append((i, j, piece_id, piece_orientation))


class PieceRef:
    def __init__(self, piece_def, dir, i, j):
        self.piece_def = piece_def
        self.dir = dir
        self.i = i
        self.j = j

    def get_color(self, pos):
        # orientation E => index 0 at E, orientation S => index 0 at S, etc
        idx = (pos - self.dir) % 4
        return self.piece_def.get_color(idx)

    def set_color(self, pos, color):
        # orientation E => index 0 at E, orientation S => index 0 at S, etc
        idx = (pos - self.dir) % 4
        self.piece_def.set_color(idx, color)
