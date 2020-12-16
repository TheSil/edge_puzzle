import argparse
from core.defs import PuzzleDefinition, PieceRef, N, E, S, W
from core import board


class BacktrackerLocal:

    def __init__(self, board, positions, fix_smaller=False):
        self.counter = 0
        self.board = board
        self.fix_smaller = fix_smaller
        self.unvisited = dict()
        self.last_found = None
        x = 4
        for i, j in positions:
            self.unvisited[i,j] = 0
        self.visited = list()
        self.placed_ids = set()
        self.unplaced_ids = set()
        self.unplaced_corners = set()
        for piece in self.board.puzzle_def.corners:
            for dir in range(4):
                self.unplaced_corners.add(PieceRef(piece, dir, 0, 0))
        self.unplaced_edges = set()
        for piece in self.board.puzzle_def.edges:
            for dir in range(4):
                self.unplaced_edges.add(PieceRef(piece, dir, 0, 0))
        self.unplaced_inner = set()
        for piece in self.board.puzzle_def.inner:
            for dir in range(4):
                self.unplaced_inner.add(PieceRef(piece, dir, 0, 0))
        self.forbidden = {}
        for i in range(self.board.puzzle_def.height):
            for j in range(self.board.puzzle_def.width):
                self.forbidden[i,j] = set()
        self.backtracked_position = None

        # pre-place hints
        self.hints_count = len(self.board.puzzle_def.hints)
        for hint in self.board.puzzle_def.hints:
            id = hint[2]
            self.placed_ids.add(id)
            piece_def = self.board.puzzle_def.all[id]
            i, j = hint[0], hint[1]
            ref = PieceRef(piece_def, 0, i, j)
            self.board.board[i][j] = ref
            if hint[3] != -1:
                ref.dir = hint[3]

            to_remove = set()
            for piece in self.unplaced_corners:
                if piece.piece_def.id == id:
                    to_remove.add(piece)

            for piece in to_remove:
                self.unplaced_corners.remove(piece)

            to_remove = set()
            for piece in self.unplaced_edges:
                if piece.piece_def.id == id:
                    to_remove.add(piece)

            for piece in to_remove:
                self.unplaced_edges.remove(piece)

            to_remove = set()
            for piece in self.unplaced_inner:
                if piece.piece_def.id == id:
                    to_remove.add(piece)

            for piece in to_remove:
                self.unplaced_inner.remove(piece)

            if (i,j) in self.unvisited:
                del self.unvisited[i, j]
            self.visited.append((i, j))
            self.board.marks[i][j] = id


        self.board.fix_orientation()

    def backtrack(self):
        if len(self.visited) == self.hints_count:
            return

        removing = self.visited.pop()
        self.unvisited[removing] = 0
        i = removing[0]
        j = removing[1]
        self.forbidden[removing].add(self.board.board[i][j])
        if self.board.is_corner(i, j):
            self.unplaced_corners.add(self.board.board[i][j])
        elif self.board.is_edge(i, j):
            self.unplaced_edges.add(self.board.board[i][j])
        else:
            self.unplaced_inner.add(self.board.board[i][j])
        self.placed_ids.remove(self.board.board[i][j].piece_def.id)
        self.board.board[i][j] = None
        if self.backtracked_position:
            self.forbidden[self.backtracked_position].clear()
        self.backtracked_position = removing

    def step(self):
        if not self.unvisited:
            # everything already placed, solved...
            #self.last_found = list(sorted([self.board.board[i][j].piece_def.id for i,j in self.visited]))

            # sort the visited lexiographically = top to bottom, left to right
            # e.g. (0,0), (0,1), (1,0), (1,1)
            self.last_found = list([(self.board.board[i][j].piece_def.id, self.board.board[i][j].dir)  for i, j in sorted(self.visited)])

            # attempt to find next one by backtracking this piece
            self.backtrack()
            return True

        # evaluate which of the unvisited nodes has least number of possibilities
        best_score = -1
        best_unplaced_container = None
        best_score_coord = None
        best_feasible_piece = None
        for unvisited in self.unvisited:
            i = unvisited[0]
            j = unvisited[1]
            score = 0
            feasible_piece = None

            if self.board.is_corner(i, j):
                possible = self.unplaced_corners
            elif self.board.is_edge(i, j):
                possible = self.unplaced_edges
            else:
                possible = self.unplaced_inner

            if self.board.is_inner(i, j):
                # for inner pieces, check if they have any neighbours, otherwise we
                # are wasting time computing thouse
                neighbours = 0
                if i > 0:
                    if self.board.board[i - 1][j]:
                        neighbours += 1
                if j > 0:
                    if self.board.board[i][j - 1]:
                        neighbours += 1
                height = self.board.puzzle_def.height
                if i < height - 1:
                    if self.board.board[i + 1][j]:
                        neighbours += 1
                width = self.board.puzzle_def.width
                if j < width - 1:
                    if self.board.board[i][j + 1]:
                        neighbours += 1
                # if neighbours == 0:
                #     continue

            for feasible in possible - self.forbidden[i, j]:
                if feasible.piece_def.id not in self.placed_ids:
                    # TEMPORARY CONDITION ENSURING THAT UPPER LEFT PICE
                    # WILL BE THE ONE WITH LOWEST ID, HENCE WE DO NOT OVERCOUNT 4 TIMES
                    # REMOVE AFTER COUNTING THIS VALUE!!!
                    if self.fix_smaller:
                        hints_count = len(self.board.puzzle_def.hints)
                        if len(self.visited) >= hints_count + 1:
                            if feasible.piece_def.id <= \
                                    self.board.board[self.visited[hints_count][0]][self.visited[hints_count][1]].piece_def.id:
                                continue # ignore pieces lesser or equal to first placed non-hint piece

                    if self.can_be_placed_at(feasible, i, j):
                        score += 1
                        feasible_piece = feasible

            if best_score == -1 or score < best_score:
                best_score = score
                best_score_coord = (i, j)
                best_feasible_piece = feasible_piece
                best_unplaced_container = possible

            if best_score == 0:
                # impossible to place anything here...
                break

        if best_score == 0:
            # impossible to place anything here... backtrack
            end = len(self.visited) > 0
            self.backtrack()
            return end

        # now we have selected piece to place, do it
        i = best_score_coord[0]
        j = best_score_coord[1]
        best_feasible_piece.i = i
        best_feasible_piece.j = j
        self.board.board[i][j] = best_feasible_piece
        best_unplaced_container.remove(best_feasible_piece)
        del self.unvisited[i,j]
        self.visited.append((i,j))
        self.board.marks[i][j] = best_feasible_piece.piece_def.id
        if self.backtracked_position and self.backtracked_position != (i,j):
            self.forbidden[self.backtracked_position].clear()
        self.backtracked_position = None
        self.placed_ids.add(best_feasible_piece.piece_def.id)
        return True

    def can_be_placed_at(self, piece, i, j):
        if i > 0:
            up = self.board.board[i-1][j]
            if up and up.get_color(S) != piece.get_color(N):
                    return False
        if j > 0:
            left = self.board.board[i][j-1]
            if left and left.get_color(E) != piece.get_color(W):
                return False
        height = self.board.puzzle_def.height
        if i < height - 1:
            down = self.board.board[i + 1][j]
            if down and down.get_color(N) != piece.get_color(S):
                return False
        width = self.board.puzzle_def.width
        if j < width - 1:
            right = self.board.board[i][j + 1]
            if right and right.get_color(W) != piece.get_color(E):
                return False

        # corners/edges special checks
        if i==0:
            if j==0:
                if piece.dir != E:
                    return False
            elif j==width-1:
                if  piece.dir != S:
                    return False
            else:
                if piece.dir != E:
                    return False
        elif i==height-1:
            if j==0:
                if piece.dir != N:
                    return False
            elif j==width-1:
                if piece.dir != W:
                    return False
            else:
                if piece.dir != W:
                    return False
        elif j==0:
            if piece.dir != N:
                return False
        elif j==width-1:
            if piece.dir != S:
                return False

        return True


