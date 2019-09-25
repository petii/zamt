#! /usr/bin/python

import argparse

parser = argparse.ArgumentParser()
parser.add_argument('-o --output', type=str, default='indices.txt',
                    metavar='output file (default: indices.txt)', dest='outputFileName')
parser.add_argument('-s --samples', type=int, required=True,
                    metavar='sample size', dest='sampleSize')
parser.add_argument('-c --history', type=int, required=True,
                    metavar='number of rows', dest='history')

args = parser.parse_args()

indicesFile = open(args.outputFileName, 'w')

# parts = args.history
parts = args.sampleSize

for passCount in range(0, args.history * args.sampleSize // (2 * parts)):
    for vertex in range(0, 2 * (parts), 2):
        currentVertex = passCount * 2 * parts + vertex
        triangles = [
            currentVertex,
            currentVertex + 1,
            currentVertex + 3,

            currentVertex,
            currentVertex + 3,
            currentVertex + 2
        ]
        squareIndices = ' '.join([str(index) for index in triangles]) + '\n'
        indicesFile.write(squareIndices)
