#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <signal.h>

#define INPUT_SIZE 100
#define NAME_SIZE 30
#define COMMAND_SIZE 30
#define ARG_SIZE 30


typedef struct task {
	char name[NAME_SIZE];
	char command[COMMAND_SIZE];
	char** args;

    char input[NAME_SIZE];
    char output[NAME_SIZE];
    char ioType[2];

	struct task *next;
} Task;

typedef struct {
	Task *head;
} TaskList;

typedef struct job {
    int id;
    int pid;
    struct job *next;
} Job;

typedef struct {
    Job *head;
} JobList;

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

    while (str[sIndex] != '\0' && str[sIndex] != '\n') {
        aux = str[sIndex];
        if (aux == ' ') {
            if (prev != ' ' && vIndex < (qtd-1)) {
                strcpy(array[vIndex], auxString);

                // printf("Copiei %s no index %d\n", array[vIndex], vIndex);
                // if (vIndex > 0) {
                //     printf("- Anterior: %s\n", array[vIndex-1]);
                // }
                strcpy(auxString, "");
                vIndex++;
            }

        } else if (aux == '\n') {
            // printf("Quebra de linha encontrada (%d)\n", sIndex);
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
        // for (int i=0; i<strlen(auxString); i++) {
        //     if (auxString[i] == '\n') {
        //         printf("/n a a a a a a a");
        //     } else if (auxString[i] == '\0') {
        //         printf("/0 a a a a a a a");
        //     } else {
        //         printf("%c", auxString[i]);
        //     }
        // }
        // printf("\n");

        // auxString[strlen(auxString) - 1] = '\0';
        strcpy(array[vIndex], auxString);
        // printf("Copiei %s no index %d\n", array[vIndex], vIndex);
        // if (vIndex > 0) {
        //     printf("- Anterior: %s\n", array[vIndex-1]);
        // }
    }
    array[qtd-1] = NULL;

    // for (int i=0; i<(qtd-1); i++) {

    //     printf("%s, ", array[i]);
    // }
    // printf("\n");

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
    strcpy(task->input, "");
    strcpy(task->output, "");
    strcpy(task->ioType, "");
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

    // printf("Nome: %s\n", task->name);
    // printf("Comando: %s\n", task->command);
    // printf("Argumentos: ");
    // for (int i=2; i<qtd; i++) {
    //     if ((task->args)[i-2] != NULL) {
    //         printf("%s, ", (task->args)[i-2]);
    //     } else {
    //         printf("NULL");
    //     }
        
    // }
    // printf("\n");

    return;
}


void changeDirectory(char **array, int qtd) {
    if (qtd != 3) {
        printf("Numero de argumentos invalido.\n");
        return;
    }
    if (chdir(array[1]) == -1) {
        printf("Diretorio nao encontrado.\n");
    }
}


void changeTaskIO(TaskList *taskList, char **array, char *ioType, int qtd) {
    if (qtd != 4) {
        printf("Numero invalido de argumentos.\n");
        return;
    }
    Task *task = findTask(taskList, array[1]);
    if (task == NULL) {
        printf("Task nao encontrada na lista.\n");
        return;
    }
    if (strcmp(ioType, "r") == 0) {
        strcpy(task->input, array[2]);
    } else if (strcmp(ioType, "w") == 0 || strcmp(ioType, "a") == 0) {
        strcpy(task->output, array[2]);
    }
    strcpy(task->ioType, ioType);

    return;
}

int changeIO(Task *task, int read, int write) {
    int arq1 = -1, arq2 = -1;
    if (strcmp(task->input, "") != 0 && read == 1) {
        arq1 = open(task->input, O_RDONLY, 0777);
        if (arq1 == -1) {
            printf("Arquivo de entrada nao encontrado.\n");
            return -1;
        }
        dup2(arq1, STDIN_FILENO);
        close(arq1);
    }
    if (strcmp(task->output, "") != 0 && write == 1) {
        if (strcmp(task->ioType, "w") == 0) {
            arq2 = open(task->output, O_WRONLY | O_CREAT, 0777);
        }
        else if (strcmp(task->ioType, "a") == 0) {
            arq2 = open(task->output, O_WRONLY | O_APPEND | O_CREAT, 0777);
        }
        if (arq2 == -1) {
            printf("Arquivo de saida nao encontrado.\n");
            return -1;
        }
        dup2(arq2, STDOUT_FILENO);
        close(arq2);
    }

    return 0;
}


