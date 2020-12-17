import argparse
import sys
import random
import uuid
import copy
import pygame.locals
from core.defs import PuzzleDefinition, N, E, S, W, TYPE_CORNER, TYPE_EDGE
from ui import ui
from core import board


class Swapper:

    QUICK_SWAPPING = 0
    RANDOM_SHUFFLING = 1
    RANDOM_RECOVERING = 2

    def __init__(self, board):
        self.board = board
        self.cached_corners_idxs = list(board.enumerate_corners())
        self.cached_edge_idxs = list(board.enumerate_edges())
        self.cached_inner_idxs = list(board.enumerate_inner())
        self.state = self.QUICK_SWAPPING
        self.max_score = 0

        # throw away hint pieces
        for hint_def in board.puzzle_def.hints:
            i = hint_def[0]
            j = hint_def[1]
            if (i, j) in self.cached_corners_idxs:
                self.cached_corners_idxs.remove((i, j))
            if (i, j) in self.cached_edge_idxs:
                self.cached_edge_idxs.remove((i, j))
            if (i, j) in self.cached_inner_idxs:
                self.cached_inner_idxs.remove((i, j))

    def have_common_edge(self, piece1, piece2):
        if piece1[1] == piece2[1]:
            if piece1[0] + 1 == piece2[0]:
                # inner2 is below inner1
                if self.board.board[piece1[0]][piece1[1]].get_color(S) == \
                        self.board.board[piece2[0]][piece2[1]].get_color(N):
                    return True
            elif piece2[0] + 1 == piece1[0]:
                # inner1 is below inner2
                if self.board.board[piece1[0]][piece1[1]].get_color(N) == \
                        self.board.board[piece2[0]][piece2[1]].get_color(S):
                    return True
        elif piece1[0] == piece2[0]:
            if piece1[1] + 1 == piece2[1]:
                # inner2 is right to inner1
                if self.board.board[piece1[0]][piece1[1]].get_color(E) == \
                        self.board.board[piece2[0]][piece2[1]].get_color(W):
                    return True
            elif piece2[1] + 1 == piece1[1]:
                # inner1 is right to inner2
                if self.board.board[piece1[0]][piece1[1]].get_color(W) == \
                        self.board.board[piece2[0]][piece2[1]].get_color(E):
                    return True
        return False

    def shuffle(self):        # bring little bit of chaos sometimes (sort of mutation)
        # shuffle random pieces
        if random.randint(1,2) == 1:
            #print("shuffling random inner pieces...")
            inners = set(self.board.enumerate_inner())
            inners = list(inners)
            inners = inners[:5]
            to_remove = []
            for i, j in inners:
                if self.board.hints[i][j]:
                    to_remove.append((i,j))
            for i, j in to_remove:
                inners.remove((i,j))

            refs = [self.board.board[i][j] for (i,j) in inners]
            random.shuffle(inners)
            for k in range(len(refs)):
                i,j = inners[k]
                self.board.board[i][j] = refs[k]
                self.board.board[i][j].i = i
                self.board.board[i][j].j = j
        else:
            #print("shuffling random edge pieces...")
            inners = set(self.board.enumerate_edges())
            inners = list(inners)
            inners = inners[:10]
            to_remove = []
            for i, j in inners:
                if self.board.hints[i][j]:
                    to_remove.append((i, j))
            for i, j in to_remove:
                inners.remove((i, j))

            refs = [self.board.board[i][j] for (i, j) in inners]
            random.shuffle(inners)
            for k in range(len(refs)):
                i, j = inners[k]
                self.board.board[i][j] = refs[k]
                self.board.board[i][j].i = i
                self.board.board[i][j].j = j

        self.board.fix_orientation()
        self.board.heuristic_orientation()
        return True

    def quick_swaps(self):
        before = self.board.evaluate()
        max_after = before
        max_indices = None

        same_indices = []

        CORNERS = 0
        EDGES = 1
        INNER = 2

        seq = [INNER, EDGES, CORNERS]
        random.shuffle(seq)

        try:
            for type in seq:
                if type == CORNERS:
                    # see how much we gain be swapping corners ...
                    corner_idxs = self.cached_corners_idxs
                    random.shuffle(corner_idxs)
                    # throw away hint pieces

                    evals = [self.board.evaluate_piece(i, j) for i, j in corner_idxs]
                    evals, corner_idxs = zip(*sorted(zip(evals, corner_idxs)))

                    for idx1 in range(1, len(corner_idxs)):
                        for idx2 in range(idx1):
                            corner1 = corner_idxs[idx1]
                            corner2 = corner_idxs[idx2]

                            self.board.exchange(*corner1, *corner2)
                            self.board.marks[corner1[0]][corner1[1]], self.board.marks[corner2[0]][corner2[1]] = \
                                self.board.marks[corner2[0]][corner2[1]], self.board.marks[corner1[0]][corner1[1]]
                            self.board.fix_orientation()

                            after = board.evaluate()
                            if after > max_after:
                                max_after = after
                                max_indices = (corner1, corner2)
                                raise Exception()

                            if after == before:
                                same_indices.append((corner1, corner2))

                            self.board.exchange(*corner1, *corner2)
                            self.board.marks[corner1[0]][corner1[1]], self.board.marks[corner2[0]][corner2[1]] = \
                                self.board.marks[corner2[0]][corner2[1]], self.board.marks[corner1[0]][corner1[1]]

                elif type == EDGES:
                    # ... or by swapping edges ...
                    edge_idxs = self.cached_edge_idxs
                    random.shuffle(edge_idxs)
                    evals = [self.board.evaluate_piece(i, j) for i, j in edge_idxs]
                    evals, edge_idxs = zip(*sorted(zip(evals, edge_idxs)))
                    for idx1 in range(1, len(edge_idxs)):
                        for idx2 in range(idx1):
                            edge1 = edge_idxs[idx1]
                            edge2 = edge_idxs[idx2]

                            self.board.exchange(*edge1, *edge2)
                            self.board.marks[edge1[0]][edge1[1]], self.board.marks[edge2[0]][edge2[1]] = \
                                self.board.marks[edge2[0]][edge2[1]], self.board.marks[edge1[0]][edge1[1]]
                            self.board.fix_orientation()
                            after = self.board.evaluate()
                            if after > max_after:
                                max_after = after
                                max_indices = (edge1, edge2)
                                raise Exception()

                            if after == before:
                                same_indices.append((edge1, edge2))

                            self.board.exchange(*edge1, *edge2)
                            self.board.marks[edge1[0]][edge1[1]], self.board.marks[edge2[0]][edge2[1]] = \
                                self.board.marks[edge2[0]][edge2[1]], self.board.marks[edge1[0]][edge1[1]]

                elif type == INNER:
                    # ... or by swapping inner pieces
                    inner_idxs = self.cached_inner_idxs
                    random.shuffle(inner_idxs)

                    # pick only those that have score < 4
                    actual_inner_idxs = []
                    evals = []
                    for i, j in inner_idxs:
                        val = self.board.evaluate_piece(i, j)
                        if val <= 4:
                            actual_inner_idxs.append((i, j))
                            evals.append(val)

                    # evals = [self.board.evaluate_piece(i,j) for i,j in inner_idxs]
                    evals, actual_inner_idxs = zip(*sorted(zip(evals, actual_inner_idxs)))

                    for idx1 in range(1, len(actual_inner_idxs)):
                        inner1 = actual_inner_idxs[idx1]

                        for idx2 in range(idx1):
                            inner2 = actual_inner_idxs[idx2]

                            if evals[idx2] >= 4:
                                break

                            orig_dir1 = self.board.board[inner1[0]][inner1[1]].dir
                            orig_dir2 = self.board.board[inner2[0]][inner2[1]].dir
                            piece_score_before = self.board.evaluate_piece(*inner1) + self.board.evaluate_piece(*inner2)

                            # if these pieces are next to each other and they share a color now,
                            # then we have over counted it
                            if self.have_common_edge(inner1, inner2):
                                piece_score_before -= 1

                            self.board.exchange(*inner1, *inner2)
                            self.board.marks[inner1[0]][inner1[1]], self.board.marks[inner2[0]][inner2[1]] = \
                                self.board.marks[inner2[0]][inner2[1]], self.board.marks[inner1[0]][inner1[1]]
                            piece_score_best_after = piece_score_before

                            for dir1 in range(4):
                                for dir2 in range(4):
                                    self.board.board[inner1[0]][inner1[1]].dir = dir1
                                    self.board.board[inner2[0]][inner2[1]].dir = dir2
                                    piece_score_after = self.board.evaluate_piece(*inner1)
                                    piece_score_after += self.board.evaluate_piece(*inner2)

                                    # if these pieces are next to each other and they share a color now,
                                    # then we have over counted it
                                    if self.have_common_edge(inner1, inner2):
                                        piece_score_after -= 1

                                    # after = board.evaluate()
                                    # if after > max_after:
                                    #     max_after = after
                                    #     max_indices = (inner1, inner2)
                                    #     raise Exception()

                                    # if after == before:
                                    #     same_indices.append((inner1, inner2))

                                    if piece_score_after > piece_score_best_after:
                                        #print(
                                        #    f"Switching {inner1} <-> {inner2}, score {piece_score_before} to {piece_score_after}")
                                        piece_score_best_after = piece_score_after
                                        self.board.heuristic_orientation()
                                        raise Exception()

                                    if piece_score_after == piece_score_before:
                                        same_indices.append((inner1, inner2))

                            self.board.exchange(*inner1, *inner2)
                            self.board.marks[inner1[0]][inner1[1]], self.board.marks[inner2[0]][inner2[1]] = \
                                self.board.marks[inner2[0]][inner2[1]], self.board.marks[inner1[0]][inner1[1]]

                            self.board.board[inner1[0]][inner1[1]].dir = orig_dir1
                            self.board.board[inner2[0]][inner2[1]].dir = orig_dir2

        except:
            # we find something, cool...
            self.board.fix_orientation()
            return True

        # If we've got here, nothing more to do
        #print("Can't find anything else...")

        if same_indices:
            # crazy variant, lets shuffle all the indicies we can -> temporaririly can decrease the score though
            random.shuffle(same_indices)
            for same_pair in same_indices[:10]:
                #print(f"... exchanging pieces {same_pair[0]} with {same_pair[1]} with same result to give chance of swing into another possibilities")
                board.exchange(*same_pair[0], *same_pair[1])
                board.marks[same_pair[0][0]][same_pair[0][1]], board.marks[same_pair[1][0]][same_pair[1][1]] = \
                    board.marks[same_pair[1][0]][same_pair[1][1]], board.marks[same_pair[0][0]][same_pair[0][1]]
            board.fix_orientation()
            board.heuristic_orientation()
            return True

            # same_pair = same_indices[random.randint(0, len(same_indices) - 1)]
            # print(f"... exchanging pieces {same_pair[0]} with {same_pair[1]} with same result to give chance of swing into another possibilities")
            # board.exchange(*same_pair[0], *same_pair[1])
            # board.marks[same_pair[0][0]][same_pair[0][1]], board.marks[same_pair[1][0]][same_pair[1][1]] = \
            #     board.marks[same_pair[1][0]][same_pair[1][1]], board.marks[same_pair[0][0]][same_pair[0][1]]
            # board.fix_orientation()
            # board.heuristic_orientation()
            # return True
        else:
            #print("... and no pieces with same score!")
            return False

    def do_swap(self):

        if self.state == self.QUICK_SWAPPING:
            self.quick_swaps()
            score = self.board.evaluate()
            if score > self.max_score:
                #print(f"Best score improved to {score}")
                self.quick_swapping_counter = 3
                self.max_score = score
                self.board_backup = copy.deepcopy(self.board)
            else:
                self.quick_swapping_counter -= 1
                if self.quick_swapping_counter <= 0:
                    #print(f"QUICK_SWAPPING not successful, switching to RANDOM_SHUFFLING")
                    # quick swapping failing for too long, try shuffling now...
                    self.state = self.RANDOM_SHUFFLING
                    # reset some states stuff just in case
                    self.quick_swapping_counter = 3

        elif self.state == self.RANDOM_SHUFFLING:
            self.recovering_counter = 2 #time.time() + 60
            self.shuffle()
            self.score_before = self.board.evaluate()
            self.state = self.RANDOM_RECOVERING

        elif self.state == self.RANDOM_RECOVERING:
            # we have been shuffled, and trying to recover
            self.quick_swaps()
            score = self.board.evaluate()
            if score > self.max_score:
                # shuffle succeeded, try regular quick swapping again
                #print(f"RANDOM_RECOVERING successful, reached {score} > {self.max_score}, trying QUICK_SWAPPING again")
                self.state = self.QUICK_SWAPPING
            elif score > self.score_before:
                # we are improving at least, keep shuffling...
                #print(f"RANDOM_RECOVERING locally improving {self.score_before} -> {score}, we keep shuffling")
                self.recovering_counter = 2
            else:
                self.recovering_counter -= 1
                if self.recovering_counter >= 0:
                    #print(f"RANDOM_RECOVERING not successful, going back to RANDOM_SHUFFLING")
                    # we did not get back where we were originally, restore backup and try again
                    if score < self.max_score:
                        # restore backup, but only if we have not reached at least the same score
                        # otherwise we would keep the new one
                        self.board = self.board_backup
                    self.state = self.RANDOM_SHUFFLING
            self.score_before = score

    def enumerate_corners_swaps(self):
        for idx1 in range(1, len(self.cached_corners_idxs)):
            corner1 = self.cached_corners_idxs[idx1]
            for idx2 in range(idx1):
                corner2 = self.cached_corners_idxs[idx2]
                yield corner1, corner2

    def enumerate_edge_swaps(self):
        for idx1 in range(1, len(self.cached_edge_idxs)):
            edge1 = self.cached_edge_idxs[idx1]
            for idx2 in range(idx1):
                edge2 = self.cached_edge_idxs[idx2]
                yield edge1, edge2

    def enumerate_inner_swaps(self):
        for idx1 in range(1, len(self.cached_inner_idxs)):
            inner1 = self.cached_inner_idxs[idx1]
            for idx2 in range(idx1):
                inner2 = self.cached_inner_idxs[idx2]
                yield inner1, inner2

    def enumerate_all_swaps(self):
        for X in self.enumerate_corners_swaps():
            yield X
        for X in self.enumerate_edge_swaps():
            yield X
        for X in self.enumerate_inner_swaps():
            yield X

    def swap(self, id1, id2):
        self.board.exchange(*id1, *id2)
        self.board.marks[id1[0]][id1[1]], self.board.marks[id2[0]][id2[1]] = \
            self.board.marks[id2[0]][id2[1]], self.board.marks[id1[0]][id1[1]]

    def do_double_swap(self):
        before = self.board.evaluate()
        swaps = list(self.enumerate_all_swaps())
        try:
            for idx1, (swap1id1, swap1id2) in enumerate(swaps):
                self.swap(swap1id1, swap1id2)
                for idx2, swap2id1, swap2id2 in enumerate(swaps):
                    self.swap(swap2id1, swap2id2)
                    self.board.fix_orientation()
                    self.board.heuristic_orientation()

                    after = board.evaluate()
                    if after > before:
                        #print(f"double swap success {swap1id1}{swap1id2}{swap2id1}{swap2id2} score {before} -> {after}")
                        raise Exception()

                    # second swap back
                    self.swap(swap2id1, swap2id2)
                # first swap back
                self.swap(swap1id1, swap1id2)
        except:
            # we find something, cool...
            self.board.fix_orientation()
            self.board.heuristic_orientation()
            return True

        return False

    def do_triple_swap(self):
        before = self.board.evaluate()
        swaps = list(self.enumerate_all_swaps())
        try:
            for idx1, (swap1id1, swap1id2) in enumerate(swaps):
                self.swap(swap1id1, swap1id2)
                for idx2, swap2id1, swap2id2 in enumerate(swaps):
                    self.swap(swap2id1, swap2id2)
                    for idx3, swap3id1, swap3id2 in enumerate(swaps):
                        self.swap(swap3id1, swap3id2)
                        self.board.fix_orientation()
                        self.board.heuristic_orientation()

                        after = board.evaluate()
                        if after > before:
                            #print(f"triple swap success {swap1id1}{swap1id2}{swap2id1}{swap2id2}{swap3id1}{swap3id2} score {before} -> {after}")
                            raise Exception()

                        # third swap back
                        self.swap(swap3id1, swap3id2)
                    # second swap back
                    self.swap(swap2id1, swap2id2)
                # first swap back
                self.swap(swap1id1, swap1id2)
        except:
            # we find something, cool...
            self.board.fix_orientation()
            self.board.heuristic_orientation()
            return True

        return False


