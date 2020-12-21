import argparse
import sys
import pygame.locals
from core.defs import PuzzleDefinition
from ui import ui
import time
from core import board
from backtracker import Backtracker


if __name__ == '__main__':

    parser = argparse.ArgumentParser()
    parser.add_argument('-conf', type=str, required=True, help='Definition file')
    parser.add_argument('-hints', type=str, required=False, default=None, help='Hint file')
    args = parser.parse_args()

    puzzle_def = PuzzleDefinition()
    puzzle_def.load(args.conf, args.hints)

    board = board.Board(puzzle_def)
    ui = ui.BoardUi(board)
    ui.init()

    selected_from = None
    selected_to = None

    LEFT_BUTTON = 1
    RIGHT_BUTTON = 2

    pieces_map = []
    for i in range((board.puzzle_def.height)):
        for j in range((board.puzzle_def.width)):
            if i+j == 1: #or i>= board.puzzle_def.height-3 or j>=board.puzzle_def.width-3:
                pieces_map.append((i,j))

    backtracker = Backtracker(board,
                              pieces_map=pieces_map,
                              enable_finalizing=False,
                              find_all=True,
                              constraint_reducing=True)
    board_update_rate = 100
    next_board_update = 0
    counter = 0

    start = time.time()
    running_min = 0
    running_sec = 0
    next_explored_stats_update = start
    explored_stats_rate = 5
    explored_str = ""

    while True:
        backtracker.step()
        # if backtracker.counter == 1:
        #     board.save("test.csv")

        counter += 1

        if counter >= next_board_update:
            print(backtracker.counter)
            ui.update()
            next_board_update = counter + board_update_rate

        for event in pygame.event.get():
            if event.type == pygame.locals.QUIT:
                pygame.quit()
                sys.exit()

        if time.time() >= next_explored_stats_update:
            try:
                explored_str = f'ExplRat:{backtracker.explored_ratio():.2E} ExplAbs:{backtracker.explored_count():.2E}'
            except Exception as e:
                explored_str = f'ExplRat:ERR% ExplAbs:ERR'
            #print(explored_str)
            next_explored_stats_update = time.time() + explored_stats_rate

        caption = f'S {explored_str}'

        pygame.display.set_caption(caption)
        pygame.display.update()



