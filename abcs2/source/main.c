#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define INPUT_SIZE 100
#define NAME_SIZE 20
#define COMMAND_SIZE 20
#define ARG_SIZE 20


typedef struct task {
	char name[NAME_SIZE];
	char command[COMMAND_SIZE];
	char** args;
	struct task *next;
} Task;

typedef struct {
	Task *head;
} TaskList;


void freeTaskList(Task *task) {
    if (task == NULL) {
        return;
    }
    freeTaskList(task->next);
    free(task);
    task = NULL;
}

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
        printf("Falha ao processar comando.\n");
        return NULL;
    }
    for (int i=0; i<qtd; i++) {
        array[i] = (char *) malloc(NAME_SIZE * sizeof(char));
        if (array[i] == NULL) {
            printf("Falha ao processar comando.\n");
            for (int j=(i-1); j>=0; j--) {
                free(array[j]);
                array[j] = NULL;
            }
            free(array);
            array = NULL;
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


void newTask(TaskList *taskList, char **array, int qtd) {
    if (qtd < 4) {
        printf("Numero de comandos invalidos.\n");
        return;
    }

    Task *task = (Task *) malloc(sizeof(Task)), *aux;
    if (task == NULL) {
        printf("Falha ao adicionar task.\n");
        return;
    }
    strcpy(task->name, array[1]);
    strcpy(task->command, array[2]);

    task->args = (char **) malloc((qtd-3) * sizeof(char *));
    if (task->args == NULL) {
        printf("Falha ao adicionar task.\n");
        free(task);
        task = NULL;
        return;
    }
    // printf("Antes do for arg\n");
    for (int i=3; i<qtd; i++) {
        (task->args)[i-3] = (char *) malloc(ARG_SIZE * sizeof(char));
        if ((task->args)[i-3] == NULL) {
            printf("Falha ao adicionar task.\n");
            for (int j=(i-4); j>=0; j--) {
                free((task->args)[j]);
                (task->args)[j] = NULL;
            }
            free(task);
            task = NULL;
            return;
        }
        // printf("Antes do strcpy\n");
        if (array[i] != NULL) {
            strcpy((task->args)[i-3], array[i]);
        } else {
            (task->args)[i-3] = NULL;
        }
    }
    // printf("Depois do for arg\n");
    task->next = NULL;

    if (taskList->head == NULL) {
        taskList->head = task;
    } else {
        aux = taskList->head;
        while (aux->next != NULL) {
            aux = aux->next;
        }
        aux->next = task;
    }

    printf("Nome: %s\n", task->name);
    printf("Comando: %s\n", task->command);
    printf("Argumentos: ");
    for (int i=3; i<qtd; i++) {
        if ((task->args)[i-3] != NULL) {
            printf("%s, ", (task->args)[i-3]);
        } else {
            printf("NULL");
        }
        
    }
    printf("\n");

    return;
}


void processString(TaskList *taskList, char *str) {
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

    if (strcmp(array[0], "task") == 0) {
        newTask(taskList, array, qtd);
    }
    else if (strcmp(array[0], "run") == 0) {

    }

    free(array);
    array = NULL;
    return;
}


int main(int argc, char** argv) {
    if (argc != 1 && argc != 2) {
        printf("Quantidade invalida de argumentos.\n");
        return 1;
    }
    TaskList *taskList = (TaskList *) malloc(sizeof(TaskList));
    if (taskList == NULL) {
        printf("Falha ao comecar o programa.\n");
        return 1;
    }
    taskList->head = NULL;
    char str[INPUT_SIZE];

    if (argc == 1) {
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
                processString(taskList, str);
            }
        }
    }
    else if (argc == 2) {
        // Workflow
    }

    freeTaskList(taskList->head);
    free(taskList);
    taskList = NULL;
    
    return 0;
}