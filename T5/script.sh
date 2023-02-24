make -B 2> /dev/null

#para re-fazer os experimentos, excluímos os arquivos antigos
#caso não queira excluir, guarde os arquivos antigos em outro diretório
rm resultados-1.csv resultados-4l.csv resultados-4r.csv 2> /dev/null

NQ=128
NP=75000
ND=300
K=128

echo "1 Processo Local" >> resultados-1.csv
echo "4 Processos Locais" >> resultados-4l.csv
echo "4 Processos Remotos" >> resultados-4r.csv
echo "$NQ elementos em Q,	$NP elementos em P,	$ND dimensões, $K pontos mais próximos" >> resultados-1.csv
echo "$NQ elementos em Q,	$NP elementos em P,	$ND dimensões, $K pontos mais próximos" >> resultados-4l.csv
echo "$NQ elementos em Q,	$NP elementos em P,	$ND dimensões, $K pontos mais próximos" >> resultados-4r.csv

for vez in $(seq 1 10)
do
	echo ""
	echo "Rodando pela $vezª vez"
	echo "Rodando o np = 1"
	mpirun -np 1 --hostfile hostfile-local.txt ./trabalho5 "$NQ" "$NP" "$ND" "$K" | grep 'Tempo:' | awk '{printf "%s\n",$2}' >> resultados-1.csv
	echo "Rodando o np = 4 local"
	mpirun -np 4 --hostfile hostfile-local.txt ./trabalho5 "$NQ" "$NP" "$ND" "$K" | grep 'Tempo:' | awk '{printf "%s\n",$2}' >> resultados-4l.csv
	echo "Rodando o np = 4 remoto"
	mpirun -np 4 --hostfile hostfile-remot.txt ./trabalho5 "$NQ" "$NP" "$ND" "$K" | grep 'Tempo:' | awk '{printf "%s\n",$2}' >> resultados-4r.csv
done