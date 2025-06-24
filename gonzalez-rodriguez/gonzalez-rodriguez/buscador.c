#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <time.h>
#include <errno.h>

#define MAX_DEPTO 64
#define LINE_BUFFER_SIZE 2048
#define TABLE_SIZE 10007
#define MAX_CAMPOS 32
#define BLOQUE_OFFSET 10000

typedef struct {
    int hashValue;
    char nombreDepartamento[MAX_DEPTO];
    int cantidadOffsets;
    long posicionLista;
} EntradaIndice;

unsigned int hash_func(const char* key) {
    unsigned int hash = 0;
    for (const char* p = key; *p; p++) {
        hash = (hash * 31) + (unsigned char)(*p);
    }
    return hash % TABLE_SIZE;
}

int buscar_entrada(const char* nombreDepartamento, EntradaIndice* salida) {
    FILE* f = fopen("/home/anna/so/practica/output/depto_index.bin", "rb");
    if (!f) {
        perror("Error abriendo depto_index.bin");
        return 0;
    }

    int hash = hash_func(nombreDepartamento);
    EntradaIndice entrada;

    while (fread(&entrada.hashValue, sizeof(int), 1, f) == 1) {
        if (fread(&entrada.nombreDepartamento, sizeof(char), MAX_DEPTO, f) != MAX_DEPTO) break;         
        if (fread(&entrada.cantidadOffsets, sizeof(int), 1, f) != 1) break;                              
        if (fread(&entrada.posicionLista, sizeof(long), 1, f) != 1) break;                               

        if (entrada.hashValue == hash && strcmp(entrada.nombreDepartamento, nombreDepartamento) == 0) {
            *salida = entrada;
            fclose(f);
            return 1;
        }
    }

    fclose(f);
    return 0;
}

int separar_campos(char* linea, char* campos[], int max_campos) {
    int i = 0;
    campos[i++] = strtok(linea, ",");
    while (i < max_campos && (campos[i] = strtok(NULL, ","))) {
        i++;
    }
    return i;
}

long obtener_memoria_en_kb() {
    FILE* f = fopen("/proc/self/status", "r");
    if (!f) return -1;

    char linea[256];
    while (fgets(linea, sizeof(linea), f)) {
        if (strncmp(linea, "VmRSS:", 6) == 0) {
            long kb = 0;
            if (sscanf(linea, "VmRSS: %ld", &kb) == 1) {
                fclose(f);
                return kb;
            }
        }
    }

    fclose(f);
    return -1;
}

int coinciden_criterios(char* linea, int edadBuscada, const char* sexoBuscado) {
    char* campos[MAX_CAMPOS];
    int n = separar_campos(linea, campos, MAX_CAMPOS);
    if (n < 10) return 0;

    int edad_actual = atoi(campos[7]);
    const char* sexo_actual = campos[9];

    if (edadBuscada != -1 && edad_actual != edadBuscada) return 0;
    if (strcmp(sexoBuscado, "*") != 0 && strcmp(sexo_actual, sexoBuscado) != 0) return 0;

    return 1;
}

