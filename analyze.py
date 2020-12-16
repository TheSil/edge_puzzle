import argparse
from core.defs import PuzzleDefinition

if __name__ == '__main__':

    parser = argparse.ArgumentParser()
    parser.add_argument('-conf', type=str, required=True, help='Definition file')
    parser.add_argument('-hints', type=str, required=False, default=None, help='Hint file')
    args = parser.parse_args()

    puzzle_def = PuzzleDefinition()
    puzzle_def.load(args.conf, args.hints)

    colors_counts = {}
    for id, piece in puzzle_def.all.items():
        for color in piece.colors:
            if color != 0:
                if color not in colors_counts:
                    colors_counts[color] = 0
                colors_counts[color] +=1

    for color in sorted(colors_counts.keys()):
        print(f"color:{color}, count:{colors_counts[color]//2}" )

    edge_middle_colors = {}
    for edge in puzzle_def.edges:
        mc = edge.colors[1]
        if mc not in edge_middle_colors:
            edge_middle_colors[mc] = 0
        edge_middle_colors[mc] += 1

    print(edge_middle_colors)

    # analyze duplicate pieces
    pieces = {}
    for id, piece in puzzle_def.all.items():
        found = False
        for dir in range(4):
            rotated_colors = tuple(piece.colors[dir:] + piece.colors[:dir])
            if rotated_colors in pieces:
                found = True
                break
        if not found:
            pieces[rotated_colors] = []
        pieces[rotated_colors].append(id)

    for colors, ids in pieces.items():
        if len(ids) > 1:
            print("duplicates pieces:", colors, ids)

    import sys
    sys.exit(0)

    # analyzing how many inner pieces A,B can exist in form
    # A X
    # X B

    for A in puzzle_def.inner:
        ref = PieceRef(A, 0, 0, 0)
        matchin_down_total = 0
        matching_right_total = 0
        for dir_a in range(4):
            ref.dir = dir_a
            right_color_a = ref.get_color(E)
            down_color_a = ref.get_color(S)
            matching_down = 0
            matching_right = 0
            matching_pieces = 0
            for B in puzzle_def.inner:
                ref_b = PieceRef(B, 0, 0, 0)
                for dir_b in range(4):
                    ref_b.dir = dir_b
                    up_color_b = ref_b.get_color(N)
                    left_color_b = ref_b.get_color(W)
                    matching_down_this = 0
                    matching_right_this = 0

                    for C in puzzle_def.inner:
                        if C.id != A.id and C.id != B.id:
                            ref_c = PieceRef(C, 0, 0, 0)
                            for dir_c in range(4):
                                ref_c.dir = dir_c
                                if ref_c.get_color(N) == down_color_a and ref_c.get_color(N) == left_color_b:
                                    matching_down += 1
                                    matching_down_this += 1
                                if ref_c.get_color(W) == right_color_a and ref_c.get_color(S) == up_color_b:
                                    matching_right += 1
                                    matching_right_this += 1

                    if matching_down_this > 0 and matching_right_this > 0:
                        matching_pieces += 1

                    matchin_down_total += matching_down
                    matching_right_total += matching_right

            print(A.id, dir_a, matching_pieces)

            if matching_pieces == 0:
                print(A.id, dir_a, matching_pieces, "OUTTER PIECE!")
            # if matching_pieces == 1:
            #     print(A.id, dir_a, matching_pieces)




