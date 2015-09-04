#!/bin/bash
#usage ./cpremote {filename} {target directory}
scp $PWD/$1 czeng@aludra.usc.edu:/home/scf-04/czeng/nachos-csci402/code/$2/$1
