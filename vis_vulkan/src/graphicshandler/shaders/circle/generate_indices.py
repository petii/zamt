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

parts = args.history
for vertex in range(1, parts+1):
    triangles = [
       0, vertex, vertex+1
    ]        
    squareIndices = ' '.join([str(index) for index in triangles]) + '\n'
    indicesFile.write(squareIndices)

for passCount in range(0, args.history * args.sampleSize // parts):
    for vertex in range(1, parts+1):
        currentVertex = passCount * parts + vertex
        triangles = [
            currentVertex,
            currentVertex + parts,
            currentVertex + parts + 1,

            currentVertex,
            currentVertex + 1,
            currentVertex + 1 + parts
        ]
        squareIndices = ' '.join([str(index) for index in triangles]) + '\n'
        indicesFile.write(squareIndices)