Job *findJob(JobList *jobList, int id) {
    Job *aux = jobList->head;
    if (aux == NULL) {
        return NULL;
    }
    while (aux != NULL) {
        if (aux->id == id) {
            return aux;
        }
        aux = aux->next;
    }
    return NULL;
}

void waitJob(JobList *jobList, char **array, int qtd) {
    Job *job, *aux = jobList->head;
    int id;
    if (qtd != 3) {
        printf("Numero de argumentos invalido.\n");
        return;
    }
    id = atoi(array[1]);
    if (id == 0) {
        printf("Nao ha um job com esse ID na lista.\n");
        return;
    }
    job = findJob(jobList, id);
    if (job == NULL) {
        printf("Nao ha um job com esse ID na lista.\n");
        return;
    }

    kill(job->pid, SIGCONT);
    waitpid(job->pid, NULL, 0);

    if (aux == job) {
        jobList->head = job->next;
    } else {
        while (aux->next != job) {
            aux = aux->next;
        }
        aux->next = job->next;
    }
    free(job);
    job = NULL;

    return;
}

void waitJobsFinal(Job *job) {
    if (job == NULL) {
        return;
    }
    waitJobsFinal(job->next);

    kill(job->pid, SIGKILL);
    waitpid(job->pid, NULL, 0);
    free(job);
    job = NULL;

    return;
}

void listJobs(JobList *jobList, int qtd) {
    Job *aux = jobList->head;
    if (qtd != 2) {
        printf("Numero de argumentos invalido.\n");
        return;
    }
    if (aux == NULL) {
        printf("Ainda nao ha jobs iniciados.\n");
        return;
    }
    while (aux != NULL) {
        printf("[%d] %d\n", aux->id, aux->pid);
        aux = aux->next;
    }
    return;
}

void startJob(TaskList *taskList, JobList *jobList, char **array, int *id, int qtd) {
    Task *task;
    int pid;
    if (qtd != 3) {
        printf("Numero de argumentos invalido.\n");
        return;
    }
    task = findTask(taskList, array[1]);
    if (task == NULL) {
        printf("Task nao encontrada na lista.\n");
        return;
    }
    // printf("Achou a task\n");

    pid = fork();
    if (pid == -1) {
        printf("Erro ao comecar o job.\n");
        return;
    }

    if (pid == 0) {
        if (changeIO(task, 1, 1) == -1) {
            exit(1);
        }

        execvp(task->command, task->args);

        printf("Comando nao encontrado.\n");
        exit(1);
    }

    kill(pid, SIGSTOP);
    Job *job = (Job *) malloc(sizeof(Job)), *aux;
    if (job == NULL) {
        printf("Erro ao comecar o job.\n");
        kill(pid, SIGKILL);
        return;
    }
    job->id = (*id)++;
    job->pid = pid;
    job->next = NULL;

    if (jobList->head == NULL) {
        jobList->head = job;
    } else {
        aux = jobList->head;
        while (aux->next != NULL) {
            aux = aux->next;
        }
        aux->next = job;
    }
    printf("[%d] %d\n", job->id, job->pid);

    return;
}


