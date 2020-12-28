import math
from decimal import Decimal

import networkx as nx

from core.defs import PieceRef, N, E, S, W


class NoSolution(Exception):
    pass


def reduce_edges(top, down, edges):
    # creating bipartite graph
    G = nx.Graph()
    G.add_nodes_from(top, bipartite=0)  # Add the node attribute "bipartite"
    G.add_nodes_from(down, bipartite=1)
    G.add_edges_from(edges)

    # get maximum pairing
    max = nx.algorithms.bipartite.matching.hopcroft_karp_matching(G, top_nodes=top)

    # the algorithm returns edges as a dictionary for nodes from each side,
    # so the maximal required is two times the number of nodes (in either group)
    if len(max) < 2 * len(top):
        raise NoSolution("no solution exists")

    # generate directed graph corresponding to the matching
    G0 = nx.DiGraph()
    for x in top:
        G0.add_edge(x, max[x])
    for node in down:
        G0.add_node(node)
    for e in G.edges(down):
        if e[0] != max[e[1]]:
            G0.add_edge(*e)

    used = set()

    # mark simple paths edges starting at free vertices as used
    for vertex in down:
        if vertex not in max:
            for edge in nx.bfs_edges(G0, vertex):
                used.add(edge)

    # generate strongly connected component
    for component in nx.strongly_connected_component_subgraphs(G0):
        if component.edges():
            for e in component.edges():
                used.add(e)
        else:
            for node in component.nodes:
                if node in max:
                    used.add((node, max[node]))
                break

    to_remove = dict()
    for e in G.edges():
        if e not in used and (e[1], e[0]) not in used:
            if e[0] not in to_remove:
                to_remove[e[0]] = set()
            to_remove[e[0]].add(e[1])

    return to_remove