if __name__ == '__main__':

    parser = argparse.ArgumentParser()
    parser.add_argument('-conf', type=str, required=True, help='Definition file')
    parser.add_argument('-hints', type=str, required=False, default=None, help='Hint file')
    parser.add_argument('-load', type=str, required=False, default=None, help='Load from saved file')
    args = parser.parse_args()

    puzzle_def = PuzzleDefinition()
    puzzle_def.load(args.conf, args.hints)

    height = puzzle_def.height
    width = puzzle_def.width

    board = board.Board(puzzle_def)
    if args.load:
        board.load(args.load)
        # fill empty pieces
        for id in range(1, height*width+1):
            if id not in board.board_by_id:
                if board.puzzle_def.all[id].get_type() == TYPE_CORNER:
                    for i,j in board.enumerate_corners():
                        if not board.board[i][j]:
                            board.put_piece(i, j, board.puzzle_def.all[id], 0)
                            break
                    pass
                elif board.puzzle_def.all[id].get_type() == TYPE_EDGE:
                    for i,j in board.enumerate_edges():
                        if not board.board[i][j]:
                            board.put_piece(i, j, board.puzzle_def.all[id], 0)
                            break
                else: # INNER
                    for i,j in board.enumerate_inner():
                        if not board.board[i][j]:
                            board.put_piece(i, j, board.puzzle_def.all[id], 0)
                            break
        board.fix_orientation()
        board.evaluate()
    else:
        board.randomize()
        board.fix_orientation()
        board.heuristic_orientation()

    # mark the pieces
    for i in range(height):
        for j in range(width):
            if board.board[i][j]:
                board.marks[i][j] = board.board[i][j].piece_def.id

    swapper = Swapper(board)

    ui = ui.BoardUi(board)
    ui.init()
    ui.update()

    selected_from = None
    selected_to = None

    LEFT_BUTTON = 1
    RIGHT_BUTTON = 2


    best = board.evaluate()
    pygame.display.update()
    pygame.display.set_caption(f'SBest {best}/{board.max_score()} SCurr {best}/{board.max_score()}')
    double_swap_counter = 3
    shuffle_counter = 3
    shuffle_backup = None

    def check_best():
        curr = board.evaluate()

        global best
        global shuffle_counter
        global shuffle_backup
        #
        # if shuffle_counter <= 0 and curr == best:
        #     # trying to find sequence of ANY 2 swaps that potentially increase the overall score ...
        #     #swapper.do_double_swap()
        #     shuffle_backup = copy.deepcopy(board)
        #     swapper.shuffle()
        #     shuffle_counter = 3

        #if True: # DEBUG TEST ONLY
        #if double_swap_counter <= 0 and curr == best:
        # if False: # double swapping disabled for now - until not optimized
        #     print("DOUBLE SWAP! Start...")
        #     # trying to find sequence of ANY 2 swaps that potentially increase the overall score ...
        #     #swapper.do_double_swap()
        #     swapper.do_double_swap()
        #     print("DOUBLE SWAP! End...")
        #     double_swap_counter = 3

        ui.update()
        if best < curr:
            print(f"[UI] Best score improved to {curr}")
            best = curr

            if best >= board.max_score() - 30:
                board.save("save " + str(uuid.uuid4()) + "_" + str(best) + ".csv")
            # we fill in remaining pieces to have a slightly better score

            if best == board.max_score():
                pygame.display.set_caption(f'Puzzle SOLVED!')
                print("SOLVED!")
        # elif best == curr:
        #     double_swap_counter -= 1

        pygame.display.set_caption(f'SBest {best}/{board.max_score()} SCurr {curr}/{board.max_score()}')

    while True:
        if best < board.max_score():
            if swapper.do_swap():
                pass
            board = swapper.board
            ui.board = swapper.board
            check_best()

        for event in pygame.event.get():
            if event.type == pygame.locals.QUIT:
                pygame.quit()
                sys.exit()
            elif event.type == pygame.KEYDOWN:
                if event.key == pygame.K_i:
                    ui.marks_enabled = not ui.marks_enabled
                    ui.update()

        pygame.display.update()



