CC= g++
CFLAGS= -lrt

all: rexec utility.o server.o client.o server client clean

# Start creating the objects
utility.o: Utility.cpp Utility.h
	$(CC) -c Utility.cpp $(CFLAGS)

server.o: Server.cpp Utility.h
	$(CC) -c Server.cpp $(CFLAGS)

client.o: Client.cpp Utility.h
	$(CC) -c Client.cpp $(CFLAGS) 
# End creating the objects

# Start creating the executables
server:
	$(CC) Utility.o Server.o $(CFLAGS) -o server

client:
	$(CC) Utility.o Client.o $(CFLAGS) -o client
# End creating the executables

# Delete all objects files
clean:
	rm -rf *.o

# Delete all executables
rexec:
	find . -perm +100 -type f -delete
