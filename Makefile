test: test.c
	gcc test.c -Wall -O2 -march=native -o test -lxxhash