void buscar_y_filtrar(const char* departamento, int edadBuscada, const char* sexoBuscado, int fd_salida) {
    struct timespec inicio, fin;
    clock_gettime(CLOCK_MONOTONIC, &inicio);

    FILE* f_csv = fopen("/home/anna/so/practica/dataset.csv", "r");
    if (!f_csv) {
        dprintf(fd_salida, "Error al abrir dataset.csv\n");
        return;
    }

    int coincidencias = 0;
    char linea[LINE_BUFFER_SIZE];
    char copia[LINE_BUFFER_SIZE];

    if (strcmp(departamento, "*") == 0) {
        if (!fgets(linea, sizeof(linea), f_csv)) {  
            fclose(f_csv);
            dprintf(fd_salida, "Error al leer cabecera.\n");
            return;
        }

        while (fgets(linea, sizeof(linea), f_csv)) {
            strcpy(copia, linea);
            if (coinciden_criterios(linea, edadBuscada, sexoBuscado)) {
                if (write(fd_salida, copia, strlen(copia)) == -1) {  
                    perror("Error escribiendo al FIFO");
                }
                coincidencias++;
            }
        }
    } else {
        EntradaIndice entrada;
        if (!buscar_entrada(departamento, &entrada)) {
            dprintf(fd_salida, "Departamento no encontrado en el índice.\n");
            fclose(f_csv);
            return;
        }

        FILE* f_offsets = fopen("/home/anna/so/practica/output/depto_lists.bin", "rb");
        if (!f_offsets) {
            dprintf(fd_salida, "No se pudieron leer los offsets.\n");
            fclose(f_csv);
            return;
        }

        fseek(f_offsets, entrada.posicionLista, SEEK_SET);
        long* buffer_offsets = malloc(sizeof(long) * BLOQUE_OFFSET);
        if (!buffer_offsets) {
            dprintf(fd_salida, "Error al asignar memoria.\n");
            fclose(f_csv);
            fclose(f_offsets);
            return;
        }

        int total = entrada.cantidadOffsets;
        int leidos = 0;

        while (leidos < total) {
            int cantidad = (total - leidos > BLOQUE_OFFSET) ? BLOQUE_OFFSET : (total - leidos);
            size_t leidos_efectivos = fread(buffer_offsets, sizeof(long), cantidad, f_offsets);
            if (leidos_efectivos == 0) break;  

            for (int i = 0; i < (int)leidos_efectivos; i++) {
                fseek(f_csv, buffer_offsets[i], SEEK_SET);
                if (fgets(linea, sizeof(linea), f_csv)) {
                    strcpy(copia, linea);
                    if (coinciden_criterios(linea, edadBuscada, sexoBuscado)) {
                        if (write(fd_salida, copia, strlen(copia)) == -1) {
                            perror("Error escribiendo al FIFO");
                        }
                        coincidencias++;
                    }
                }
            }

            leidos += leidos_efectivos;
        }

        free(buffer_offsets);
        fclose(f_offsets);

        if (coincidencias == 0) {
            dprintf(fd_salida, "NA\n");
        }
    }

    fclose(f_csv);

    clock_gettime(CLOCK_MONOTONIC, &fin);
    long segundos = fin.tv_sec - inicio.tv_sec;
    long nanosegundos = fin.tv_nsec - inicio.tv_nsec;
    double tiempo_total = segundos + nanosegundos / 1e9;
    long memoria_kb = obtener_memoria_en_kb();

    dprintf(fd_salida, "\nTiempo de búsqueda: %.6f segundos\n", tiempo_total);
    dprintf(fd_salida, "Total de coincidencias: %d\n", coincidencias);
    if (memoria_kb >= 0)
        dprintf(fd_salida, "Memoria utilizada: %ld KB\n", memoria_kb);
}

int main() {
    const char* entrada_fifo = "entrada_fifo";
    const char* salida_fifo = "salida_fifo";

    mkfifo(entrada_fifo, 0666);
    mkfifo(salida_fifo, 0666);

    printf("Buscador listo. Esperando criterios...\n");

    while (1) {
        int fd_entrada = open(entrada_fifo, O_RDONLY);
        if (fd_entrada < 0) {
            perror("Error al abrir entrada_fifo");
            exit(1);
        }

        char buffer[256];
        ssize_t n = read(fd_entrada, buffer, sizeof(buffer) - 1);
        close(fd_entrada);

        if (n <= 0) continue;
        buffer[n] = '\0';

        char departamento[MAX_DEPTO];
        int edad;
        char sexo[10];

        if (sscanf(buffer, "\"%63[^\"]\" %d %9s", departamento, &edad, sexo) != 3) {
            fprintf(stderr, "Error de formato en mensaje recibido: %s\n", buffer);
            continue;
        }

        int fd_salida = open(salida_fifo, O_WRONLY);
        if (fd_salida < 0) {
            perror("Error al abrir salida_fifo");
            continue;
        }

        buscar_y_filtrar(departamento, edad, sexo, fd_salida);
        close(fd_salida);
    }

    return 0;
}
