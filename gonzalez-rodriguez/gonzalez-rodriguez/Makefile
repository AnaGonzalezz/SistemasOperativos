CC = gcc
CFLAGS = -Wall -O2
BINARIOS = hash_indexador buscador p1-dataProgram

all: $(BINARIOS)

hash_indexador: hash_indexador.c
	$(CC) $(CFLAGS) hash_indexador.c -o hash_indexador

buscador: buscador.c
	$(CC) $(CFLAGS) buscador.c -o buscador

p1-dataProgram: p1-dataProgram.c
	$(CC) $(CFLAGS) p1-dataProgram.c -o p1-dataProgram

run: all
	./hash_indexador
	./buscador &
	sleep 1
	./p1-dataProgram

clean:
	rm -f $(BINARIOS) entrada_fifo salida_fifo depto_index.bin depto_lists.bin
