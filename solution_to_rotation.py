import argparse
from core.defs import PuzzleDefinition, PieceRef
from core import board

if __name__ == '__main__':

    parser = argparse.ArgumentParser()
    parser.add_argument('-conf', type=str, required=True, help='Definition file')
    parser.add_argument('-load', type=str, required=True, default=None, help='Solution file')
    parser.add_argument('-save', type=str, required=True, default=None, help='Rotations file output')
    args = parser.parse_args()

    puzzle_def = PuzzleDefinition()
    puzzle_def.load(args.conf, args.load)

    board = board.Board(puzzle_def)

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

    with open(args.save, "w") as out:
        for i in range(board.puzzle_def.height):
            for j in range(board.puzzle_def.width):
                out.write(f"{board.board[i][j].piece_def.id}, {board.board[i][j].dir}\n")


