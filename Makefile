all:
	gcc raycast.c -o raycast
clean:
	rm -f raycast raycast.o
run: ./raycast
	./raycast