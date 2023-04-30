build: 
	gcc -o ex1 main.c trio.c spend_time.c -lm

clean: 
	@rm -rf ex1