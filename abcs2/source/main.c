#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define INPUT_SIZE 100
#define NAME_SIZE 20
#define COMMAND_SIZE 20
#define ARG_SIZE 20

int getQtd(char *str) {
    // Pegar qtd de argumentos na string de input
    int qtd = 0, i = 0;
    char aux = ' ', prev = ' ';

    while (str[i] != '\0') {
        aux = str[i];
        if (aux != ' ' && prev == ' ') {
            qtd++;
        }
        prev = aux;
        i++;
    }

    return qtd;
}

char **getCommand(char *str, int qtd) {
    int sIndex = 0, vIndex = 0;
    char aux = ' ', prev = ' ', auxString[NAME_SIZE] = "", aux2[2] = " ";
    char **array = (char **) malloc(qtd * sizeof(char *));
    if (array == NULL) {
        return NULL;
    }
    for (int i=0; i<qtd; i++) {
        array[i] = (char *) malloc(NAME_SIZE * sizeof(char));
        if (array[i] == NULL) {
            return NULL;
        }
    }

    while (str[sIndex] != '\0') {
        aux = str[sIndex];
        if (aux == ' ') {
            if (prev != ' ' && vIndex < (qtd-1)) {
                strcpy(array[vIndex], auxString);
                // printf("Copiei %s\n", array[vIndex]);
                // if (vIndex > 0) {
                //     printf("- Anterior: %s\n", array[vIndex-1]);
                // }
                strcpy(auxString, "");
                vIndex++;
            }
        } else {
            aux2[0] = aux;
            // printf("Aux2: %s\n", aux2);
            strcat(auxString, aux2);
        }
        aux2[0] = '\0';
        prev = aux;
        sIndex++;
    }
    if (strlen(auxString) > 0 && vIndex < (qtd-1)) {
        strcpy(array[vIndex], auxString);
        // printf("Copiei %s\n", array[vIndex]);
        // if (vIndex > 0) {
        //     printf("- Anterior: %s\n", array[vIndex-1]);
        // }
    }
    array[qtd-1] = NULL;

    return array;
}

void processString(char *str) {
    int qtd = getQtd(str) + 1;
    // printf("%s\nQtd: %d + 1\n", str, qtd-1);
    char **array = getCommand(str, qtd);

    printf("[");
    for (int i=0; i<qtd; i++) {
        if (array[i] != NULL) {
            printf("%s, ", array[i]);
        } else {
            printf("NULL");
        }
    }
    printf("]\n");

    free(array);
    return;
}

int main(int argc, char** argv) {
    char str[INPUT_SIZE];
    if (argc != 1 && argc != 2) {
        printf("Quantidade invalida de argumentos.\n");
        return 1;
    }
    else if (argc == 1) {
        // Interativo
        while (1) {
            printf("processflow> ");
            fgets(str, INPUT_SIZE, stdin);
            str[strlen(str) - 1] = '\0';

            // Debug
            if (strcmp(str, "exit") == 0) {
                break;
            }
            //

            if (strcmp(str, "") != 0) {
                processString(str);
            }
        }
    }
    else if (argc == 2) {
        // Workflow
    }
    
    return 0;
}