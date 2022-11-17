make -B 2> /dev/null

ARGELEM=1073741824

echo "Thread,Tempo Min,OP/s Max,Tempo Medio,OP/s Medio,Aceleracao" >> resultados.csv

for ARGTHREADS in 1 2 3 4 5 6 7 8
do

	./prefixSumPth "$ARGELEM" "$ARGTHREADS" > temp.txt

	TEMPOMED="$(cat temp.txt | cut -d "," -f 1)"
	OPSMED="$(cat temp.txt | cut -d "," -f 2)"
	TEMPOMIN=$TEMPOMED
	OPSMAX=$OPSMED

	for i in 2 3 4 5 6 7 8 9 10
	do
		./prefixSumPth "$ARGELEM" "$ARGTHREADS" > temp.txt

		TEMPO="$(cat temp.txt | cut -d "," -f 1)"
		OPS="$(cat temp.txt | cut -d "," -f 2)"

		TEMPOMED="$(echo ""$TEMPOMED" + "$TEMPO"" | bc)"
		OPSMED="$(echo ""$OPSMED" + "$OPS"" | bc)"

		if [ 1 -eq "$(echo "$TEMPOMIN > $TEMPO" | bc)" ];
		then
			TEMPOMIN=$TEMPO
		fi
		if [ 1 -eq "$(echo "$OPSMAX < $OPS" | bc)" ];
		then
			OPSMAX=$OPS
		fi

	done

	TEMPOMED="$(echo "scale=6;"$TEMPOMED" / 10" | bc)"
	OPSMED="$(echo "scale=6;"$OPSMED" / 10" | bc)"

	if [ $ARGTHREADS = 1 ]; then
		OPSANT=$OPSMED
	fi

	ACELERACAO="$(echo "scale=6;"$OPSMED" / "$OPSANT"" | bc)"

	echo "$ARGTHREADS,$TEMPOMIN,$OPSMAX,$TEMPOMED,$OPSMED,$ACELERACAO" >> resultados.csv
	OPSANT=$OPSMED

	echo "Rodou para "$ARGTHREADS" threads..."

done

rm temp.txt

gnuplot <<-EOFMarker
set title "Resultados"
set datafile separator ","		
set xlabel "Threads"
set ylabel "Tempo e OP/s"
set y2label "Aceleracao"
set term png
set output 'grafico.png'
set y2tics
plot "resultados.csv" using 1:3 title 'OP/s Maximo' with linespoints,\
"resultados.csv" using 1:5 title 'OP/s Medio' with linespoints,\
"resultados.csv" using 1:6 title 'Aceleracao' with linespoints axis x1y2,
EOFMarker

#caso queira o tempo no grafico:
#"resultados.csv" using 1:4 title 'Tempo Medio' with linespoints,\
#plot "resultados.csv" using 1:2 title 'Tempo Minimo' with linespoints,\
