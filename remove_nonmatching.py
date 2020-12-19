import argparse
import sys
import time
import random
import uuid
import pygame.locals
from ui import ui
from core import board
from core.defs import PuzzleDefinition, TYPE_CORNER, TYPE_EDGE, E, W, N, S

if __name__ == '__main__':

    parser = argparse.ArgumentParser()
    parser.add_argument('-conf', type=str, required=True, help='Definition file')
    parser.add_argument('-hints', type=str, required=False, default=None, help='Hint file')
    parser.add_argument('--disable_reducing', action='store_true', required=False, default = False, help='Reducing constraints')
    parser.add_argument('-stats', type=str, required=False, default=None, help='Collocation stats file')
    parser.add_argument('-load', type=str, required=True, default=None, help='Load from saved file')
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

    start = time.time()
    running_min = 0
    running_sec = 0
    next_explored_stats_update = start
    explored_stats_rate = 5
    explored_str = ""

    def check_piece_score(board, i, j):
        rem = []
        if j < board.puzzle_def.width - 1 and board.board[i][j + 1]:
            if board.board[i][j].get_color(E) != board.board[i][j + 1].get_color(W):
                rem.append((i, j + 1))

        if j > 0 and board.board[i][j - 1]:
            if board.board[i][j].get_color(W) != board.board[i][j - 1].get_color(E):
                rem.append((i, j - 1))

        if i < board.puzzle_def.height - 1 and board.board[i + 1][j]:
            if board.board[i][j].get_color(S) != board.board[i + 1][j].get_color(N):
                rem.append((i + 1, j))
        if i > 0 and board.board[i - 1][j]:
            if board.board[i][j].get_color(N) != board.board[i - 1][j].get_color(S):
                rem.append((i - 1, j))

        return rem

    indeces = list()
    for i in range(board.puzzle_def.height):
        for j in range(board.puzzle_def.width):
            indeces.append((i,j))
    random.shuffle(indeces)
    for i, j in indeces:
        to_remove = check_piece_score(board, i, j)
        for piece in to_remove:
            board.board[i][j] = None


    best =  board.evaluate()
    while True:
        ui.update()

        for event in pygame.event.get():
            if event.type == pygame.locals.QUIT:
                pygame.quit()
                sys.exit()
            elif event.type == pygame.KEYDOWN:
                if event.key == pygame.K_i:
                    ui.marks_enabled = not ui.marks_enabled
                    ui.update()
                elif event.key == pygame.K_s:
                    board.save("manual_save " + str(uuid.uuid4()) + "_" + str(best) + ".csv")


        caption = f'S {best}/{board.max_score()}'

        pygame.display.set_caption(caption)
        pygame.display.update()



7