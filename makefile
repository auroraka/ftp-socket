all:
	g++ server.cpp -o server
	g++ client.cpp -o client
server:
	g++ server.cpp -o server
	./server
clean:
	rm server client
