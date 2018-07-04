TARGET=server client

all: $(TARGET)

%.o: %.c
	gcc  $< -c 

server: server.o
	gcc -pthread server.o -o $@
client: client.o
	gcc -pthread client.o -o $@ 

debug:
	gcc -pthread -D DEBUG server.c -o serverDebug
	gcc -pthread -D DEBUG client.c -o clientDebug

clean:
	rm *.o
	rm server client

clean_debug:
	rm *Debug


