import numpy as np 
from PIL import Image
import struct
import sys

if len(sys.argv) < 2:
    print("usage: python png.py [grey|rgb]")
    quit()

if len(sys.argv) >= 3:
    inputfile = sys.argv[2]
else:
    inputfile = "raw/sample1.raw"

if len(sys.argv) >= 4:
    outputfile = sys.argv[3]
else:
    outputfile = "output.png"

rawData = open(inputfile, "rb").read()

matrix = [[[0 for i in range(3)] for i in range(256)] for i in range(256)]
for i in range(256):
    for j in range(256):
        matrix[i][j][0] = rawData[0+i*256+j]
        if sys.argv[1] == "grey":
            matrix[i][j][1] = matrix[i][j][0]
            matrix[i][j][2] = matrix[i][j][0]
        elif sys.argv[1] == "rgb":
            matrix[i][j][1] = rawData[256*256+i*256+j]
            matrix[i][j][2] = rawData[256*256*2+i*256+j]
        else:
            print("usage: python png.py [grey|rgb]")
            quit()

newimage = Image.new('RGB', (len(matrix[0]), len(matrix)))
newimage.putdata([tuple(p) for row in matrix for p in row])
newimage.save(outputfile)
