#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define INPUT_SIZE 100
#define NAME_SIZE 30
#define COMMAND_SIZE 30
#define ARG_SIZE 30

// TODO modo workflow


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
        } else if (strlen(auxString) < NAME_SIZE) {
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

Task *findTask(TaskList *taskList, char *name) {
    Task *aux = taskList->head, *task = NULL;
    while (aux != NULL) {
        if (strcmp(aux->name, name) == 0) {
            task = aux;
        }
        aux = aux->next;
    }
    // printf("Retornando uma task\n");
    return task;
}


void newTask(TaskList *taskList, char **array, int qtd) {
    if (qtd < 4) {
        printf("Quantidade invalida de argumentos.\n");
        return;
    }

    Task *task = (Task *) malloc(sizeof(Task)), *aux;
    if (task == NULL) {
        printf("Falha ao adicionar task.\n");
        return;
    }
    strcpy(task->name, array[1]);
    strcpy(task->command, array[2]);

    task->args = (char **) malloc((qtd-2) * sizeof(char *));
    if (task->args == NULL) {
        printf("Falha ao adicionar task.\n");
        free(task);
        task = NULL;
        return;
    }
    // printf("Antes do for arg\n");
    for (int i=2; i<qtd; i++) {
        (task->args)[i-2] = (char *) malloc(ARG_SIZE * sizeof(char));
        if ((task->args)[i-2] == NULL) {
            printf("Falha ao adicionar task.\n");
            for (int j=(i-3); j>=0; j--) {
                free((task->args)[j]);
                (task->args)[j] = NULL;
            }
            free(task);
            task = NULL;
            return;
        }
        // printf("Antes do strcpy\n");
        if (array[i] != NULL) {
            strcpy((task->args)[i-2], array[i]);
        } else {
            (task->args)[i-2] = NULL;
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
    for (int i=2; i<qtd; i++) {
        if ((task->args)[i-2] != NULL) {
            printf("%s, ", (task->args)[i-2]);
        } else {
            printf("NULL");
        }
        
    }
    printf("\n");

    return;
}


void runTaskPipe(TaskList *taskList, char **array, int qtd) {
    int n = qtd - 2, pid[n];
    int fd[n-2][2], readIndex, writeIndex;
    Task **taskQueue = (Task **) malloc(n * sizeof(Task *));
    if (taskQueue == NULL) {
        printf("Erro ao executar o comando.\n");
        return;
    }
    for (int i=2; i<(qtd-1); i++) {
        // printf("%d ", i-2);
        taskQueue[i-2] = findTask(taskList, array[i]);
        if (taskQueue[i-2] == NULL) {
            printf("Task nao encontrada na lista.\n");
            free(taskQueue);
            taskQueue = NULL;
            return;
        }
    }
    // printf("Achou as tasks\n");
    for (int i=0; i<(n-2); i++) {
        if (pipe(fd[i]) == -1) {
            printf("Erro ao executar o comando.\n");
            free(taskQueue);
            taskQueue = NULL;
            return;
        }
    }

    // for (int i=0; i<(n-1); i++) {
    //     printf("%s, ", taskQueue[i]->name);
    // }
    // printf("\n");

    for (int i=0; i<(n-1); i++) {
        pid[i] = fork();
        if (pid[i] == -1) {
            printf("Erro ao executar o comando.\n");
            free(taskQueue);
            taskQueue = NULL;
            return;
        }

        if (pid[i] == 0) {
            // Processo filho
            readIndex = i - 1;
            writeIndex = i;
			for (int j=0; j<(n-2); j++) {
				if (j != readIndex) {
					close(fd[j][0]);
                    // printf("%d: Closed fd[%d][0]\n", i, j);
				}
				if (j != writeIndex) {
					close(fd[j][1]);
                    // printf("%d: Closed fd[%d][1]\n", i, j);
				}
			}

			if (readIndex > -1) {
                // printf("%d: Ler o index %d\n", i, readIndex);
                // printf("%d: Closed fd[%d][0]\n", i, readIndex);
				dup2(fd[readIndex][0], STDIN_FILENO);
				close(fd[readIndex][0]);
			}
			if (writeIndex < (n-2)) {
                // printf("%d: Escrever para o index %d\n", i, writeIndex);
                // printf("%d: Closed fd[%d][1]\n", i, writeIndex);
				dup2(fd[writeIndex][1], STDOUT_FILENO);
				close(fd[writeIndex][1]);
			}

            execvp(taskQueue[i]->command, taskQueue[i]->args);

            printf("Comando nao encontrado.\n");
            exit(1);
        }
    }
    // Processo pai (paralelo)
	for (int i=0; i<(n-2); i++) {
		close(fd[i][0]);
		close(fd[i][1]);
	}
    for (int i=0; i<(n-1); i++) {
        wait(NULL);
        // printf("Esperou por um processo\n");
    }

    free(taskQueue);
    taskQueue = NULL;
    return;
}

void runTaskSP(TaskList *taskList, char **array, int qtd, int type) {
    int n = qtd - 2, pid[n];
    Task **taskQueue = (Task **) malloc(n * sizeof(Task *));
    if (taskQueue == NULL) {
        printf("Erro ao executar o comando.\n");
        return;
    }
    for (int i=2; i<(qtd-1); i++) {
        // printf("%d ", i-2);
        taskQueue[i-2] = findTask(taskList, array[i]);
        if (taskQueue[i-2] == NULL) {
            printf("Task nao encontrada na lista.\n");
            free(taskQueue);
            taskQueue = NULL;
            return;
        }
    }
    // printf("Achou as tasks\n");

    // for (int i=0; i<(n-1); i++) {
    //     printf("%s, ", taskQueue[i]->name);
    // }
    // printf("\n");

    for (int i=0; i<(n-1); i++) {
        pid[i] = fork();
        if (pid[i] == -1) {
            printf("Erro ao executar o comando.\n");
            free(taskQueue);
            taskQueue = NULL;
            return;
        }

        if (pid[i] == 0) {
            // Processo filho
            execvp(taskQueue[i]->command, taskQueue[i]->args);

            printf("Comando nao encontrado.\n");
            exit(1);
        }
        // Processo pai (sequencial)
        if (type == 0) {
            wait(NULL);
        }
    }
    // Processo pai (paralelo)
    if (type == 1) {
        for (int i=0; i<n; i++) {
            wait(NULL);
        }
    }

    free(taskQueue);
    taskQueue = NULL;
    return;
}

void runTaskAlone(TaskList *taskList, char **array) {
    int pid;
    Task *task = findTask(taskList, array[1]);
    if (task == NULL) {
        printf("Task nao encontrada na lista.\n");
        return;
    }
    // printf("Achou a task\n");

    pid = fork();
    if (pid == -1) {
        printf("Erro ao executar o comando.\n");
        return;
    }

    if (pid == 0) {
        execvp(task->command, task->args);

        printf("Comando nao encontrado.\n");
        exit(1);
    }

    wait(NULL);

    return;
}

void runTask(TaskList *taskList, char **array, int qtd) {
    // qtd conta o NULL
    if (qtd == 3) {
        runTaskAlone(taskList, array);
    }
    else if (qtd < 4) {
        printf("Quantidade invalida de argumentos.\n");
        return;
    }
    else if (strcmp(array[1], "sequential") == 0) {
        runTaskSP(taskList, array, qtd, 0);
    }
    else if (strcmp(array[1], "parallel") == 0) {
        runTaskSP(taskList, array, qtd, 1);
    }
    else if (strcmp(array[1], "pipe") == 0) {
        runTaskPipe(taskList, array, qtd);
    }
    else {
        printf("Comando para run invalido.\n");
    }

    return;
}


void processString(TaskList *taskList, char *str) {
    int qtd = getQtd(str) + 1;
    // printf("%s\nQtd: %d + 1\n", str, qtd-1);
    char **array = getCommand(str, qtd);

    // printf("[");
    // for (int i=0; i<qtd; i++) {
    //     if (array[i] != NULL) {
    //         printf("%s, ", array[i]);
    //     } else {
    //         printf("NULL");
    //     }
    // }
    // printf("]\n");

    if (strcmp(array[0], "task") == 0) {
        newTask(taskList, array, qtd);
    }
    else if (strcmp(array[0], "run") == 0) {
        runTask(taskList, array, qtd);
    }
    else {
        printf("Comando invalido.\n");
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
            if (fgets(str, INPUT_SIZE, stdin) == NULL) {
                printf("\n");
                return 0;
            }
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