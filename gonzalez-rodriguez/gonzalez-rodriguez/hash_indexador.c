#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LINE_BUFFER_SIZE 2048
#define MAX_DEPARTAMENTO 64
#define TABLE_SIZE 10007 // número primo grande para menos colisiones

typedef struct OffsetNode {
long offset;
struct OffsetNode* next;
} OffsetNode;

typedef struct HashEntry {
char nombreDepartamento[MAX_DEPARTAMENTO];
OffsetNode* offsets;
struct HashEntry* next;
} HashEntry;

HashEntry* hash_table[TABLE_SIZE];

// Función hash simple: suma de caracteres
unsigned int hash_func(const char* key) {
unsigned int hash = 0;
for (const char* p = key; *p; p++) {
hash = (hash * 31) + (unsigned char)(*p);
}
return hash % TABLE_SIZE;
}

// Agrega un nuevo offset a la tabla hash
void insertar(const char* nombreDepartamento, long offset) {
unsigned int index = hash_func(nombreDepartamento);
HashEntry* actual = hash_table[index];

// Buscar si ya existe el nombreDepartamento en este bucket
while (actual) {
    if (strcmp(actual->nombreDepartamento, nombreDepartamento) == 0) {
        break;
    }
    actual = actual->next;
}

// Si no lo encontramos, crear nueva entrada
if (!actual) {
    actual = (HashEntry*)malloc(sizeof(HashEntry));
    strncpy(actual->nombreDepartamento, nombreDepartamento, MAX_DEPARTAMENTO);
    actual->offsets = NULL;
    actual->next = hash_table[index];
    hash_table[index] = actual;
}

// Crear nuevo nodo de offset
OffsetNode* nuevo = (OffsetNode*)malloc(sizeof(OffsetNode));
nuevo->offset = offset;
nuevo->next = actual->offsets;
actual->offsets = nuevo;

}

// Escribe la tabla hash y listas de offsets en archivos binarios
void guardar_tabla_hash(const char* archivo_index, const char* archivo_offsets) {
FILE* f_index = fopen(archivo_index, "wb");
FILE* f_offsets = fopen(archivo_offsets, "wb");

if (!f_index || !f_offsets) {
    perror("Error abriendo archivos binarios");
    exit(1);
}

for (int i = 0; i < TABLE_SIZE; i++) {
    HashEntry* actual = hash_table[i];
    while (actual) {
        // Escribir clave
        fwrite(&i, sizeof(int), 1, f_index);
        fwrite(actual->nombreDepartamento, sizeof(char), MAX_DEPARTAMENTO, f_index);

        // Obtener cantidad de offsets
        int count = 0;
        for (OffsetNode* p = actual->offsets; p; p = p->next) {
            count++;
        }
        fwrite(&count, sizeof(int), 1, f_index);

        // Escribir offsets en archivo de listas
        long first_offset_pos = ftell(f_offsets);
        for (OffsetNode* p = actual->offsets; p; p = p->next) {
            fwrite(&p->offset, sizeof(long), 1, f_offsets);
        }

        // Volver a escribir posición inicial de offsets (por si se usa luego)
        fwrite(&first_offset_pos, sizeof(long), 1, f_index);

        actual = actual->next;
    }
}

fclose(f_index);
fclose(f_offsets);

}

// Limpia toda la memoria usada
void liberar_memoria() {
for (int i = 0; i < TABLE_SIZE; i++) {
HashEntry* entry = hash_table[i];
while (entry) {
OffsetNode* n = entry->offsets;
while (n) {
OffsetNode* temp = n;
n = n->next;
free(temp);
}
HashEntry* tmp = entry;
entry = entry->next;
free(tmp);
}
hash_table[i] = NULL;
}
}

// Lee el archivo CSV línea por línea, indexando por nombre del departamento
void procesar_csv(const char* nombre_csv) {
FILE* f = fopen(nombre_csv, "r");
if (!f) {
perror("Error al abrir CSV");
exit(1);
}

char linea[LINE_BUFFER_SIZE];
long offset = 0;

// Leer cabecera
fgets(linea, sizeof(linea), f);
offset = ftell(f);

while (fgets(linea, sizeof(linea), f)) {
    // Guardar offset actual de esta línea
    long linea_offset = offset;
    offset = ftell(f);

    // Separar campos
    char* campos[24];
    int i = 0;
    campos[i++] = strtok(linea, ",");

    while (i < 24 && (campos[i] = strtok(NULL, ","))) {
        i++;
    }

    // Validar si tenemos suficientes columnas
    if (i < 10) continue;

    const char* nombreDepartamento = campos[4]; // campo 5 (base 0)
    if (!nombreDepartamento || strlen(nombreDepartamento) == 0) continue;

    insertar(nombreDepartamento, linea_offset);
}

fclose(f);

}

// Función principal
int main() {
const char* csv = "/home/anna/so/practica/dataset.csv";
const char* index_file = "depto_index.bin";
const char* offset_file = "depto_lists.bin";

procesar_csv(csv);
guardar_tabla_hash(index_file, offset_file);
liberar_memoria();

printf("Indexación completada.\n");
return 0;

}