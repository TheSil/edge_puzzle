import sys
import argparse
from core.defs import PuzzleDefinition, TYPE_CORNER, TYPE_EDGE
from core import board
from ui import ui
import pygame.locals

if __name__ == '__main__':

    parser = argparse.ArgumentParser()
    parser.add_argument('-conf', type=str, required=True, help='Definition file')
    parser.add_argument('-hints', type=str, required=False, default=None, help='Hint file')
    parser.add_argument('-load', type=str, required=False, default=None, help='Load from saved file')
    args = parser.parse_args()

    puzzle_def = PuzzleDefinition()
    puzzle_def.load(args.conf, args.hints)

    height = puzzle_def.height
    width = puzzle_def.width

    board = board.Board(puzzle_def)

    if args.load:
        board.load(args.load)
        # fill empty pieces
        for id in range(1, height*width+1):
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
        # board.save(args.load+".2")

    else:
        board.randomize()
        board.heuristic_orientation()

    # mark the pieces
    for i in range(height):
        for j in range(width):
            if board.board[i][j]:
                board.marks[i][j] = board.board[i][j].piece_def.id

    ui = ui.BoardUi(board)
    ui.init()
    pygame.display.set_caption(f'Puzzle (score {board.evaluate()})')

    selected_from = None
    selected_to = None

    LEFT_BUTTON = 1
    RIGHT_BUTTON = 2

    while True:

        for event in pygame.event.get():
            if event.type == pygame.locals.QUIT:
                pygame.quit()
                sys.exit()
            elif event.type == pygame.MOUSEBUTTONUP:
                x, y = pygame.mouse.get_pos()
                i = y // ui.piece_width
                j = x // ui.piece_width

                if event.button == LEFT_BUTTON:
                    # move
                    if not selected_from:
                        selected_from = (i,j)
                    else:
                        selected_to = (i,j)

                        if board.is_corner(*selected_from) and board.is_corner(*selected_to) or \
                                board.is_edge(*selected_from) and board.is_edge(*selected_to) or \
                                board.is_inner(*selected_from) and board.is_inner(*selected_to):
                            board.exchange(*selected_from, *selected_to)
                            from_i = selected_from[0]
                            from_j = selected_from[1]
                            to_i = selected_to[0]
                            to_j = selected_to[1]

                            board.marks[from_i][from_j], board.marks[to_i][to_j] = \
                                board.marks[to_i][to_j], board.marks[from_i][from_j]
                            board.fix_orientation()
                            # experimental feature
                            board.heuristic_orientation()
                            pygame.display.set_caption(f'Puzzle (score {board.evaluate()})')
                            ui.update()

                        selected_from = selected_to = None

                else:
                    # rotate
                    if (i > 0) and (i < height - 1) and (j > 0) and (j < width - 1):
                        board.board[i][j].dir += 1
                        board.board[i][j].dir %= 4
                        pygame.display.set_caption(f'Puzzle (score {board.evaluate()})')
                        ui.update()


            elif event.type == pygame.MOUSEBUTTONDOWN:
                x, y = pygame.mouse.get_pos()
                i = y // ui.piece_width
                j = x // ui.piece_width

            elif event.type == pygame.KEYDOWN:
                if event.key == pygame.K_i:
                    ui.marks_enabled = not ui.marks_enabled
                    ui.update()

        pygame.display.update()



