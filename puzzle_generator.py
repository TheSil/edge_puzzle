import random
import argparse
from core.defs import E, S, W, N

# actual pieces init - color lists in clock-wise order
class PieceDef:
    def __init__(self, E=0, S=0, W=0, N=0):
        self.colors = [E, S, W, N]

    def get_color(self, idx):
        return self.colors[idx]

    def set_color(self, idx, color):
        self.colors[idx] = color

    def __repr__(self):
        return str(self.colors)

    def __hash__(self):
        return hash(tuple(self.colors))

    def __eq__(self, other):
        return isinstance(other, PieceDef) and self.__dict__ == other.__dict__


# use the sequence and de-serialize it into individual pieces
# we need to be careful so that neighbour edges match
# for this we define virtual 2D board with corresponding mapping
class PieceRef:
    def __init__(self, piece_def, dir):
        self.piece_def = piece_def
        self.dir = dir

    def get_color(self, pos):
        # orientation E => index 0 at E, orientation S => index 0 at S, etc
        idx = (pos - self.dir) % 4
        return self.piece_def.get_color(idx)

    def set_color(self, pos, color):
        # orientation E => index 0 at E, orientation S => index 0 at S, etc
        idx = (pos - self.dir) % 4
        self.piece_def.set_color(idx, color)


def generate_puzzle(height, width, unique_edge_colors_count, unique_inner_colors_count, color_counts):
    # derived parameters
    pieces_count = height * width
    corners_count = 4
    edges_count = 2 * (height - 2) + 2 * (width - 2)
    inner_count = (height - 2) * (width - 2)
    edge_color_subpieces = edges_count + corners_count
    inner_color_subpieces = (height - 2) * (height - 1) * 2
    # color_subpieces = edge_color_subpieces + inner_color_subpieces

    corners = [PieceDef() for _ in range(4)]
    edges = [PieceDef() for _ in range(edges_count)]
    inner = [PieceDef() for _ in range(inner_count)]

    board = [width * [PieceRef(0, 0)] for i in range(height)]
    E = 0
    S = 1
    W = 2
    N = 3
    board[0][0] = PieceRef(corners[0], E)
    board[0][width - 1] = PieceRef(corners[1], S)
    board[height - 1][width - 1] = PieceRef(corners[2], W)
    board[height - 1][0] = PieceRef(corners[3], N)

    # edges
    edge_index = 0
    # upper edges
    i = 0
    for j in range(1, width - 1):
        board[i][j] = PieceRef(edges[edge_index], E)
        edge_index += 1
    # right edges
    j = width - 1
    for i in range(1, height - 1):
        board[i][j] = PieceRef(edges[edge_index], S)
        edge_index += 1
    # bottom edges
    i = height - 1
    for j in range(1, width - 1):
        board[i][j] = PieceRef(edges[edge_index], W)
        edge_index += 1
    # left edges
    j = 0
    for i in range(1, height - 1):
        board[i][j] = PieceRef(edges[edge_index], N)
        edge_index += 1
    # inner
    inner_idx = 0
    for i in range(1, height - 1):
        for j in range(1, width - 1):
            board[i][j] = PieceRef(inner[inner_idx], E)
            inner_idx += 1

    # generate the pieces colors
    random_edge_permutation = edge_color_subpieces * [0]
    random_inner_permutation = inner_color_subpieces * [0]
    if not color_counts:
        for i in range(edge_color_subpieces):
            random_edge_permutation[i] = random.randint(1, unique_edge_colors_count)
        for i in range(inner_color_subpieces):
            random_inner_permutation[i] = random.randint(unique_edge_colors_count + 1,
                                                         unique_edge_colors_count + unique_inner_colors_count)
    else:
        random_edge_permutation = []
        random_inner_permutation = []
        for color_count_idx in range(unique_edge_colors_count):
            color = color_count_idx + 1
            random_edge_permutation += [color] * color_counts[color_count_idx]
        for color_count_idx in range(unique_edge_colors_count, unique_inner_colors_count + unique_edge_colors_count):
            color = color_count_idx + 1
            random_inner_permutation += [color] * color_counts[color_count_idx]
        random.shuffle(random_edge_permutation)
        random.shuffle(random_inner_permutation)

        # raise NotImplemented("predefined color counts not implemented")

    sequence_index = 0

    # edge colors
    # top edges
    i = 0
    for j in range(0, width - 1):
        color = random_edge_permutation[sequence_index]
        left = board[i][j]
        right = board[i][j + 1]
        left.set_color(E, color)
        right.set_color(W, color)
        sequence_index += 1

    # right edges
    j = width - 1
    for i in range(0, height - 1):
        color = random_edge_permutation[sequence_index]
        up = board[i][j]
        down = board[i + 1][j]
        up.set_color(S, color)
        down.set_color(N, color)
        sequence_index += 1

    # bottom edges
    i = height - 1
    for j in range(0, width - 1):
        color = random_edge_permutation[sequence_index]
        left = board[i][j]
        right = board[i][j + 1]
        left.set_color(E, color)
        right.set_color(W, color)
        sequence_index += 1

    # left edges
    j = 0
    for i in range(0, height - 1):
        color = random_edge_permutation[sequence_index]
        up = board[i][j]
        down = board[i + 1][j]
        up.set_color(S, color)
        down.set_color(N, color)
        sequence_index += 1

    # inner edges
    sequence_index = 0
    for i in range(1, height):
        for j in range(1, width - 1):
            center = board[i][j]
            up = board[i - 1][j]

            color = random_inner_permutation[sequence_index]
            center.set_color(N, color)
            up.set_color(S, color)
            sequence_index += 1

    for i in range(1, height - 1):
        for j in range(1, width):
            center = board[i][j]
            left = board[i][j - 1]

            color = random_inner_permutation[sequence_index]
            center.set_color(W, color)
            left.set_color(E, color)
            sequence_index += 1

    return board, corners, edges, inner