void runTaskPipe(TaskList *taskList, char **array, int qtd) {
    int n = qtd - 2, pid[n];
    int fd[n-2][2], readIndex, writeIndex, read, write;
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
            read = (readIndex > -1)? 1 : 0;
            write = (writeIndex < (n-2))? 1 : 0;
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
            if (changeIO(taskQueue[i], !read, !write) == -1) {
                free(taskQueue);
                taskQueue = NULL;
                exit(1);
            }

			if (read == 1) {
                // printf("%d: Ler o index %d\n", i, readIndex);
                // printf("%d: Closed fd[%d][0]\n", i, readIndex);
				dup2(fd[readIndex][0], STDIN_FILENO);
				close(fd[readIndex][0]);
			}
			if (write == 1) {
                // printf("%d: Escrever para o index %d\n", i, writeIndex);
                // printf("%d: Closed fd[%d][1]\n", i, writeIndex);
				dup2(fd[writeIndex][1], STDOUT_FILENO);
				close(fd[writeIndex][1]);
			}

            execvp(taskQueue[i]->command, taskQueue[i]->args);

            printf("Comando nao encontrado.\n");
            free(taskQueue);
            taskQueue = NULL;
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
            if (changeIO(taskQueue[i], 1, 1) == -1) {
                free(taskQueue);
                taskQueue = NULL;
                exit(1);
            }

            execvp(taskQueue[i]->command, taskQueue[i]->args);

            printf("Comando nao encontrado.\n");
            free(taskQueue);
            taskQueue = NULL;
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
        if (changeIO(task, 1, 1) == -1) {
            exit(1);
        }

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


void processString(TaskList *taskList, JobList *jobList, char *str, int *jobId) {
    int qtd = getQtd(str) + 1;
    // printf("Qtd: %d + 1\n", qtd-1);
    if (qtd < 2) {
        return;
    }
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
    else if (strcmp(array[0], "input") == 0) {
        changeTaskIO(taskList, array, "r", qtd);
    }
    else if (strcmp(array[0], "output") == 0) {
        changeTaskIO(taskList, array, "w", qtd);
    }
    else if (strcmp(array[0], "append") == 0) {
        changeTaskIO(taskList, array, "a", qtd);
    }
    else if (strcmp(array[0], "workdir") == 0) {
        changeDirectory(array, qtd);
    }
    else if (strcmp(array[0], "start") == 0) {
        startJob(taskList, jobList, array, jobId, qtd);
    }
    else if (strcmp(array[0], "jobs") == 0) {
        listJobs(jobList, qtd);
    }
    else if (strcmp(array[0], "wait") == 0) {
        waitJob(jobList, array, qtd);
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
    JobList *jobList = (JobList *) malloc(sizeof(JobList));
    if (jobList == NULL) {
        printf("Falha ao comecar o programa.\n");
        free(taskList);
        taskList = NULL;
        return 1;
    }
    jobList->head = NULL;
    char str[INPUT_SIZE], aux[INPUT_SIZE];
    int qtd = 0, jobId = 1;

    if (argc == 1) {
        // Interativo
        while (1) {
            printf("processflow> ");
            if (fgets(str, INPUT_SIZE, stdin) == NULL) {
                printf("\n");
                break;
            }
            str[strlen(str) - 1] = '\0';

            if (strcmp(str, "exit") == 0) {
                break;
            }
            else if (strlen(str) > 0) {
                processString(taskList, jobList, str, &jobId);
            }
        }
    }
    else if (argc == 2) {
        // Workflow
        FILE *arq = fopen(argv[1], "r");
        if (arq == NULL) {
            printf("Falha ao abrir o arquivo.\n");
            free(taskList);
            taskList = NULL;
            free(jobList);
            jobList = NULL;
            return 0;
        }
        while ((fgets(aux, INPUT_SIZE, arq) != NULL)) {
            // printf("Comeco while\n");
            qtd++;
            if (strcspn(aux, "\r\n") < strlen(aux)) {
                aux[strcspn(aux, "\r\n")] = '\0';
            } else if (aux[strcspn(aux, "\n")] < strlen(aux)) {
                aux[strcspn(aux, "\n")] = '\0';
            }
            if (strcmp(aux, "exit") == 0) {
                break;
            }
            // printf("Fim while\n");
        }
        if (qtd == 0) {
            free(taskList);
            taskList = NULL;
            return 0;
        }

        char inputArray[qtd][INPUT_SIZE];
        rewind(arq);

        for (int i=0; i<qtd; i++) {
            // printf("Comeco if 1\n");
            fgets(str, INPUT_SIZE, arq);
            if (strcspn(str, "\r\n") < strlen(str)) {
                str[strcspn(str, "\r\n")] = '\0';
            } else if (str[strcspn(str, "\n")] < strlen(str)) {
                str[strcspn(str, "\n")] = '\0';
            }
            strcpy(inputArray[i], str);
            // printf("Fim if 1\n");
        }

        fclose(arq);

        for (int i=0; i<qtd; i++) {
            // printf("Comeco if 2\n");
            if (strlen(inputArray[i]) > 0) {
                printf("%s\n", inputArray[i]);
                if (strcmp(inputArray[i], "exit") != 0) {
                    processString(taskList, jobList, inputArray[i], &jobId);
                }
            }
            // printf("Fim if 2\n");
        }
    }

    freeTaskList(taskList->head);
    free(taskList);
    taskList = NULL;
    waitJobsFinal(jobList->head);
    free(jobList);
    jobList = NULL;
    
    return 0;
}