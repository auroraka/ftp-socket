clear:
	rm server client
all:
	g++ server.cpp -o server
	g++ client.cpp -o client