def validate(board, height, width):
    def check(color1, color2):
        if color1 != color2:
            raise Exception("Not valid color placement")

    for i in range(1, height):
        for j in range(1, width):

            center = board[i][j]
            up = board[i - 1][j] if i > 0 else None
            down = board[i + 1][j] if i < N - 1 else None
            left = board[i][j - 1] if j > 0 else None
            right = board[i][j + 1] if j < N - 1 else None

            if up:
                check(center.get_color(N), up.get_color(S))
            if down:
                check(center.get_color(S), down.get_color(N))
            if right:
                check(center.get_color(E), right.get_color(W))
            if left:
                check(center.get_color(W), left.get_color(E))


def has_duplicates(container):
    scontainer = set(container)

    if len(container) != len(scontainer):
        return True

    for piece in container:
        for rot in (1, 2, 3):
            rotated = piece.colors[rot:] + piece.colors[0:rot]
            if PieceDef(*rotated) in container:
                return True

    return False


if __name__ == '__main__':
    # use colors_inner_counts = "24, 24, 24, 25, 25, 25, 24, 24, 25, 25, 25, 25, 25, 25, 25, 25, 25"
    # for exactly the same Eternity 2 inner colors counts

    parser = argparse.ArgumentParser()
    parser.add_argument('-height', type=int, default=16, help='Height')
    parser.add_argument('-width', type=int, default=16, help='Width')
    parser.add_argument('-colors_edge', type=int, default=5, help='Unique color on edge')
    parser.add_argument('-colors_inner', type=int, default=17, help='Unique colors inside')
    parser.add_argument('-colors_edge_counts', type=str, default=None,
                        help='Specific counts for individual edge color')
    parser.add_argument('-colors_inner_counts', type=str, default=None,
                        help='Specific counts for individual inner color')
    parser.add_argument('-no_duplicates', type=bool, default=True, help='Flag to avoid duplicate pieces.')
    args = parser.parse_args()

    color_counts = []
    if args.colors_edge_counts and args.colors_inner_counts:
        edge_counts = [int(item) for item in args.colors_edge_counts.split(',')]
        inner_counts = [int(item) for item in args.colors_inner_counts.split(',')]
        color_counts = edge_counts + inner_counts

    height = args.height
    width = args.width
    unique_edge_colors_count = args.colors_edge
    unique_inner_colors_count = args.colors_inner
    no_duplicates = args.no_duplicates

    generated = False
    while not generated:
        board, corners, edges, inner = generate_puzzle(height, width, unique_edge_colors_count,
                                                       unique_inner_colors_count,
                                                       color_counts)
        if no_duplicates:
            if has_duplicates(corners) or has_duplicates(edges) or has_duplicates(inner):
                print("duplicates found, repeating...")
                continue

        break

    validate(board, height, width)

    # report
    print(f"height={height}, "
          f"width={width}, "
          f"pieces={len(corners) + len(edges) + len(inner)}, "
          f"corners={len(corners)}, "
          f"edges={len(edges)}, "
          f"inner={len(inner)}")

    print("corners:")
    for piece in corners:
        print(piece)

    print("edges:")
    for piece in edges:
        print(piece)

    print("inner:")
    for piece in inner:
        print(piece)

    # save shuffled pieces into the config file (csv for now)
    random_corner_idxs = list(range(len(corners)))
    random.shuffle(random_corner_idxs)
    random_edge_idxs = list(range(len(edges)))
    random.shuffle(random_edge_idxs)
    random_inner_idxs = list(range(len(inner)))
    random.shuffle(random_inner_idxs)

    with open(f"{height}_{width}_edge{unique_edge_colors_count}_inner{unique_inner_colors_count}.csv", "w") as f:
        f.write(f"{height},{width},{unique_edge_colors_count},{unique_inner_colors_count}\n")

        id = 1
        for piece_idx in random_corner_idxs:
            piece = corners[piece_idx]
            piece.shuffled_id = id
            piece.shuffled_orientation = -1
            f.write(f"{id},{piece.colors[0]},{piece.colors[1]}\n")
            id += 1

        for piece_idx in random_edge_idxs:
            piece = edges[piece_idx]
            piece.shuffled_id = id
            piece.shuffled_orientation = -1
            f.write(f"{id},{piece.colors[0]},{piece.colors[1]},{piece.colors[2]}\n")
            id += 1

        for piece_idx in random_inner_idxs:
            piece = inner[piece_idx]
            piece.shuffled_id = id
            piece.shuffled_orientation = random.randint(0, 3)
            f.write(f"{id},{piece.colors[(0 + piece.shuffled_orientation) % 4]}"
                    f",{piece.colors[(1 + piece.shuffled_orientation) % 4]}"
                    f",{piece.colors[(2 + piece.shuffled_orientation) % 4]}"
                    f",{piece.colors[(3 + piece.shuffled_orientation) % 4]}\n")
            id += 1

    with open(f"{height}_{width}_edge{unique_edge_colors_count}_inner{unique_inner_colors_count}_solution.csv",
              "w") as f:
        for i in range(0, height):
            for j in range(0, width):
                f.write(f"{i},{j},{board[i][j].piece_def.shuffled_id},{board[i][j].piece_def.shuffled_orientation}\n")
