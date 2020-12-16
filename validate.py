import argparse
from pygame.locals import *
from core.defs import PuzzleDefinition

if __name__ == '__main__':

    parser = argparse.ArgumentParser()
    parser.add_argument('-conf', type=str, required=True, help='Definition file')
    parser.add_argument('-hints', type=str, required=False, default=None, help='Hint file')
    args = parser.parse_args()

    puzzle_def = PuzzleDefinition()
    puzzle_def.load(args.conf, args.hints)

    # 1. number of each color occurrences must be even
    counts = {}
    for id, piece in puzzle_def.all.items():
        for color in piece.colors:
            if color not in counts:
                counts[color] = 0
            counts[color] += 1

    for color in counts:
        if counts[color] % 2 != 0:
            raise Exception("Validation failure, color %i not in even count" % color)

    # 2. number of "left" and "right" colors on the edge must match
    left_counts = {}
    right_counts = {}
    for piece in puzzle_def.corners:
        right_color = piece.colors[0]
        left_color = piece.colors[1]
        if left_color not in left_counts:
            left_counts[left_color] = 0
        left_counts[left_color] += 1
        if right_color not in right_counts:
            right_counts[right_color] = 0
        right_counts[right_color] += 1

    for piece in puzzle_def.edges:
        right_color = piece.colors[0]
        left_color = piece.colors[2]
        if left_color not in left_counts:
            left_counts[left_color] = 0
        left_counts[left_color] += 1
        if right_color not in right_counts:
            right_counts[right_color] = 0
        right_counts[right_color] += 1

    if left_counts != right_counts:
        raise Exception("Validation failure, left and right colors counts on edge do not match" % color)




