efc:	efc.o		
	cc efc.o -o efc	-lpthread
efc.o:	efc.c	
	cc -c efc.c	-std=gnu99
clean:
	rm efc.o
test: 
	chmod +x run.sh
	./run.sh