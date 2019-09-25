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

for row in range(0, args.history):
    for vertex in range(0, args.sampleSize):
        currentVertex = row * args.sampleSize + vertex
        indicesFile.write(str(currentVertex))
        indicesFile.write('\n')
