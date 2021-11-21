       
.PHONY : all clean cproxy sproxy

all : cproxy sproxy

cproxy :
	gcc cproxy.c -o cproxy

sproxy :
	gcc sproxy.c -o sproxy

clean :
	/bin/rm -f *.o *.gcov *.gcno *gcda cproxy sproxy
