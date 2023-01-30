make -B 2> /dev/null

#para re-fazer os experimentos, excluímos os arquivos antigos
#caso não queira excluir, guarde os arquivos antigos em outro diretório
rm resultados-b.csv resultados-nb.csv 2> /dev/null

##numero de elementos
NMSG=20000

# para bloqueantes
for ARGTAM in 8 1024 4096 16384
do
	echo "Mensagens de tamanho $ARGTAM" >> resultados.csv
	for vez in $(seq 1 10)
	do
		mpirun -np 8 ./trabalho4 --hostfile hostfile2.txt "$NMSG" "$ARGTAM" -r 0 | grep 'Tempo:' | awk '{printf "%s\n",$2,$4}' >> resultados.csv
	done
done
