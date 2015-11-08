for x in (seq $argv[1]);
	 set times  (echo "10 ^ $x" | bc);
	 ./a.out $times | datamash mean 2
end

