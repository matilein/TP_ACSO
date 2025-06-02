#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main(int argc, char **argv) {
    if (argc != 4) {
        fprintf(stderr, "Uso: %s <n> <c> <s>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int n = atoi(argv[1]);      // cantidad de procesos hijos
    int c = atoi(argv[2]);      // mensaje inicial
    int start = atoi(argv[3]);  // proceso que inicia la comunicación (1..n)

    if (n < 1 || start < 1 || start > n) {
        fprintf(stderr, "Entradas inválidas: n>=1, 1<=s<=n\n");
        exit(EXIT_FAILURE);
    }

    int (*pipes)[2] = malloc(n * sizeof(int[2]));
    if (!pipes) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    // Creo las n pipes
    for (int i = 0; i < n; i++) {
        if (pipe(pipes[i]) == -1) {
            perror("pipe");
            for (int j = 0; j < i; j++) {
                close(pipes[j][0]);
                close(pipes[j][1]);
            }
            free(pipes);
            exit(EXIT_FAILURE);
        }
    }

    printf("Se crearán %d procesos, se enviará el valor %d desde proceso %d\n", n, c, start);

    // 0-indexed
    int idx_devuelve = (start - 2 + n) % n;
    int idx_inicia   = start - 1; 

    // Creo los n hijos
    for (int i = 0; i < n; i++) {
        pid_t pid = fork();
        if (pid < 0) {
            perror("fork");
            exit(EXIT_FAILURE);
        }
        if (pid == 0) {
            int next = (i + 1) % n;          
            int write_idx = (i == idx_devuelve) ? idx_inicia : next;
            int value;

            // Cierro todos los descriptores que no voy a usar
            for (int j = 0; j < n; j++) {
                if (j != i) {
                    close(pipes[j][0]); 
                }
                if (j != write_idx) {
                    close(pipes[j][1]); 
                }
            }

            if (read(pipes[i][0], &value, sizeof(int)) != sizeof(int)) {
                // Si hay error o EOF inesperado, simplemente salgo
                exit(EXIT_FAILURE);
            }

            int recibido = value;
            value++;  // incremento para pasar o devolver

            if (i == idx_devuelve) {
                // Este hijo devuelve el valor al padre
                printf("Soy el hijo %d, recibí el valor %d y lo devuelvo al padre aumentado en 1\n",
                       i + 1, recibido);
                write(pipes[write_idx][1], &value, sizeof(int));
            } else {
                // Pasa el mensaje al siguiente hijo del anillo
                printf("Soy el hijo %d, recibí el valor %d y envié el mensaje %d al hijo %d\n",
                       i + 1, recibido, value, next + 1);
                write(pipes[write_idx][1], &value, sizeof(int));
            }

            close(pipes[i][0]);
            close(pipes[write_idx][1]);
            exit(EXIT_SUCCESS);
        }
    }
    for (int j = 0; j < n; j++) {
        if (j != idx_inicia) {
            close(pipes[j][0]);
            close(pipes[j][1]);
        }
    }

    int value = c;
    write(pipes[idx_inicia][1], &value, sizeof(int));
    close(pipes[idx_inicia][1]); 

    for (int i = 0; i < n; i++) {
        wait(NULL);
    }

    if (read(pipes[idx_inicia][0], &value, sizeof(int)) == sizeof(int)) {
        printf("El proceso padre recibe el valor final: %d\n", value);
    } else {
        fprintf(stderr, "Error al leer el valor final\n");
    }
    close(pipes[idx_inicia][0]);

    free(pipes);
    return 0;
}