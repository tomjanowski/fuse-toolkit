fuse: main.o
	g++ $< -o $@ -lfuse
