CFLAGS = -Wall -O2 -ansi
filter: filter.o
	gcc -o filter filter.o
filter.o: 
	gcc -c filter.c
runall:
	./filter input.ppm kernel output.ppm
clean:
	@rm -rf filter *.o output.ppm
