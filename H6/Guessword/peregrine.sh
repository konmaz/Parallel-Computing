#!/bin/bash
#SBATCH --time=00:12:00
#SBATCH --nodes=1
#SBATCH --ntasks-per-node=24
#SBATCH --job-name=ParallelComputing
#SBATCH --mem-per-cpu=250
#SBATCH --partition=short

echo "Starting Peregrine Job"
echo -n "Loading modules..."
module load foss/2021a
module load Julia/1.7.2-linux-x86_64
echo "Done"

echo -n "Compiling..."
make
echo "Done"

echo -n "Running..."
timeout -sHUP 10m mpiexec -n 24 ./guessword ./sample2-passwd.txt ./sample2-shadow.txt > ./peregrine.out
echo "Done"

echo -n "Checking output..."
julia evaluateUniquePasswords.jl 
echo "Done"