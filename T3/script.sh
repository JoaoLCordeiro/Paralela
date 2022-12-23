make -B 2> /dev/null

#para re-fazer os experimentos, excluímos os arquivos antigos
#caso não queira excluir, guarde os arquivos antigos em outro diretório
rm resultados-b.csv resultados-nb.csv 2> /dev/null

##numero de elementos
NMSG=262144

# para bloqueantes
for ARGTAM in 8 1024 4096 16384
do
	echo "Mensagens de tamanho $ARGTAM, bloqueantes" >> resultados-b.csv
	for vez in $(seq 1 10)
	do
		mpirun -np 2 ./trabalho3 "$NMSG" "$ARGTAM" 2 -b | grep 'Tempo:' | awk '{printf "%s\n",$2}' >> resultados-b.csv
	done
done

#para nao bloqueantes
for ARGTAM in 8 1024 4096 16384
do
	echo "Mensagens de tamanho $ARGTAM, nao-bloqueantes" >> resultados-nb.csv
	for vez in $(seq 1 10)
	do
		mpirun -np 2 ./trabalho3 "$NMSG" "$ARGTAM" 2 -nb | grep 'Tempo:' | awk '{printf "%s\n",$2}' >> resultados-nb.csv
	done
done