class Backtracker:
    SEARCHING = 0
    BACKTRACKING = 1
    FINALIZING = 2
    SOLVED = 3

    def __init__(self, board,
                 enable_finalizing=True,
                 constraint_reducing=True,
                 connecting=False,
                 pieces_map=None,
                 find_all=False,
                 grid_file=None,
                 rotations_file=None):
        # pre-calculated factorials
        self.fact = [math.factorial(i) for i in range(4 * 256 + 1)]  # 4 times for each rotation
        self.enable_finalizing = enable_finalizing
        self.constraint_reducing = constraint_reducing
        self.connecting = connecting
        self.find_all = find_all
        self.grid_file = grid_file
        self.grid_scores = None
        self.finalizing_threshold = 90
        self.counter = 0
        self.placed_best = 0
        self.board = board
        self.unvisited = dict()
        self.backtrack_to = 0
        self.state = self.SEARCHING
        for i in range(self.board.puzzle_def.height):
            for j in range(self.board.puzzle_def.width):
                if not pieces_map or (i, j) in pieces_map:
                    self.unvisited[i, j] = 0
        self.visited = list()
        self.placed_ids = set()
        # self.unplaced_ids = set()
        self.unplaced_corners_ids = set()
        self.unplaced_corners = set()

        rotations = None
        if rotations_file is not None:
            rotations = {}
            with open(rotations_file) as f:
                for line in f.readlines():
                    id, dir = line.split(",")
                    rotations[int(id)] = int(dir)

        for piece in self.board.puzzle_def.corners:
            if rotations is not None:
                self.unplaced_corners.add(PieceRef(piece, rotations[piece.id], 0, 0))
            else:
                for dir in range(4):
                    self.unplaced_corners.add(PieceRef(piece, dir, 0, 0))
            self.unplaced_corners_ids.add(piece.id)
        self.unplaced_edges = set()
        self.unplaced_edges_ids = set()
        for piece in self.board.puzzle_def.edges:
            if rotations is not None:
                self.unplaced_edges.add(PieceRef(piece, rotations[piece.id], 0, 0))
            else:
                for dir in range(4):
                    self.unplaced_edges.add(PieceRef(piece, dir, 0, 0))
            self.unplaced_edges_ids.add(piece.id)
        self.unplaced_inner = set()
        self.unplaced_inner_ids = set()
        for piece in self.board.puzzle_def.inner:
            if rotations is not None:
                self.unplaced_inner.add(PieceRef(piece, rotations[piece.id], 0, 0))
            else:
                for dir in range(4):
                    self.unplaced_inner.add(PieceRef(piece, dir, 0, 0))
            self.unplaced_inner_ids.add(piece.id)
        self.forbidden = {}
        for i in range(self.board.puzzle_def.height):
            for j in range(self.board.puzzle_def.width):
                self.forbidden[i, j] = dict()
        self.backtracked_position = None
        self.explored = [0] * len(self.board.puzzle_def.all)
        self.explored_max = self.fact[4] \
                            * self.fact[len(self.board.puzzle_def.edges)] \
                            * self.fact[len(self.board.puzzle_def.inner)] \
                            * (4 ** len(self.board.puzzle_def.inner))

        # pre-place hints
        for hint in self.board.puzzle_def.hints:
            id = hint[2]
            self.placed_ids.add(id)
            piece_def = self.board.puzzle_def.all[id]
            i, j = hint[0], hint[1]
            ref = PieceRef(piece_def, 0, i, j)
            self.board.board[i][j] = ref
            if hint[3] != -1:
                ref.dir = hint[3]

            if id in self.unplaced_corners_ids:
                self.unplaced_corners_ids.remove(id)
            if id in self.unplaced_edges_ids:
                self.unplaced_edges_ids.remove(id)
            if id in self.unplaced_inner_ids:
                self.unplaced_inner_ids.remove(id)

            to_remove = set()
            for piece in self.unplaced_corners:
                if piece.piece_def.id == id:
                    to_remove.add(piece)

            for piece in to_remove:
                self.unplaced_corners.remove(piece)

            to_remove = set()
            for piece in self.unplaced_edges:
                if piece.piece_def.id == id:
                    to_remove.add(piece)

            for piece in to_remove:
                self.unplaced_edges.remove(piece)

            to_remove = set()
            for piece in self.unplaced_inner:
                if piece.piece_def.id == id:
                    to_remove.add(piece)

            for piece in to_remove:
                self.unplaced_inner.remove(piece)

            if (i, j) in self.unvisited:
                del self.unvisited[i, j]
            self.visited.append((i, j))
            self.board.marks[i][j] = id

        if self.grid_file:
            self.grid_scores = dict()

            with open(self.grid_file, "r") as f:
                for line in f.readlines():
                    fields = line.strip().split(",")
                    for i, field in enumerate(fields):
                        fields[i] = int(field)
                    self.grid_scores[fields[0], fields[1]] = fields[2]

        self.board.fix_orientation()

    def backtrack(self):
        if not self.visited:
            return False

        removing = self.visited.pop()
        self.unvisited[removing] = 0
        i = removing[0]
        j = removing[1]
        stack_pos = len(self.visited)

        # updated explored statistics
        self.explored[stack_pos] += self.fact[len(self.unplaced_corners_ids)] \
                                    * self.fact[len(self.unplaced_edges_ids)] \
                                    * self.fact[len(self.unplaced_inner_ids)] * (4 ** len(self.unplaced_inner_ids))
        if len(self.explored) > stack_pos + 1:
            self.explored[stack_pos + 1] = 0

        if stack_pos not in self.forbidden[removing]:
            self.forbidden[removing][stack_pos] = set()
        self.forbidden[removing][stack_pos].add(self.board.board[i][j])
        if self.board.is_corner(i, j):
            self.unplaced_corners.add(self.board.board[i][j])
            self.unplaced_corners_ids.add(self.board.board[i][j].piece_def.id)
        elif self.board.is_edge(i, j):
            self.unplaced_edges.add(self.board.board[i][j])
            self.unplaced_edges_ids.add(self.board.board[i][j].piece_def.id)
        else:
            self.unplaced_inner.add(self.board.board[i][j])
            self.unplaced_inner_ids.add(self.board.board[i][j].piece_def.id)
        self.placed_ids.remove(self.board.board[i][j].piece_def.id)
        self.board.board[i][j] = None

        # remove all forbidden due to later position
        for pos in self.forbidden:
            to_remove_forbidden_keys = set()
            for key in self.forbidden[pos]:
                if key > stack_pos:
                    to_remove_forbidden_keys.add(key)
            for key in to_remove_forbidden_keys:
                del self.forbidden[pos][key]

        self.backtracked_position = removing

    def check_feasible(self, ignore_impossible=False):
        # evaluate which of the unvisited nodes has least number of possibilities
        self.best_score = -1
        self.best_unplaced_container = None

        self.feasible_pieces = {}
        for unvisited in self.unvisited:
            i = unvisited[0]
            j = unvisited[1]
            self.feasible_pieces[i, j] = set()

            if self.board.is_corner(i, j):
                possible = self.unplaced_corners
                possible_ids = self.unplaced_corners_ids
            elif self.board.is_edge(i, j):
                possible = self.unplaced_edges
                possible_ids = self.unplaced_edges_ids
            else:
                possible = self.unplaced_inner
                possible_ids = self.unplaced_inner_ids

            if (self.connecting or self.board.is_inner(i, j)) and self.visited:
                # for inner pieces, check if they have any neighbours, otherwise we
                # are wasting time computing those
                neighbours = 0
                if i > 0:
                    if self.board.board[i - 1][j]:
                        neighbours += 1
                if j > 0:
                    if self.board.board[i][j - 1]:
                        neighbours += 1
                height = self.board.puzzle_def.height
                if i < height - 1:
                    if self.board.board[i + 1][j]:
                        neighbours += 1
                width = self.board.puzzle_def.width
                if j < width - 1:
                    if self.board.board[i][j + 1]:
                        neighbours += 1
                if neighbours == 0:
                    self.feasible_pieces[i, j] = None
                    continue

            feasible_possibilities = set(possible)
            for key in self.forbidden[i, j]:
                feasible_possibilities -= self.forbidden[i, j][key]
            for feasible in feasible_possibilities:
                if feasible.piece_def.id not in self.placed_ids:
                    if self.can_be_placed_at(feasible, i, j):
                        self.feasible_pieces[i, j].add(feasible)

        if not ignore_impossible and self.constraint_reducing:
            # # reduce the domain using Regin's algorithm for alldifferences constraint filtering
            # # 1. formulate constraints as nodes and edges
            top = list()  # positions
            down = list(set(range(1, len(self.board.puzzle_def.all) + 1)) - self.placed_ids)  # piece id's
            edges = []
            for unvisited in self.unvisited:
                if self.feasible_pieces[unvisited]:
                    top.append(unvisited)
                    for feasible in self.feasible_pieces[unvisited]:
                        edges.append((unvisited, feasible.piece_def.id))

            try:
                to_reduce = reduce_edges(top, down, edges)
            except NoSolution as e:
                # self.best_score should be at -1 now
                if self.best_score != -1:
                    raise Exception("best_score not initialized to -1 when expected")
                return

            reduced_something = False
            for pos in to_reduce:
                if self.feasible_pieces[pos]:
                    to_remove_refs = []
                    for feasible in self.feasible_pieces[pos]:
                        if feasible.piece_def.id in to_reduce[pos]:
                            to_remove_refs.append(feasible)
                    for to_remove in to_remove_refs:
                        self.feasible_pieces[pos].remove(to_remove)
                        reduced_something = True

        # reduce the constraints for domains which occur too often
        # (e.g. if three positions all have domain {2,3}, then this is impossible
        # DISABLED - too slow
        # deg = {}
        # for unvisited in self.unvisited:
        #     i1 = unvisited[0]
        #     j1 = unvisited[1]
        #     deg[i1, j1] = 0
        #
        # for unvisited in self.unvisited:
        #     i1 = unvisited[0]
        #     j1 = unvisited[1]
        #
        #     for unvisited2 in self.unvisited:
        #         i2 = unvisited2[0]
        #         j2 = unvisited2[1]
        #         if i1!=i2 or j1!=j2:
        #             if feasible_pieces[i1, j1] == feasible_pieces[i2, j2]:
        #                 deg[i1,j1] += 1
        #
        # for unvisited in self.unvisited:
        #     i1 = unvisited[0]
        #     j1 = unvisited[1]
        #     size = len(feasible_pieces[i1, j1])
        #     if size > 1:
        #         if deg[i1, j1] == size - 1:
        #             # TODO update all feasible pieces
        #             pass
        #         elif deg[i1, j1] > size - 1:
        #             # impossible...
        #             self.backtrack()
        #             return False

        self.best_feasible_locations = []
        for unvisited in self.unvisited:
            i = unvisited[0]
            j = unvisited[1]
            feasible_piece = None

            if self.board.is_corner(i, j):
                possible = self.unplaced_corners
                possible_ids = self.unplaced_corners_ids
            elif self.board.is_edge(i, j):
                possible = self.unplaced_edges
                possible_ids = self.unplaced_edges_ids
            else:
                possible = self.unplaced_inner
                possible_ids = self.unplaced_inner_ids

            if self.feasible_pieces[i, j] is None:
                continue

            score = len(self.feasible_pieces[i, j])

            if score == 0:
                self.best_score = 0
                break

            if score == self.best_score and self.best_unplaced_container == possible:
                self.best_feasible_locations.append((i, j))
            elif (self.best_score == -1 or score < self.best_score) \
                    and (not ignore_impossible or score > 0):
                self.best_score = score
                for p in self.feasible_pieces[i, j]:
                    feasible_piece = p
                    break
                self.best_feasible_locations = []
                self.best_feasible_locations.append((i, j))
                self.best_unplaced_container = possible
                self.best_unplaced_container_ids = possible_ids

            if self.best_score == 0:
                # impossible to place anything here...
                break

    def place(self, i, j, ref):
        ref.i = i
        ref.j = j
        self.board.board[i][j] = ref
        self.best_unplaced_container.remove(ref)
        self.best_unplaced_container_ids.remove(ref.piece_def.id)
        del self.unvisited[i, j]
        self.visited.append((i, j))
        self.board.marks[i][j] = ref.piece_def.id
        if self.backtracked_position and self.backtracked_position != (i, j):
            raise Exception("Removing backtracking position when placing piece?!?")
            # self.forbidden[self.backtracked_position].clear()
        self.backtracked_position = None
        self.placed_ids.add(ref.piece_def.id)

    def step(self):

        if self.state == self.SOLVED:
            return
        elif self.state == self.FINALIZING:
            # we are just putting any piece we can, until we can
            # and then we backtrack
            self.check_feasible(ignore_impossible=True)
            if self.best_score == -1:
                self.state = self.BACKTRACKING
            else:
                i = j = -1
                piece = None
                # select place with most neighbours, so that we at least put near
                # to already filled board
                most_neighbours = -1
                best_piece = None
                best_coord = None
                for i, j in self.best_feasible_locations:
                    count = self.board.neighbours_count(i, j)
                    if count > most_neighbours:
                        most_neighbours = count
                        for best_piece in self.feasible_pieces[i, j]:
                            break
                        best_coord = (i, j)
                        if most_neighbours == 4:
                            break
                self.place(best_coord[0], best_coord[1], best_piece)

        elif self.state == self.BACKTRACKING:
            self.backtrack()
            if self.backtrack_to == len(self.visited):
                self.backtrack_to = 0
                self.backtracked_position = None
                self.state = self.SEARCHING
        elif self.state == self.SEARCHING:

            if not self.unvisited:
                if self.find_all:
                    self.counter += 1
                    print(f"Next solution found ({self.counter})")
                else:
                    # everything already placed, solved...
                    self.state = self.SOLVED
                    return False

            self.check_feasible()

            if self.best_score <= 0:
                # impossible to place anything here... backtrack
                self.backtrack_to = len(self.visited) - 1
                if len(self.unvisited) < self.finalizing_threshold and self.enable_finalizing:
                    self.state = self.FINALIZING
                    # decrease threshold a bit
                    if self.finalizing_threshold > 70:
                        self.finalizing_threshold -= 1
                        print(f"debug: finalizing threshold = {self.finalizing_threshold}")
                else:
                    self.state = self.BACKTRACKING

                # self.backtrack()
                return False

            # from the set of selected pieces, select one with least number
            # of possibilities (most constrained)

            # i,j = self.best_feasible_locations[random.randint(0, len(self.best_feasible_locations) - 1)]
            # for best_feasible_piece in self.feasible_pieces[i,j]:
            #     break

            # counts = {}
            if self.grid_scores:
                max_score = -1
                max_repre = None
                for i, j in self.best_feasible_locations:
                    for piece in self.feasible_pieces[i, j]:
                        score = 0
                        for neighbour in self.board.enumerate_neigbours(i, j, diagonal=True):
                            if (piece.piece_def.id, neighbour.piece_def.id) in self.grid_scores:
                                score += self.grid_scores[piece.piece_def.id, neighbour.piece_def.id]
                            else:
                                # these two pieces do not work well together at all!
                                score = 0
                                break
                        else:
                            score = 1

                        if score > max_score:
                            max_score = score
                            max_repre = [i, j, piece]

                i = max_repre[0]
                j = max_repre[1]
                best_feasible_piece = max_repre[2]
            else:

                # from the set of selected pieces, select one with least number
                # of possibilities (most constrained)
                counts = {}
                for i, j in self.best_feasible_locations:
                    for piece in self.feasible_pieces[i, j]:
                        if piece.piece_def.id not in counts:
                            counts[piece.piece_def.id] = [0, None]  # count, representative
                        counts[piece.piece_def.id][0] += 1
                        counts[piece.piece_def.id][1] = (i, j, piece)

                max_count = -1
                max_repre = None
                for id, info in counts.items():
                    count = info[0]
                    if max_count == -1 or count > max_count:
                        max_count = count
                        max_repre = info[1]

                i = max_repre[0]
                j = max_repre[1]
                best_feasible_piece = max_repre[2]

            # now we have selected piece to place, do it
            self.place(i, j, best_feasible_piece)
            return True

    def can_be_placed_at(self, piece, i, j):
        if i > 0:
            up = self.board.board[i - 1][j]
            if up and up.get_color(S) != piece.get_color(N):
                return False
        if j > 0:
            left = self.board.board[i][j - 1]
            if left and left.get_color(E) != piece.get_color(W):
                return False
        height = self.board.puzzle_def.height
        if i < height - 1:
            down = self.board.board[i + 1][j]
            if down and down.get_color(N) != piece.get_color(S):
                return False
        width = self.board.puzzle_def.width
        if j < width - 1:
            right = self.board.board[i][j + 1]
            if right and right.get_color(W) != piece.get_color(E):
                return False

        # corners/edges special checks
        if i == 0:
            if j == 0:
                if piece.dir != E:
                    return False
            elif j == width - 1:
                if piece.dir != S:
                    return False
            else:
                if piece.dir != E:
                    return False
        elif i == height - 1:
            if j == 0:
                if piece.dir != N:
                    return False
            elif j == width - 1:
                if piece.dir != W:
                    return False
            else:
                if piece.dir != W:
                    return False
        elif j == 0:
            if piece.dir != N:
                return False
        elif j == width - 1:
            if piece.dir != S:
                return False

        return True

    def explored_count(self):
        return Decimal(sum(self.explored))

    def explored_ratio(self):
        return self.explored_count() / Decimal(self.explored_max)
