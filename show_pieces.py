import argparse
import sys
import pygame.locals
from core.defs import PuzzleDefinition, PieceRef
from ui import ui
from core import board

if __name__ == '__main__':

    parser = argparse.ArgumentParser()
    parser.add_argument('-conf', type=str, required=True, help='Definition file')
    parser.add_argument('-hints', type=str, required=False, default=None, help='Hint file')
    args = parser.parse_args()

    puzzle_def = PuzzleDefinition()
    puzzle_def.load(args.conf, args.hints)

    height = puzzle_def.height
    width = puzzle_def.width

    board = board.Board(puzzle_def)
    ui = ui.BoardUi(board)
    ui.init()

    selected_from = None
    selected_to = None

    LEFT_BUTTON = 1
    RIGHT_BUTTON = 2

    idx = 1
    for i in range(board.puzzle_def.height):
        for j in range(board.puzzle_def.width):
            board.board[i][j] = PieceRef(board.puzzle_def.all[idx], 0, i, j)
            board.marks[i][j] = idx
            idx += 1
    ui.update()

    best = 0
    while True:

        for event in pygame.event.get():
            if event.type == pygame.locals.QUIT:
                pygame.quit()
                sys.exit()
            elif event.type == pygame.KEYDOWN:
                if event.key == pygame.K_i:
                    ui.marks_enabled = not ui.marks_enabled
                    ui.update()

        pygame.display.update()



