import random
from core.defs import PieceRef, N, E, S, W

class Board:
    def __init__(self, puzzle_def):
        self.puzzle_def = puzzle_def
        self.board = [self.puzzle_def.width*[None] for _ in range(self.puzzle_def.height)]
        self.board_by_id = {}
        self.marks = [self.puzzle_def.width * [None] for _ in range(self.puzzle_def.height)]
        self.hints = [self.puzzle_def.width*[None] for _ in range(self.puzzle_def.height)]
        # place the hints
        for i, j, hint_id, hint_orientation in puzzle_def.hints:
            self.hints[i][j] = puzzle_def.all[hint_id]
            self.hints[i][j].dir = hint_orientation

    def clear(self):
        self.board = [self.puzzle_def.width * [None] for i in range(self.puzzle_def.height)]
        self.board_by_id = {}

    def load(self, filename):
        with open(filename, "r") as f:
            for line in f.readlines():
                i, j, piece_id, piece_orientation = line.strip().split(",")
                i = int(i)
                j = int(j)
                piece_id = int(piece_id)
                piece_orientation = int(piece_orientation)
                self.put_piece(i,j, self.puzzle_def.all[piece_id], piece_orientation)
        self.fix_orientation()

    def save(self, filename):
        with open(filename, "w") as f:
            for i in range(self.puzzle_def.height):
                for j in range(self.puzzle_def.width):
                    if self.board[i][j]:
                        f.write(f"{i},{j},{self.board[i][j].piece_def.id},{self.board[i][j].dir}\n")

    def max_score(self):
        return self.puzzle_def.width*(self.puzzle_def.height - 1) \
               + self.puzzle_def.height*(self.puzzle_def.width-1)

    def neighbours_count(self, i, j):
        count = 0
        for neighbour in self.enumerate_neigbours(i, j, diagonal=False):
            count += 1

        return count

    def enumerate_neigbours(self, i, j, diagonal=False):
        if j < self.puzzle_def.width - 1 and self.board[i][j + 1]:
            yield self.board[i][j + 1]
        if j > 0 and self.board[i][j - 1]:
            yield self.board[i][j - 1]
        if i < self.puzzle_def.height - 1 and self.board[i + 1][j]:
            yield self.board[i + 1][j]
        if i > 0 and self.board[i - 1][j]:
            yield self.board[i - 1][j]

        if diagonal:
            if j < self.puzzle_def.width - 1:
                if i < self.puzzle_def.height - 1 and self.board[i + 1][j + 1]:
                   yield self.board[i + 1][j + 1]
                if i > 0 and self.board[i - 1][j + 1]:
                   yield self.board[i - 1][j + 1]
            if i < self.puzzle_def.height - 1:
                if j < self.puzzle_def.width - 1 and self.board[i + 1][j + 1]:
                    yield self.board[i + 1][j + 1]
                if j > 0 and self.board[i + 1][j - 1]:
                    yield self.board[i + 1][j - 1]


    def enumerate_corners(self):
        height = self.puzzle_def.height
        width = self.puzzle_def.width
        yield 0,0
        yield 0,width - 1
        yield height - 1,0
        yield height - 1,width - 1

    def enumerate_top_edges(self):
        width = self.puzzle_def.width
        for k in range(1, width-1):
            yield 0, k

    def enumerate_right_edges(self):
        height = self.puzzle_def.height
        width = self.puzzle_def.width
        for k in range(1, height-1):
            yield k, width-1

    def enumerate_bottom_edges(self):
        height = self.puzzle_def.height
        width = self.puzzle_def.width
        for k in range(1, width-1):
            yield height-1, k

    def enumerate_left_edges(self):
        height = self.puzzle_def.height
        for k in range(1, height-1):
            yield k, 0

    def enumerate_edges(self):
        for i, j in self.enumerate_bottom_edges():
            yield i, j
        for i, j in self.enumerate_left_edges():
            yield i, j
        for i, j in self.enumerate_right_edges():
            yield i, j
        for i, j in self.enumerate_top_edges():
            yield i, j


    def enumerate_inner(self):
        height = self.puzzle_def.height
        width = self.puzzle_def.width

        for i in range(1, height-1):
            for j in range(1, width-1):
                yield i, j

    def put_piece(self, i, j, piece_def, dir):
        # TODO remove currently placed piece in given position
        ref = PieceRef(piece_def, dir, i, j)
        self.board[i][j] = ref
        if piece_def.id in self.board_by_id:
            raise Exception(f"ID {piece_def.id} to be placed already on the board!")
        self.board_by_id[piece_def.id] = ref


    def randomize(self):
        # fill the pieces in random spots
        corner_idxs = list(range(len(self.puzzle_def.corners)))
        edges_idxs =  list(range(len(self.puzzle_def.edges)))
        inners_idxs = list(range(len(self.puzzle_def.inner)))

        random.shuffle(corner_idxs)
        random.shuffle(edges_idxs)
        random.shuffle(inners_idxs)

        height = self.puzzle_def.height
        width = self.puzzle_def.width
        self.put_piece(0, 0, self.puzzle_def.corners[corner_idxs[0]], E)
        self.put_piece(0, width-1, self.puzzle_def.corners[corner_idxs[1]], S)
        self.put_piece(height-1, 0, self.puzzle_def.corners[corner_idxs[2]], N)
        self.put_piece(height-1, width-1, self.puzzle_def.corners[corner_idxs[3]], W)

        idx = 0
        for x, y in self.enumerate_top_edges():
            self.put_piece(x, y, self.puzzle_def.edges[edges_idxs[idx]], E)
            idx += 1
        for x, y in self.enumerate_right_edges():
            self.put_piece(x, y, self.puzzle_def.edges[edges_idxs[idx]], S)
            idx += 1
        for x, y in self.enumerate_bottom_edges():
            self.put_piece(x, y, self.puzzle_def.edges[edges_idxs[idx]], W)
            idx += 1
        for x, y in self.enumerate_left_edges():
            self.put_piece(x, y, self.puzzle_def.edges[edges_idxs[idx]], N)
            idx += 1
        idx = 0
        for x, y in self.enumerate_inner():
            self.put_piece(x, y, self.puzzle_def.inner[inners_idxs[idx]], E)
            idx += 1

        # place the hints to their corresponding locations - inefficient atm
        for i in range(height):
            for j in range(width):
                if self.hints[i][j]:
                    hint_id = self.hints[i][j].id
                    curr_i = self.board_by_id[hint_id].i
                    curr_j = self.board_by_id[hint_id].j
                    self.exchange(curr_i, curr_j, i, j)
                    if self.hints[i][j].dir != -1:
                        self.board[i][j].dir = self.hints[i][j].dir

        self.fix_orientation()


    def fix_orientation(self):
        height = self.puzzle_def.height
        width = self.puzzle_def.width
        if self.board[0][0]:
            self.board[0][0].dir = E
        if self.board[0][width-1]:
            self.board[0][width-1].dir = S
        if self.board[height-1][0]:
            self.board[height-1][0].dir = N
        if self.board[height-1][width-1]:
            self.board[height-1][width-1].dir = W

        for x, y in self.enumerate_top_edges():
            if self.board[x][y]:
                self.board[x][y].dir = E
        for x, y in self.enumerate_right_edges():
            if self.board[x][y]:
                self.board[x][y].dir = S
        for x, y in self.enumerate_bottom_edges():
            if self.board[x][y]:
                self.board[x][y].dir = W
        for x, y in self.enumerate_left_edges():
            if self.board[x][y]:
                self.board[x][y].dir = N

    def heuristic_orientation(self):
        # try to fix the inner pieces orientation by trying various rotations
        did_change = True
        k = 0
        while did_change:
            k=+1
            did_change = False
            for i, j in self.enumerate_inner():
                if self.board[i][j]:
                    if not self.hints[i][j]:
                        best_dir = 0
                        best_score = 0
                        orig_dir = self.board[i][j].dir
                        for dir in range(4):
                            self.board[i][j].dir = dir
                            score = self.evaluate_piece(i,j)
                            if score > best_score:
                                best_score = score
                                best_dir = dir
                        if best_dir != orig_dir:
                            did_change = True
                        self.board[i][j].dir = best_dir
        if k > 1:
            print(f"heuristic required {k} iterations")

    def exchange(self, i1, j1, i2, j2):
        # TODO - check we are not mixing corners, edges and inners
        self.board[i1][j1], self.board[i2][j2] = self.board[i2][j2], self.board[i1][j1]
        if self.board[i1][j1]:
            self.board[i1][j1].i = i1
            self.board[i1][j1].j = j1
        if self.board[i2][j2]:
            self.board[i2][j2].i = i2
            self.board[i2][j2].j = j2

    def evaluate_piece(self, i, j):
        if self.board[i][j]:
            return self.evaluate_at(self.board[i][j], i, j)
        return 0
    
    def evaluate_at(self, piece, i, j):
        matching = 0
        if piece:
            if j < self.puzzle_def.width - 1 and self.board[i][j + 1]:
                if piece.get_color(E) == self.board[i][j + 1].get_color(W):
                    matching += 1
            if j > 0 and self.board[i][j - 1]:
                if piece.get_color(W) == self.board[i][j - 1].get_color(E):
                    matching += 1

            if i < self.puzzle_def.height - 1 and self.board[i + 1][j]:
                if piece.get_color(S) == self.board[i + 1][j].get_color(N):
                    matching += 1
            if i > 0 and self.board[i - 1][j]:
                if piece.get_color(N) == self.board[i - 1][j].get_color(S):
                    matching += 1
        return matching


    def evaluate(self):
        height = self.puzzle_def.height
        width = self.puzzle_def.width
        matching = 0

        # number of matching edges
        for a in range(0, height):
            for b in range(0, width - 1):
                if self.board[a][b] and self.board[a][b + 1]:
                    if self.board[a][b].get_color(E) == self.board[a][b + 1].get_color(W):
                        matching += 1

        for a in range(0, width):
            for b in range(0, height - 1):
                if self.board[b][a] and self.board[b+1][a]:
                    if self.board[b][a].get_color(S) == self.board[b+1][a].get_color(N):
                        matching += 1

        return matching

    def is_corner(self, i, j):
        return (i==0 and j==0) or \
               (i==0 and j==self.puzzle_def.width-1)or \
               (i==self.puzzle_def.height-1 and j==0)or \
               (i==self.puzzle_def.height-1 and j==self.puzzle_def.width-1)

    def is_inner(self, i, j):
        return (0 < i < self.puzzle_def.height-1) and (0 < j < self.puzzle_def.width-1)

    def is_edge(self, i, j):
        return not self.is_inner(i,j) and not self.is_corner(i,j)

