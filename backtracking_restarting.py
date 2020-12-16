import argparse
import sys
import time
import uuid

import pygame.locals

from backtracker import Backtracker
from core import board
from core.defs import PuzzleDefinition
from ui import ui

if __name__ == '__main__':

    parser = argparse.ArgumentParser()
    parser.add_argument('-conf', type=str, required=True, help='Definition file')
    parser.add_argument('-hints', type=str, required=False, default=None, help='Hint file')
    parser.add_argument('-stats', type=str, required=False, default=None, help='Collocation stats file')
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

    connecting = False
    backtracker = Backtracker(board, connecting=connecting, grid_file=args.stats)
    start = time.time()
    running_min = 0
    running_sec = 0

    # thresholds = [(20, 100), (60,250), (300,300), (600,400), (1200,415)]
    thresholds = [(60, 400), (1200, 415)]

    # keep_threshold = 415
    # time_threshold = 20*60

    next_threshold_idx = 0

    best = 0
    next_check = start + thresholds[next_threshold_idx][0]
    while True:
        backtracker.step()
        if best < board.evaluate():
            ui.update()
            best = board.evaluate()
            if best >= thresholds[-1][1]:
                board.save("save " + str(uuid.uuid4()) + "_" + str(best) + ".csv")
            # we fill in remaining pieces to have a slightly better score

        if time.time() >= next_check != -1:
            if best < thresholds[next_threshold_idx][1]:
                print(f"{best} is not good enough, restarting backtracker")
                best = 0
                board.clear()
                backtracker = Backtracker(board, connecting=connecting)
                start = time.time()
                next_threshold_idx = 0
            else:
                print(f"threshold {thresholds[next_threshold_idx][1]} passed")
                next_threshold_idx += 1

            if next_threshold_idx < len(thresholds):
                next_check = start + thresholds[next_threshold_idx][0]
            else:
                next_check = -1

        for event in pygame.event.get():
            if event.type == pygame.locals.QUIT:
                pygame.quit()
                sys.exit()
            elif event.type == pygame.KEYDOWN:
                if event.key == pygame.K_i:
                    ui.marks_enabled = not ui.marks_enabled
                    ui.update()

        if backtracker.state != backtracker.SOLVED:
            running = int(time.time() - start)
            running_min = running // 60
            running_sec = running % 60
        pygame.display.set_caption(f'S {best}/{board.max_score()} {running_min}:{running_sec:02} (restarting)')
        pygame.display.update()
