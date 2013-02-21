CC = gcc

TARGET = sample_client sample_server

CFLAGS += -g -I/opt/local/include
LDFLAGS += -g -lprotobuf-c -L/opt/local/lib

all:	$(TARGET)

sample_client: lsp_client.o udp.o lspmessage.pb-c.o

sample_server: lsp_server.o udp.o lspmessage.pb-c.o

%.o:	%.c
	$(CC) -c $(CFLAGS) $< -o $@

lsp_client.o: lsp_client.c 
	gcc -c lsp_client.c
	
lsp_server.o: lsp_server.c
	gcc -c lsp_server.c
	
udp.o: udp.c
	gcc -c udp.c

lspmessage.pb-c.o: lspmessage.pb-c.c
	gcc -c lspmessage.pb-c.c
	
clean:
	rm -f *.o 
	rm -f $(TARGET)
