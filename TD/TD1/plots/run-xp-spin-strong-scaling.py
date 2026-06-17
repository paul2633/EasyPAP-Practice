#!/usr/bin/env python3
from expTools import *

options = {
    "--kernel": ["spin"],
    "--variant": ["omp_tiled"],
    "--iterations": [20],
    "--size": [4096],
    "--tile-size": [4, 8, 16, 32, 64, 128],
    "-of": ["spin-strong-scaling.csv"],
}

ompenv = {
    "OMP_NUM_THREADS": list(range(4, os.cpu_count() + 8 + 1, 4)),
    "OMP_SCHEDULE": ["static", "dynamic"],
}

# Lancement des experiences
execute("./run ", ompenv, options, nbruns=3, verbose=False, easyPath=".")

# Version séquentielle
options["--variant"] = ["seq"]
del options["--tile-size"]
ompenv = {"OMP_NUM_THREADS": [1]}
execute("./run", ompenv, options, nbruns=1, verbose=False, easyPath=".")

execute_simple("./plots/easyplot.py -if spin-strong-scaling.csv -v omp_tiled -- col=schedule")
