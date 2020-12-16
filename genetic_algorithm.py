import random
import sys
import argparse
from core.defs import PuzzleDefinition, PieceRef
from ui.ui import BoardUi
from core.board import Board
import pygame.locals

from deap import algorithms
from deap import base
from deap import creator
from deap import tools




def main():
    creator.create("FitnessMax", base.Fitness, weights=(1.0, ))
    creator.create("Individual", list, fitness=creator.FitnessMax)

    toolbox = base.Toolbox()

    parser = argparse.ArgumentParser()
    parser.add_argument('-conf', type=str, required=True, help='Definition file')
    parser.add_argument('-hints', type=str, required=False, default=None, help='Hint file')
    args = parser.parse_args()

    puzzle_def = PuzzleDefinition()
    puzzle_def.load(args.conf, args.hints)

    board = Board(puzzle_def)

    toolbox.register("corners_perm", random.sample, range(4), 4)
    toolbox.register("edges_perm", random.sample, range(len(puzzle_def.edges)), len(puzzle_def.edges))
    toolbox.register("inner_perm", random.sample, range(len(puzzle_def.inner)), len(puzzle_def.inner))

    toolbox.register("individual", tools.initCycle, creator.Individual,
                     (toolbox.corners_perm, toolbox.edges_perm, toolbox.inner_perm), n=1)

    toolbox.register("population", tools.initRepeat, list, toolbox.individual)

    def custom_mate(ind1, ind2):
        i = random.randint(1, 3)
        if i == 1:
            corners = tools.cxPartialyMatched(ind1[0], ind2[0])
        elif i == 2:
            edges = tools.cxPartialyMatched(ind1[1], ind2[1])
        elif i == 3:
            inners = tools.cxPartialyMatched(ind1[2], ind2[2])

        return ind1, ind2

    toolbox.register("mate", custom_mate)

    def custom_mutate(ind, indpb):
        i = random.randint(1,3)
        if i == 1:
            tools.mutShuffleIndexes(ind[0], indpb)
        elif i == 2:
            tools.mutShuffleIndexes(ind[1], indpb)
        elif i == 3:
            tools.mutShuffleIndexes(ind[2], indpb)
        return ind,

    toolbox.register("mutate", custom_mutate, indpb=0.1)
    toolbox.register("select", tools.selTournament, tournsize=3)

    def evaluate(individual):
        corners_perm = individual[0]
        edges_perm = individual[1]
        inner_perm = individual[2]

        idx = 0
        for i, j in board.enumerate_corners():
            board.board[i][j] = PieceRef(puzzle_def.corners[corners_perm[idx]], 0, i, j)
            idx += 1

        idx = 0
        for i, j in board.enumerate_edges():
            board.board[i][j] = PieceRef(puzzle_def.edges[edges_perm[idx]], 0, i, j)
            idx += 1

        idx = 0
        for i, j in board.enumerate_inner():
            board.board[i][j] = PieceRef(puzzle_def.inner[inner_perm[idx]], 0, i, j)
            idx += 1

        board.fix_orientation()
        board.heuristic_orientation()

        return board.evaluate(),

    toolbox.register("evaluate", evaluate)

    NGEN=40

    pop = toolbox.population(n=200)

    # Evaluate the entire population
    hof = tools.HallOfFame(1)

    ui = BoardUi(board)
    ui.init()

    gen = 0
    while True:
        offspring = algorithms.varAnd(pop, toolbox, cxpb=0.5, mutpb=0.1)
        fits = toolbox.map(toolbox.evaluate, offspring)
        for fit, ind in zip(fits, offspring):
            ind.fitness.values = fit
        hof.update(pop)
        pop = toolbox.select(offspring, k=len(pop))

        # draw best
        score = evaluate(hof[0])
        pygame.display.set_caption(f'Puzzle (#{gen}, score {score[0]})')
        ui.update()
        for event in pygame.event.get():
            if event.type == pygame.locals.QUIT:
                pygame.quit()
                sys.exit()


        pygame.display.update()

        gen += 1


    top10 = tools.selBest(pop, k=10)

main()


