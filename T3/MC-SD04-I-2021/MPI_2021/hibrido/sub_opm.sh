#!/bin/bash
#SBATCH --nodes=2                      #Numero de Nós
#SBATCH --ntasks-per-node=2            #Numero de tarefas por Nó
#SBATCH --ntasks=4                     #Numero total de tarefas MPI
#SBATCH --cpus-per-task=12             #Numero de threads por tarefa MPI
#SBATCH -p cpu_dev                 #Fila (partition) a ser utilizada
#SBATCH -J sdumont-mpi                 #Nome job
#SBATCH --exclusive                    #Utilização exclusiva dos nós durante a execução do job

###############################################################
#   Para a submissão na Sdumont:                              #
#   sbatch sub.sh EXECUTÁVEL N ENERGY NITERS PX PY            #
#                                                             #
#   OBS:                                                      #
#       1. PX * PY deve ser igual ao numero de processos MPI  #
#       2. N % PY deve ser igual a 0                          #   
#       3. N % PX deve ser igual a 0                          #
#-------------------------------------------------------------#
#   Para a visualização dos Jobs:                             #
#   squeue -u $USER                                           #
#   ou                                                        #
#   watch squeue -u $USER (modo de visão                      #
#   tela cheia, para sair CTRL+C)                             #
#-------------------------------------------------------------#
#   Para cancelar o Job:                                      #
#   scancel NUMERO_DO_JOB                                     #
###############################################################


source /scratch/app/modulos/intel-psxe-2017.1.043.sh
export I_MPI_PMI_LIBRARY=/usr/lib64/libpmi.so
export OMP_NUM_THREADS=$SLURM_CPUS_PER_TASK

EXEC=${1}           
n=${2}             # nxn grid 
energy=${3}        # energy to be injected per iteration
niters=${4}        # number of iterations
px=${5}            # 1st dim processes
py=${6}            # 2nd dim processes

srun -n $SLURM_NTASKS -c $SLURM_CPUS_PER_TASK $EXEC $n $energy $niters $px $py




