CC = g++

TARGET = sample_client sample_server server worker

CFLAGS += -lssl -lcrypto -g -I/opt/local/include
LDFLAGS += -g -lprotobuf-c -L/opt/local/lib

all:	$(TARGET)

sample_client: lsp_client.o udp.o lspmessage.pb-c.o -lpthread

sample_server: lsp_server.o udp.o lspmessage.pb-c.o -lpthread

server: lsp_server.o udp.o lspmessage.pb-c.o -lpthread

worker: lsp_client.o udp.o lspmessage.pb-c.o -lpthread -lssl

%.o:	%.c
	$(CC) -c $(CFLAGS) $< -o $@

lsp_client.o: lsp_client.c 
	$(CC) -c lsp_client.c
	
lsp_server.o: lsp_server.c
	$(CC) -c lsp_server.c
	
udp.o: udp.c
	$(CC) -c udp.c

lspmessage.pb-c.o: lspmessage.pb-c.c
	$(CC) -c lspmessage.pb-c.c
	
clean:
	rm -f *.o 
	rm -f $(TARGET)
