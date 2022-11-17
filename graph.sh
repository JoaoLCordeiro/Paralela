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