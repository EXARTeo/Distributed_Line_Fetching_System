CFLAGS = -Wall -IIncludes  # Flags for warnings and include directory
LIBS = -lpthread -lrt      # Link the pthread and rt libraries

# Targets
all: dispatcher server client

# Compile dispatcher.c
dispatcher: Modules/dispatcher.c
	gcc $(CFLAGS) -o dispatcher Modules/dispatcher.c $(LIBS)

# Compile server.c
server: Modules/server.c
	gcc $(CFLAGS) -o server Modules/server.c $(LIBS)

# Compile client.c
client: Modules/client.c
	gcc $(CFLAGS) -o client Modules/client.c $(LIBS)

# Clean up compiled files
clean:
	rm -f dispatcher server client
