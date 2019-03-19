.PHONY: clean build debug

build:
# -g command allows debugging using command gdb main
	g++ -g main.cpp -o main.o `pkg-config --cflags --libs gtk+-3.0`
	# g++ -g main.cpp -o main.o

clean:
	rm *.o

debug:
	gdb main.o
