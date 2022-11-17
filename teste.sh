#make -B

ARGELEM=1000000

for ARGTHREADS in 1 #2 3 4 5 6 7 8
do
	echo "Testes com "$ARGELEM" elementos com "$ARGTHREADS" threads\nTempo em secs	OP/secs" >> resultados.txt

	./prefixSumPth "$ARGELEM" "$ARGTHREADS" > temp.txt

	TEMPOMED="$(cat temp.txt | cut -d "," -f 1)"
	OPSMED="$(cat temp.txt | cut -d "," -f 2)"
	TEMPOMIN=TEMPOMED
	OPSMIN=OPSMED

	for i in 2 3 4 5 6 7 8 9 10
	do
		./prefixSumPth "$ARGELEM" "$ARGTHREADS" > temp.txt
		cat temp.txt >> resultados.txt

		TEMPO="$(cat temp.txt | cut -d "," -f 1)"
		OPS="$(cat temp.txt | cut -d "," -f 2)"

		TEMPOMED="$(echo ""$TEMPOMED" + "$TEMPO"" | bc)"
		OPSMED="$(echo ""$OPSMED" + "$OPS"" | bc)"

		if [ $TEMPOMIN > $TEMPO ];
		then
			TEMPOMIN=TEMPO
		fi
		if [ $OPSMIN > $OPS ];
		then
			OPSMIN=OPS
		fi

	done

	TEMPOMED="$(echo ""$TEMPOMED" / 10" | bc)"
	OPSMED="$(echo ""$OPSMED" / 10" | bc)"

	echo "Minimos:" >> resultados.txt
	echo "$TEMPOMIN	$OPSMIN" >> resultados.txt
	echo "Media:" >> resultados.txt
	echo "$TEMPOMED $OPSMED" >> resultados.txt

	#echo "rodou para "$ARGTHREADS" threads..."

done
