all: server client
server: server.cpp SBCP.h
	g++ -o server server.cpp 

client: client.cpp SBCP.h
	g++ -o client client.cpp
	

clean:
	rm -rf *.o
