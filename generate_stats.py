
OFFSET_ID = 0
OFFSET_ID1 = 1
OFFSET_ID2 = 2
OFFSET_ID3 = 3
OFFSET_ID4 = 4
OFFSET_ORD1 = 5
OFFSET_DIR1 = 6
OFFSET_ORD2 = 7
OFFSET_DIR2 = 8
OFFSET_ORD3 = 9
OFFSET_DIR3 = 10
OFFSET_ORD4 = 11
OFFSET_DIR4 = 12
OFFSET_NEXT1 = 13
OFFSET_NEXT2 = 14
OFFSET_NEXT3 = 15
OFFSET_NEXT4 = 16


import argparse
from core import board
from core.defs import PuzzleDefinition

if __name__ == '__main__':

    parser = argparse.ArgumentParser()
    parser.add_argument('-conf', type=str, required=True, help='Definition file')
    parser.add_argument('-hints', type=str, required=False, default=None, help='Hint file')
    parser.add_argument('-i', type=str, required=True, help='Input patterns file')
    parser.add_argument('-o', type=str, required=True, help='Output stats file')
    args = parser.parse_args()

    puzzle_def = PuzzleDefinition()
    puzzle_def.load(args.conf, args.hints)

    board = board.Board(puzzle_def)

    print("Loading 2x2 patterns...")
    grid_scores = dict()
    with open(args.i, "r") as f:
        f.readline()  # consume header
        for line in f.readlines():
            fields = line.strip().split(",")
            for i, field in enumerate(fields):
                fields[i] = int(field)

            for id1_idx in range(3):
                for id2_idx in range(id1_idx + 1, 4):
                    id1 = fields[1 + id1_idx]
                    id2 = fields[1 + id2_idx]
                    if (id1, id2) not in grid_scores:
                        grid_scores[id1, id2] = 0
                    grid_scores[id1, id2] += 1
                    if (id2, id1) not in grid_scores:
                        grid_scores[id2, id1] = 0
                    grid_scores[id2, id1] += 1

    with open(args.o, "w") as f:
        for id1, id2 in grid_scores:
            f.write(f"{id1},{id2},{grid_scores[id1, id2]}\n")


