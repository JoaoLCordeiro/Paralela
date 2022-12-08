make -B 2> /dev/null

#para re-fazer os experimentos, excluímos os arquivos antigos
#caso não queira excluir, guarde os arquivos antigos em outro diretório
rm resultados.csv todos-tempo.csv todos-ops.csv 2> /dev/null

#numero de elementos
ARGELEM=1073741824

echo "Thread,Tempo Min,OP/s Max,Tempo Medio,OP/s Medio,Aceleracao" >> resultados.csv

for ARGTHREADS in 1 2 3 4 5 6 7 8
do

	./prefixSumPth "$ARGELEM" "$ARGTHREADS" > temp.txt

	TEMPOMED="$(cat temp.txt | cut -d "," -f 1)"
	OPSMED="$(cat temp.txt | cut -d "," -f 2)"
	TEMPOMIN=$TEMPOMED
	OPSMAX=$OPSMED

	TODOSTEMPO=$ARGTHREADS","$TEMPOMED
	TODOSOPS=$ARGTHREADS","$OPSMED

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

		TODOSTEMPO=$TODOSTEMPO","$TEMPO
		TODOSOPS=$TODOSOPS","$OPS

	done

	TEMPOMED="$(echo "scale=6;"$TEMPOMED" / 10" | bc)"
	OPSMED="$(echo "scale=6;"$OPSMED" / 10" | bc)"

	if [ $ARGTHREADS = 1 ]; then
		OPSANT=$OPSMED
	fi

	ACELERACAO="$(echo "scale=6;"$OPSMED" / "$OPSANT"" | bc)"

	echo "$ARGTHREADS,$TEMPOMIN,$OPSMAX,$TEMPOMED,$OPSMED,$ACELERACAO" >> resultados.csv
	OPSANT=$OPSMED

	echo "$TODOSTEMPO" >> todos-tempo.csv
	echo "$TODOSOPS" >> todos-ops.csv

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
