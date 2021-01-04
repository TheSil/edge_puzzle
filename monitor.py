import argparse
import sys
import glob
import os
import time
import pygame.locals
from core.defs import PuzzleDefinition, PieceRef
from ui import ui
from core import board

if __name__ == '__main__':

    parser = argparse.ArgumentParser()
    parser.add_argument('-conf', type=str, required=True, help='Definition file')
    parser.add_argument('-dir', type=str, default=".", help='Folder to monitor')
    args = parser.parse_args()

    puzzle_def = PuzzleDefinition()
    puzzle_def.load(args.conf)

    board_inst = board.Board(puzzle_def)
    ui = ui.BoardUi(board_inst)
    ui.init()

    prev_csv_files = set()
    max_score = 0

    def update(filename):
        try:

            board_inst = board.Board(puzzle_def)
            board_inst.load(filename)
            score = board_inst.evaluate()
            global max_score
            if score >= max_score:
                print(f"Loading new file {filename}")
                max_score = score
                ui.board = board_inst
                ui.update()
                caption = f'S {board_inst.evaluate()}/{board_inst.max_score()}'
                pygame.display.set_caption(caption)
        except:
            pass

    next_check = time.time() + 3
    while True:
        if time.time() >= next_check:
            next_check = time.time() + 3
            csv_files = set(glob.glob(os.path.join(args.dir, "*.csv")))
            new = csv_files - prev_csv_files
            prev_csv_files = csv_files
            if new:
                for file in new:
                    update(file)

        for event in pygame.event.get():
            if event.type == pygame.locals.QUIT:
                pygame.quit()
                sys.exit()
            elif event.type == pygame.KEYDOWN:
                if event.key == pygame.K_i:
                    ui.marks_enabled = not ui.marks_enabled
                    ui.update()

        pygame.display.update()
        time.sleep(0.5)




