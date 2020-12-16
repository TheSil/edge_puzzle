import argparse
import sys
import time
import copy
import pygame.locals
from core.defs import PuzzleDefinition
from backtracker import Backtracker
from ui import ui
from core import board

if __name__ == '__main__':

    parser = argparse.ArgumentParser()
    parser.add_argument('-conf', type=str, required=True, help='Definition file')
    parser.add_argument('-hints', type=str, required=False, default=None, help='Hint file')
    args = parser.parse_args()

    puzzle_def = PuzzleDefinition()
    puzzle_def.load(args.conf, args.hints)

    board = board.Board(puzzle_def)


    selected_from = None
    selected_to = None

    LEFT_BUTTON = 1
    RIGHT_BUTTON = 2

    backtracker_count = 8
    backtrackers = [Backtracker(board)]
    bests = [0]
    for i in range(backtracker_count - 1):
        backtrackers.append(copy.deepcopy(backtrackers[0]))
        bests.append(0)

    start = time.time()
    running_min = 0
    running_sec = 0

    keep_threshold = 415
    time_threshold = 20*60

    branching_iterations = 100
    explored_str = ""

    ui = ui.BoardUi(board)
    ui.init()

    best = 0
    next_check = time.time() + time_threshold
    iteration = 0
    while True:
        for idx, backtracker in enumerate(backtrackers):
            backtracker.step()
            explored = backtracker.explored_count()

            if explored > bests[idx]:
                bests[idx] = explored

            score = backtrackers[idx].board.evaluate()
            if score > best:
                best = score
                ui.board = backtrackers[idx].board
                ui.update()

        iteration += 1

        if iteration >= branching_iterations:

            max_explored = bests[0]
            max_idx = 0
            for idx, explored in enumerate(bests):
                if explored > max_explored:
                    max_explored = explored
                    max_idx = idx

            rest = ""
            for x in bests:
                rest += f"{x:.2E},"
            print(f"Branching {max_explored:.2E} > {rest}")

            try:
                explored_str = f'ExplRat:{backtrackers[max_idx].explored_ratio():.2E}% ExplAbs:{backtrackers[max_idx].explored_count():.2E}'
            except Exception as e:
                explored_str = f'ExplRat:ERR% ExplAbs:ERR'


            # make the best backtracker the main one
            backtrackers = [backtrackers[max_idx]]
            backtracker_count //= 2
            branching_iterations *= 2
            if backtracker_count < 1:
                backtracker_count = 1
            bests = [0]
            for i in range(backtracker_count - 1):
                backtrackers.append(copy.deepcopy(backtrackers[0]))
                bests.append(0)

            iteration = 0

        for event in pygame.event.get():
            if event.type == pygame.locals.QUIT:
                pygame.quit()
                sys.exit()

        pygame.display.set_caption(f'S {best}/{ui.board.max_score()} {explored_str} (branching)')
        pygame.display.update()



