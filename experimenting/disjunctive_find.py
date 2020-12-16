
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

class DisjunctiveSetsEnumerator:

    def __init__(self, filename):
        self.iteration_counter = 0
        self.parsed = []

        with open(filename, "r") as f:
            f.readline()  # consume header
            for line in f.readlines():
                fields = line.strip().split(",")
                for i, field in enumerate(fields):
                    fields[i] = int(field)
                self.parsed.append(fields)

    def get_next_id(self, field, at=-1):
        if at >= OFFSET_NEXT4 or at == -1 and field[OFFSET_NEXT4] != -1:
            return field[OFFSET_NEXT4]
        if at >= OFFSET_NEXT3 or at == -1 and field[OFFSET_NEXT3] != -1:
            return field[OFFSET_NEXT3]
        if at >= OFFSET_NEXT2 or at == -1 and field[OFFSET_NEXT2] != -1:
            return field[OFFSET_NEXT2]
        if at >= OFFSET_NEXT1 or at == -1 and field[OFFSET_NEXT1] != -1:
            return field[OFFSET_NEXT1]
        return -1

    def get_next_disjunctive_id(self, curr, prevs=None):
        idx = curr + 1
        if idx >= len(self.parsed):
            return -1

        while idx != -1:
            self.iteration_counter += 1
            record = self.parsed[idx]
            skip = None
            for level, id in enumerate(record[OFFSET_ID1:OFFSET_ID4 + 1]):
                for prev_id in prevs:
                    if prev_id is not None and id in self.parsed[prev_id][OFFSET_ID1:OFFSET_ID4 + 1]:
                        skip = level
                        break
                if skip is not None:
                    break

            if skip is not None:
                idx = self.get_next_id(record, skip + OFFSET_NEXT1)
                while idx == -1 and skip > 0:
                    skip -= 1
                    idx = self.get_next_id(record, skip + OFFSET_NEXT1)

            else:
                break
        return idx

    def enumerate(self, k, mins, maxs, start_at=(0, 0)):
        # test to find all disjunctive sequences of 4 items
        p_idx = k*[0]
        self.iteration_counter = 0
        pointer_max = 0

        pointer = start_at[0]
        p_idx[pointer] = start_at[1]

        current = k*[None]
        while True:
            #print(f"debug: {current}")
            if p_idx[pointer] == -1:
                if pointer == 0:
                    break
                else:
                    current[pointer - 1] = None
                    if pointer > 1:
                        p_idx[pointer - 1] = self.get_next_disjunctive_id(p_idx[pointer - 1], current)
                    else:
                        p_idx[0] += 1
                        if p_idx[0] == len(self.parsed):
                            break
                    pointer -= 1
            else:
                current[pointer] = p_idx[pointer]

                # special mechanism to apply min/max restrictions on the first (smallest) id
                # thus forcing first 4 to be corners, then edges, and then inners
                smallest_id = self.parsed[current[pointer]][1]
                #if smallest_id > maxs[pointer]:
                if smallest_id > maxs[pointer]:
                    current[pointer] = None
                    pointer -= 1
                    current[pointer] = None
                    p_idx[pointer] = self.get_next_disjunctive_id(p_idx[pointer], current)
                    continue
                elif smallest_id < mins[pointer]:
                    current[pointer] = None
                    # a short cut, jump to next one with smallest id of the min...
                    while self.parsed[p_idx[pointer]][1] < mins[pointer] and p_idx[pointer]:
                        p_idx[pointer] = self.get_next_id(self.parsed[p_idx[pointer]], OFFSET_NEXT1)
                    #p_idx[pointer] = self.get_next_disjunctive_id(p_idx[pointer] - 1, current)
                    current[pointer] = p_idx[pointer]


                pointer += 1
                if pointer > pointer_max:
                    pointer_max = pointer
                    print(f"reached {pointer_max}")

                if pointer == k:
                    yield current
                    pointer -= 1
                    current[pointer] = None
                    p_idx[pointer] = self.get_next_disjunctive_id(p_idx[pointer], current)
                else:
                    p_idx[pointer] = self.get_next_disjunctive_id(p_idx[pointer - 1], current)

        print(f"{self.iteration_counter} sequential incrementing iterations required")


import argparse
from core import board
from core.defs import PuzzleDefinition

if __name__ == '__main__':

    parser = argparse.ArgumentParser()
    parser.add_argument('-conf', type=str, required=True, help='Definition file')
    parser.add_argument('-hints', type=str, required=False, default=None, help='Hint file')
    parser.add_argument('-i', type=str, required=True, help='Input patterns file')
    args = parser.parse_args()

    puzzle_def = PuzzleDefinition()
    puzzle_def.load(args.conf, args.hints)

    board = board.Board(puzzle_def)

    # todo generate compound colors for each of the 2x2 piece

    print("Loading 2x2 patterns...")
    enumerator = DisjunctiveSetsEnumerator(args.i)

    # debug
    # counts = dict()
    # for item in enumerator.parsed:
    #     for id1 in item[1:5]:
    #         for id2 in item[1:5]:
    #             if id1 != id2:
    #                 if (id1, id2) not in counts:
    #                     counts[id1, id2] = 0
    #                 counts[id1, id2] += 1
    #                 if (id2, id1) not in counts:
    #                     counts[id2, id1] = 0
    #                 counts[id2, id1] += 1


    print("Enmuerating the set combinations")
    corners_count = 4

    reduced_height = puzzle_def.height//2
    reduced_width = puzzle_def.width//2
    reduced_edges_count = 2*(reduced_height+reduced_width-4)

    all_count = puzzle_def.height*puzzle_def.width
    edges_count = 2 * (puzzle_def.height + puzzle_def.width - 4)
    reduced_all_count = reduced_height*reduced_width
    min_ids = reduced_all_count*[0]
    max_ids = reduced_all_count*[0]
    for i in range(corners_count):
        min_ids[i] = 1
        max_ids[i] = 4
    for i in range(corners_count, corners_count+reduced_edges_count):
        min_ids[i] = 2*i-3
        max_ids[i] = corners_count + edges_count
    for i in range(corners_count + reduced_edges_count, reduced_all_count):
        k = 4+edges_count+1-4*(4+2*(reduced_height+reduced_width-4))
        min_ids[i] = 4*i+k
        #min_ids[i] = max(4*i-51, corners_count + edges_count + 1)
        max_ids[i] = all_count

    # for x in enumerator.enumerate(reduced_height * reduced_width, min_ids, max_ids, start_at=[28,74315]):
    #     # TODO check that piece colors are paired correctly to avoid obvious non-solutions
    #     print(x)
    #     pass

    for x in enumerator.enumerate(reduced_height * reduced_width, min_ids, max_ids):
        # TODO check that piece colors are paired correctly to avoid obvious non-solutions
        print(x)
        pass