if __name__ == '__main__':

    parser = argparse.ArgumentParser()
    parser.add_argument('-conf', type=str, required=True, help='Definition file')
    parser.add_argument('-hints', type=str, required=False, default=None, help='Hint file')
    parser.add_argument('-o', type=str, required=True, help='Output file')
    args = parser.parse_args()

    puzzle_def = PuzzleDefinition()
    puzzle_def.load(args.conf, args.hints)

    board = board.Board(puzzle_def)
    #ui = ui.BoardUi(board)
    # ui.init()

    selected_from = None
    selected_to = None

    LEFT_BUTTON = 1
    RIGHT_BUTTON = 2

    corner_positions =[(0,0),(0,1),(1,0),(1,1)]
    edge_positions = [(0,2),(0,3),(1,2),(1,3)]
    inner_positions = [(2,2),(2,3),(3,2),(3,3)]

    counter = 0
    all = []

    # corners
    board.clear()
    backtracker = BacktrackerLocal(board, corner_positions)
    while backtracker.step():
        if backtracker.last_found:
            all.append(backtracker.last_found)
            # print(backtracker.last_found)
            backtracker.last_found = None

    # edges
    board.clear()
    backtracker = BacktrackerLocal(board, edge_positions)
    while backtracker.step():
        if backtracker.last_found:
            all.append(backtracker.last_found)
            # f.write(f"{counter},{','.join(str(x) for x in backtracker.last_found)}\n")
            # counter += 1
            # print(backtracker.last_found)
            backtracker.last_found = None

    # inners
    board.clear()
    backtracker = BacktrackerLocal(board, inner_positions, fix_smaller=True)
    while backtracker.step():
        if backtracker.last_found:
            all.append(backtracker.last_found)
            # f.write(f"{counter},{','.join(str(x) for x in backtracker.last_found)}\n")
            # counter += 1
            # print(backtracker.last_found)
            backtracker.last_found = None

    with open(args.o,"w") as f:

        f.write("Index,ID1,ID2,ID3,ID4,Ordered1,Dir1,Ordered2,Dir2,Ordered3,Dir3,Ordered4,Dir4\n")
        for id, item in enumerate(sorted(all)):
            sorted_ids = list(sorted([x[0] for x in item]))
            f.write(f"{id},{','.join(str(x) for x in sorted_ids)},"
                  f"{item[0][0]},{item[0][1]},"
                  f"{item[1][0]},{item[1][1]},"
                  f"{item[2][0]},{item[2][1]},"
                  f"{item[3][0]},{item[3][1]}\n")

        print(counter)

    # while True:
    #     backtracker.step()
    #     counter += 1
    #
    #     if counter >= next_board_update:
    #         print(backtracker.counter)
    #         ui.update()
    #         next_board_update = counter + board_update_rate
    #
    #     for event in pygame.event.get():
    #         if event.type == pygame.locals.QUIT:
    #             pygame.quit()
    #             sys.exit()
    #
    #     pygame.display.update()



