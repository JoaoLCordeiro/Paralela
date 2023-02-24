make -B 2> /dev/null

#para re-fazer os experimentos, excluímos os arquivos antigos
#caso não queira excluir, guarde os arquivos antigos em outro diretório
rm resultados-a.csv resultados-b.csv resultados-c.csv resultados-d.csv 2> /dev/null

#numero de elementos
NMSG=5000

# para bloqueantes
for ARGTAM in 8 1024 4096 16384
do
	echo "Mensagens de tamanho $ARGTAM" >> resultados-a.csv
	echo "Mensagens de tamanho $ARGTAM" >> resultados-b.csv
	echo "Mensagens de tamanho $ARGTAM" >> resultados-c.csv
	echo "Mensagens de tamanho $ARGTAM" >> resultados-d.csv
	#echo "NMSG = $NMSG"
	for vez in $(seq 1 10)
	do
		mpirun -np 2 --hostfile hostfile.txt ./trabalho4-a "$NMSG" "$ARGTAM" -r 0 | grep 'Tempo:' | awk '{printf "%s	%s\n",$2, $4}' >> resultados-a.csv
		mpirun -np 8 --hostfile hostfile.txt ./trabalho4-b "$NMSG" "$ARGTAM" -r 0 | grep 'Tempo:' | awk '{printf "%s	%s\n",$2, $4}' >> resultados-b.csv
		mpirun -np 8 --hostfile hostfile.txt ./trabalho4-c "$NMSG" "$ARGTAM" -r 0 | grep 'Tempo:' | awk '{printf "%s	%s\n",$2, $4}' >> resultados-c.csv
		mpirun -np 8 --hostfile hostfile.txt ./trabalho4-d "$NMSG" "$ARGTAM" -r 0 | grep 'Tempo:' | awk '{printf "%s	%s\n",$2, $4}' >> resultados-d.csv
	done
	NMSG=$(expr $NMSG / 2)
done
