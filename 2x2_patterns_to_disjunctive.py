import argparse

parsed = []

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

if __name__ == '__main__':

    parser = argparse.ArgumentParser()
    parser.add_argument('-i', type=str, required=True, help='Input file')
    parser.add_argument('-o', type=str, required=True, help='Output file')
    args = parser.parse_args()

    input_filename = args.i
    output_filename = args.o

    with open(input_filename, "r") as f:
        f.readline() # consume header
        for line in f.readlines():
            fields = (id, id1, id2, id3, id4, ord1, dir1, ord2, dir2, ord3, dir3, ord4, dir4) = line.strip().split(",")
            for i, field in enumerate(fields):
                fields[i] = int(field)
            parsed.append([*fields,-1,-1,-1,-1])

    # sort the parsed data lexiographically by ids
    parsed = list(sorted(parsed, key=lambda item: item[1:5]))
    # re-index the ID's
    for id, item in enumerate(parsed):
        item[0] = id

    iteration_counter = 0

    todo_next1 = 0
    todo_next2 = 0
    todo_next3 = 0
    todo_next4 = 0

    prev = 0

    for index, item in enumerate(parsed):
        if item[1] != parsed[prev][1]:
            for i1 in range(todo_next1, index):
                iteration_counter += 1
                parsed[i1][OFFSET_NEXT1] = index
            todo_next1 = index
            todo_next2 = index
            todo_next3 = index
            todo_next4 = index
        elif item[2] != parsed[prev][2]:
            for i2 in range(todo_next2, index):
                iteration_counter += 1
                parsed[i2][OFFSET_NEXT2] = index
            todo_next2 = index
            todo_next3 = index
            todo_next4 = index
        elif item[3] != parsed[prev][3]:
            for i3 in range(todo_next3, index):
                iteration_counter += 1
                parsed[i3][OFFSET_NEXT3] = index
            todo_next3 = index
            todo_next4 = index
        elif item[4] != parsed[prev][4]:
            for i4 in range(todo_next4, index):
                iteration_counter += 1
                parsed[i4][OFFSET_NEXT4] = index
            todo_next4 = index

        prev = index
        pass

    with open(output_filename, "w") as f:
        f.write("Index,ID1,ID2,ID3,ID4,Ordered1,Dir1,Ordered2,Dir2,Ordered3,Dir3,Ordered4,Dir4,Next1,Next2,Next3,Next4\n")
        for item in parsed:
            f.write(",".join(str(x) for x in item)+"\n")


    print(f"{iteration_counter} sequential incrementing iterations required")
