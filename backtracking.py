import argparse
import copy
import sys
import time
import uuid

import pygame.locals

from ui import ui
from backtracker import Backtracker
from core import board
from core.defs import PuzzleDefinition

if __name__ == '__main__':

    parser = argparse.ArgumentParser()
    parser.add_argument('-conf', type=str, required=True, help='Definition file')
    parser.add_argument('-hints', type=str, required=False, default=None, help='Hint file')
    parser.add_argument('--disable_reducing', action='store_true', required=False, default=False,
                        help='Reducing constraints')
    parser.add_argument('-stats', type=str, required=False, default=None, help='Collocation stats file')
    parser.add_argument('-rotations', type=str, required=False, default=None, help='Rotations file')
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

    backtracker = Backtracker(board,
                              enable_finalizing=False,
                              constraint_reducing=(not args.disable_reducing),
                              connecting=True,
                              grid_file=args.stats,
                              find_all=True,
                              rotations_file=args.rotations)
    start = time.time()
    running_min = 0
    running_sec = 0
    next_explored_stats_update = start
    explored_stats_rate = 5
    explored_str = ""

    best_board = board

    best = 0
    it = 0
    steps_report = time.time() + 1
    explored_last = 0
    while True:
        it += 1
        if time.time() >= steps_report:
            explored = backtracker.explored_count()

            print(f"iters/s:{it}, explored/s:{explored - explored_last:.2E}")
            explored_last = explored
            it = 0
            steps_report = time.time() + 1


        backtracker.step()
        val = board.evaluate()
        # if backtracker.state == backtracker.SOLVED:
        #     print(f"Solved in {it} iterations")

        if best < val:
            best_board = copy.deepcopy(board)
            ui.board = best_board
            ui.update()

            best = val
            # we fill in remaining pieces to have a slightly better score

        for event in pygame.event.get():
            if event.type == pygame.locals.QUIT:
                pygame.quit()
                sys.exit()
            elif event.type == pygame.KEYDOWN:
                if event.key == pygame.K_i:
                    ui.marks_enabled = not ui.marks_enabled
                    ui.update()
                elif event.key == pygame.K_s:
                    best_board.save("manual_save " + str(uuid.uuid4()) + "_" + str(best) + ".csv")

        if backtracker.state != backtracker.SOLVED:
            running = int(time.time() - start)
            running_min = running // 60
            running_sec = running % 60
        # print(f"ExplRat:{backtracker.explored_ratio()*100:.2E}% ExplAbs:{backtracker.explored_count():.2E}")

        if time.time() >= next_explored_stats_update:
            try:
                explored_str = f'ExplRat:{backtracker.explored_ratio():.2E} ExplAbs:{backtracker.explored_count():.2E}'
            except Exception as e:
                explored_str = f'ExplRat:ERR% ExplAbs:ERR'
            # print(explored_str)
            next_explored_stats_update = time.time() + explored_stats_rate

        caption = f'S {best}/{board.max_score()} {running_min}:{running_sec:02} {explored_str}'

        pygame.display.set_caption(caption)
        pygame.display.update()
