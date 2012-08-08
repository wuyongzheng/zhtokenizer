zhtokenizer: zhtokenizer.c utf8util.c htfunc.c generated_ht.o
	gcc -g -O -Wall -o zhtokenizer zhtokenizer.c utf8util.c htfunc.c generated_ht.o

httest: httest.c utf8util.c htfunc.c generated_ht.o
	gcc -g -O -Wall -o httest httest.c utf8util.c htfunc.c generated_ht.o

generated_ht.c: buildht words.weight.txt
	./buildht generated_ht.h generated_ht.c < words.weight.txt

buildht: buildht.c utf8util.c
	gcc -g -O -Wall -o buildht buildht.c utf8util.c
