n=0;

while test $n -lt 13; do
	i=0;
	while test $i -le $n; do
		echo -en "`./choose $n $i`\t";
		i=`expr $i + 1`
	done;
	echo;
	n=`expr $n + 1`
done;
