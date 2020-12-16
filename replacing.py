import argparse
import sys
import time
from collections import deque
import copy
import random
import uuid
import pygame
from core.defs import PuzzleDefinition, PieceRef, N, E, S, W, TYPE_CORNER, TYPE_EDGE
from ui import ui
from core import board


class Replacer:

    def __init__(self, board):
        self.board = board
        self.unplaced = deque([x for x in self.board.puzzle_def.all.values()])
        random.shuffle(self.unplaced)
        self.orientations = [self.board.puzzle_def.width*[None] for i in range(self.board.puzzle_def.height)]
        self.orientations[0][0]=[0]
        self.orientations[0][self.board.puzzle_def.width - 1]=[1]
        self.orientations[self.board.puzzle_def.height - 1][0]=[3]
        self.orientations[self.board.puzzle_def.height - 1][self.board.puzzle_def.width - 1]=[2]
        for i in range(1, self.board.puzzle_def.height - 1):
            self.orientations[i][0]=[3]
            self.orientations[i][self.board.puzzle_def.width - 1]=[1]
        for j in range(1, self.board.puzzle_def.width - 1):
            self.orientations[0][j]=[0]
            self.orientations[self.board.puzzle_def.height - 1][j]=[2]
        for i in range(1, self.board.puzzle_def.height - 1):
            for j in range(1, self.board.puzzle_def.width - 1):
                self.orientations[i][j] = list(range(4))

        # if there is anything already placed on the board, account for that
        for i in range(0, self.board.puzzle_def.height):
            for j in range(0, self.board.puzzle_def.width):
                if self.board.board[i][j]:
                    self.unplaced.remove(self.board.board[i][j].piece_def)


    def check_piece_score(self, ref, i, j):
        new = 0
        penalty = 0
        rem = []
        if j < self.board.puzzle_def.width - 1 and self.board.board[i][j + 1]:
            if ref.get_color(E) == self.board.board[i][j + 1].get_color(W):
                new += 1
            else:
                penalty += self.board.neighbours_count(i, j + 1)
                rem.append((i, j + 1))

        if j > 0 and self.board.board[i][j - 1]:
            if ref.get_color(W) == self.board.board[i][j - 1].get_color(E):
                new += 1
            else:
                penalty += self.board.neighbours_count(i, j - 1)
                rem.append((i, j - 1))

        if i < self.board.puzzle_def.height - 1 and self.board.board[i + 1][j]:
            if ref.get_color(S) == self.board.board[i + 1][j].get_color(N):
                new += 1
            else:
                penalty += self.board.neighbours_count(i + 1, j)
                rem.append((i + 1, j))
        if i > 0 and self.board.board[i - 1][j]:
            if ref.get_color(N) == self.board.board[i - 1][j].get_color(S):
                new += 1
            else:
                penalty += self.board.neighbours_count(i - 1, j)
                rem.append((i - 1, j))

        return new - penalty, rem

    def remove_random(self, n):
        possibilities = []
        for i in range(1, self.board.puzzle_def.height - 1):
            for j in range(1, self.board.puzzle_def.width - 1):
                if self.board.board[i][j]:
                    possibilities.append((i,j))
        random.shuffle(possibilities)
        for k in range(n):
            i = possibilities[k][0]
            j = possibilities[k][1]
            self.unplaced.append(self.board.board[i][j].piece_def)
            self.board.board[i][j] = None

    def step(self):
        if not self.unplaced:
            # nothing to place, solved probably...
            return

        random.shuffle(self.unplaced)

        max_score = None
        max_i = None
        max_j = None
        max_tile = None
        max_ref = None
        max_dir = None
        max_rem = []

        tile = self.unplaced.popleft()
        if tile.type == TYPE_CORNER:
            coll = self.board.enumerate_corners()
        elif tile.type == TYPE_EDGE:
            coll = self.board.enumerate_edges()
        else:
            coll = self.board.enumerate_inner()

        for i, j in coll:
            curr = 0
            if self.board.board[i][j]:
                curr = self.board.evaluate_piece(i, j)
            ref = PieceRef(tile, 0, i, j)
            for dir in self.orientations[i][j]:
                ref.dir = dir
                score, rem = self.check_piece_score(ref, i, j)
                score -= curr
                if max_score is None or score > max_score:
                    max_score = score
                    max_i = i
                    max_j = j
                    max_ref = ref
                    max_dir = dir
                    max_tile = tile
                    max_rem = rem

        if self.board.board[max_i][max_j]:
            self.unplaced.append(self.board.board[max_i][max_j].piece_def)
            self.board.board[max_i][max_j] = None
        self.board.board[max_i][max_j] = max_ref
        self.board.board[max_i][max_j].dir = max_dir
        for i, j in max_rem:
            self.unplaced.append(self.board.board[i][j].piece_def)
            self.board.board[i][j] = None

        print(f"Placing {max_ref.piece_def.id} at {max_i},{max_j}")


        pass


