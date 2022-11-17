make -B

ARGELEM=1000000000

for ARGTHREADS in 1 2 3 4 5 6 7 8
do
	echo "Testes com "$ARGELEM" elementos com "$ARGTHREADS" threads\nTempo em secs	OP/secs" >> resultados.txt

	for i in 1 2 3 4 5 6 7 8 9 10
	do
		./prefixSumPth "$ARGELEM" "$ARGTHREADS" >> resultados.txt
	done

	echo "" >> resultados.txt

	echo "rodou para "$ARGTHREADS" threads..."

done
