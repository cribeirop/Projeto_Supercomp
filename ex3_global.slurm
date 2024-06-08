
#!/bin/bash
#SBATCH --job-name=ex3_global
#SBATCH --output=output_%j.txt
#SBATCH --error=errors_%j.txt
#SBATCH --partition=espec
#SBATCH --nodes=2
#SBATCH --ntasks-per-node=8
#SBATCH --cpus-per-task=1
#SBATCH --mem-per-cpu=500M
#SBATCH --time=0:01:00

num_proc=$((SLURM_NTASKS_PER_NODE * SLURM_NNODES))

mpirun -np $num_proc ./ex3_global
