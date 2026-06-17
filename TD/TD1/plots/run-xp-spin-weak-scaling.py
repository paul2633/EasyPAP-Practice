#!/usr/bin/env python3
from expTools import *
from math import sqrt

options = {
    "--kernel": ["spin"],
    "--variant": ["omp_tiled"],
    "--iterations": [50],
    "--tile-size": [64],
    "-of": ["spin-weak-scaling.csv"]
}

ompenv = {
    "OMP_SCHEDULE": ["static", "dynamic"],
}

def weak_scaling(num_threads):
    ompenv["OMP_NUM_THREADS"] = [num_threads]
    options["--size"] = [2048 * sqrt(num_threads)]
    execute("./run ", ompenv, options, nbruns=3, verbose=False, easyPath=".")

weak_scaling(1)
weak_scaling(4)
weak_scaling(16)

execute_simple("./plots/easyplot.py -if spin-weak-scaling.csv -x size -y time -- col=schedule")
