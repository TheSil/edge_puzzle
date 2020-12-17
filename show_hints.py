import argparse
import sys
import pygame.locals
from core.defs import PuzzleDefinition, PieceRef
from ui import ui
from core import board

if __name__ == '__main__':

    parser = argparse.ArgumentParser()
    parser.add_argument('-conf', type=str, required=True, help='Definition file')
    parser.add_argument('-hints', type=str, required=True, default=None, help='Hint file')
    args = parser.parse_args()

    puzzle_def = PuzzleDefinition()
    puzzle_def.load(args.conf, args.hints)

    board = board.Board(puzzle_def)
    ui = ui.BoardUi(board)
    ui.init()

    # place hints
    for hint in board.puzzle_def.hints:
        id = hint[2]
        piece_def = board.puzzle_def.all[id]
        i, j = hint[0], hint[1]
        ref = PieceRef(piece_def, 0, i, j)
        board.board[i][j] = ref
        if hint[3] != -1:
            ref.dir = hint[3]
        board.marks[i][j] = id

    board.fix_orientation()
    ui.update()
    print(f"score:{board.evaluate()}")

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



