CFLAGS = -Wall -g

all: client

client: client.cpp helpers.o nlohmann/json.hpp
	g++ $(CFLAGS) -o client client.cpp helpers.o

helpers.o: helpers.c helpers.h
	gcc -g -c helpers.c

run: client
	./client

clean:
	rm -f client helpers.o