if __name__ == '__main__':

    parser = argparse.ArgumentParser()
    parser.add_argument('-conf', type=str, required=True, help='Definition file')
    parser.add_argument('-hints', type=str, required=False, default=None, help='Hint file')
    parser.add_argument('--disable_reducing', action='store_true', required=False, default = False, help='Reducing constraints')
    parser.add_argument('-stats', type=str, required=False, default=None, help='Collocation stats file')
    parser.add_argument('-load', type=str, required=False, default=None, help='Load from saved file')
    args = parser.parse_args()

    puzzle_def = PuzzleDefinition()
    puzzle_def.load(args.conf, args.hints)

    board = board.Board(puzzle_def)
    ui = ui.BoardUi(board)
    ui.init()

    if args.load:
        board.load(args.load)
        # fill empty pieces
        for id in range(1, puzzle_def.height*puzzle_def.width+1):
            if id not in board.board_by_id:
                if board.puzzle_def.all[id].get_type() == TYPE_CORNER:
                    for i,j in board.enumerate_corners():
                        if not board.board[i][j]:
                            board.put_piece(i, j, board.puzzle_def.all[id], 0)
                            break
                    pass
                elif board.puzzle_def.all[id].get_type() == TYPE_EDGE:
                    for i,j in board.enumerate_edges():
                        if not board.board[i][j]:
                            board.put_piece(i, j, board.puzzle_def.all[id], 0)
                            break
                else: # INNER
                    for i,j in board.enumerate_inner():
                        if not board.board[i][j]:
                            board.put_piece(i, j, board.puzzle_def.all[id], 0)
                            break
        board.fix_orientation()

    selected_from = None
    selected_to = None

    replacer = Replacer(board)
    start = time.time()
    running_min = 0
    running_sec = 0
    next_explored_stats_update = start
    explored_stats_rate = 5
    explored_str = ""

    best_replacer = replacer
    best_score_count_limit = 500
    best_score_counter = best_score_count_limit


    best = 0
    while True:
        replacer.step()
        ui.update()
        val = board.evaluate()
        if best < val:
            best_replacer = copy.deepcopy(replacer)
            #ui.board = best_board
            #ui.update()

            best = val
            # we fill in remaining pieces to have a slightly better score
            best_score_counter = best_score_count_limit
        else:
            best_score_counter -= 1

        if best_score_counter <= 0:
            best_score_counter = best_score_count_limit
            replacer = best_replacer
            ui.board = best_replacer.board
            board = best_replacer.board
            replacer.remove_random(20)

        for event in pygame.event.get():
            if event.type == pygame.locals.QUIT:
                pygame.quit()
                sys.exit()
            elif event.type == pygame.KEYDOWN:
                if event.key == pygame.K_i:
                    ui.marks_enabled = not ui.marks_enabled
                    ui.update()
                elif event.key == pygame.K_s:
                    best_replacer.board.save("manual_save " + str(uuid.uuid4()) + "_" + str(best) + ".csv")


        caption = f'S {best}/{board.max_score()}'

        pygame.display.set_caption(caption)
        pygame.display.update()



7