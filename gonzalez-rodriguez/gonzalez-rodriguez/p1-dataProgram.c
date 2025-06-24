#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

#define MAX_DEPTO 64
#define BUFFER 2048

int main() {
    const char* entrada_fifo = "entrada_fifo";
    const char* salida_fifo = "salida_fifo";

    mkfifo(entrada_fifo, 0666);
    mkfifo(salida_fifo, 0666);

    char departamento[MAX_DEPTO] = "";
    int edad = -1;
    char sexo[10] = "*";

    int opcion = 0;

    while (opcion != 5) {
        printf("\n=== Bienvenido al sistema de búsqueda de casos COVID ===\n");
        printf("Criterios actuales:\n");
        printf("1. Nombre del departamento: %s\n", strlen(departamento) ? departamento : "[cualquiera]");
        if (edad >= 0)
            printf("2. Edad: %d\n", edad);
        else
            printf("2. Edad: [cualquiera]\n");
        printf("3. Sexo (M/F): %s\n", strcmp(sexo, "*") != 0 ? sexo : "[cualquiera]");
        printf("4. Realizar búsqueda\n");
        printf("5. Salir\n");
        printf("Seleccione una opción: ");
        if (scanf("%d", &opcion) != 1) {
            printf("Entrada inválida. Intente nuevamente.\n");
            opcion = 0;
            while (getchar() != '\n');
            continue;
        }
        getchar();

        switch (opcion) {
            case 1: {
                printf("Ingrese el nombre del departamento (vacío para cualquiera): ");
                if (!fgets(departamento, MAX_DEPTO, stdin)) {
                    printf("Error de entrada.\n");
                    break;
                }
                departamento[strcspn(departamento, "\n")] = 0;
                break;
            }

            case 2: {
                char edad_input[16];
                printf("Ingrese la edad exacta (0–110, vacío para cualquiera): ");
                if (!fgets(edad_input, sizeof(edad_input), stdin)) {
                    printf("Error de entrada.\n");
                    break;
                }
                if (edad_input[0] == '\n') {
                    edad = -1;
                } else {
                    int valor = atoi(edad_input);
                    if (valor >= 0 && valor <= 110) {
                        edad = valor;
                    } else {
                        printf("Edad inválida. Debe estar entre 0 y 110.\n");
                    }
                }
                break;
            }

            case 3: {
                printf("Ingrese el sexo (M/F, vacío para cualquiera): ");
                if (!fgets(sexo, sizeof(sexo), stdin)) {
                    printf("Error de entrada.\n");
                    break;
                }
                sexo[strcspn(sexo, "\n")] = 0;
                if (strlen(sexo) == 0) {
                    strcpy(sexo, "*");
                } else if (!(strcmp(sexo, "M") == 0 || strcmp(sexo, "F") == 0)) {
                    printf("Sexo inválido. Solo se permite M o F.\n");
                    strcpy(sexo, "*");
                }
                break;
            }

            case 4: {
                int fd_entrada = open(entrada_fifo, O_WRONLY);
                if (fd_entrada < 0) {
                    perror("Error al abrir entrada_fifo");
                    break;
                }

                char mensaje[256];
                snprintf(mensaje, sizeof(mensaje), "\"%s\" %d %s\n",
                         strlen(departamento) > 0 ? departamento : "*",
                         edad,
                         sexo);
                if (write(fd_entrada, mensaje, strlen(mensaje)) == -1) {
                    perror("Error al escribir en entrada_fifo");
                }
                close(fd_entrada);

                int fd_salida = open(salida_fifo, O_RDONLY);
                if (fd_salida < 0) {
                    perror("Error al abrir salida_fifo");
                    break;
                }

                printf("\n=== RESULTADOS ===\n");

                char buffer[BUFFER];
                ssize_t n;
                while ((n = read(fd_salida, buffer, sizeof(buffer) - 1)) > 0) {
                    buffer[n] = '\0';
                    printf("%s", buffer);
                }
                if (n == -1) {
                    perror("Error al leer de salida_fifo");
                }

                close(fd_salida);

                strcpy(departamento, "");
                edad = -1;
                strcpy(sexo, "*");

                break;
            }

            case 5:
                printf("Saliendo del sistema...\n");
                break;

            default:
                printf("Opción inválida. Intente nuevamente.\n");
                break;
        }
    }

    return 0;
}
