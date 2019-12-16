/* CSC 360
 * Assignment P1
 * Parm Johal
 * V00787710
 */

//Resources used:
//CSC 360 Lecture slides courtesy of Jianping Pan, CSC 360 Tutorial slides, https://brennan.io/2015/01/16/write-a-shell-in-c/ courtesy of Stephen Brennan

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

struct bgProcess {
	pid_t pid;
	char *input;
	struct bgProcess *next;
};

struct bgProcess *processList = NULL;

int getSize(struct bgProcess *list) {
	struct bgProcess *currItem = list;
	int count;
	count = 0;
	while(currItem != NULL) {
		count++;
		currItem = currItem->next;
	}
	return count;
}


int executeCommand(char **tokens) {
	if(tokens[0] == NULL) {
		return 1;
	}
	
	if(strcmp(tokens[0], "cd") == 0) {

		if(tokens[1] == NULL || strcmp(tokens[1], "~") == 0) {
			if(chdir(getenv("HOME")) != 0) {
		       		perror("error");
			}
		}
		else {
			if(chdir(tokens[1]) != 0) {
				perror("error");
			}
		}
	}
	if(strcmp(tokens[0], "bg") == 0) {
		if(tokens[1] == NULL) {
			fprintf(stderr, "need a bg process\n");
			exit(0);
		}
		char **bgTokens = &tokens[1];
		pid_t pid = fork();

		if(pid) {
			//Create new bg process
			int size = 256;
			struct bgProcess *item;

			char *input = malloc(sizeof(char) * size);
			if(!input) {
				fprintf(stderr, "failed to allocate.\n");
				exit(1);
			}

			strcpy(input, tokens[1]);
			
			int i;
			i = 2;
			while(tokens[i] != NULL) {
				strcat(input, " ");
				strcat(input, tokens[i]);
				i++;
			}
			item = (struct bgProcess *) malloc(sizeof(struct bgProcess) * size);
			item->pid = pid;
			item->input = input;
			item->next = NULL;

			if(getSize(processList) == 0) {
				processList = item;
			} else {
				struct bgProcess *newItem = processList;
				while(newItem->next != NULL);
				newItem->next = item;
			}
		}
		else {
			if(execvp(bgTokens[0], bgTokens) == -1) {
				perror("error");
			}
			exit(-1);
		}
		return 1;
	}
	if(strcmp(tokens[0], "bglist") == 0) {
		struct bgProcess *iterator = processList;
		while(iterator != NULL) {
			printf("%d: %s\n", iterator->pid, iterator->input);
			iterator = iterator->next;
		}
	}

	return defCommand(tokens);
}

//ls process
int defCommand(char **args) {
	int pid_state;
	pid_t pid;
	pid_t wpid;
	

	pid = fork();
	
	if(pid < 0) { //error
		fprintf(stderr, "Error in forking");
		exit(0);
	} else if (pid == 0) { //child process
		execvp(args[0], args);
	} else { //parent process
		do {
			wpid = waitpid(pid, &pid_state, WUNTRACED);
		} while(!WIFEXITED(pid_state) && !WIFSIGNALED(pid_state));
	}
	return 1;
}

char *readInput() {
	int size = 256;
	int pos = 0;

	char *input = malloc(sizeof(char) * size);
	if(!input){ fprintf(stderr, "malloc error\n"); exit(1); }

	char c;
	while(1) {
		c = getchar();
		if(c == EOF || c == '\n') {
			input[pos] = '\0';
			return input;
		} else {
			input[pos] = c;
		}
		pos += 1;

		if(pos >= size) {
			size = size * 2;
			input = realloc(input, size);
			if(!input){ fprintf(stderr, "realloc error\n"); exit(1); }
		}

	}
}


//splits the command line arguments
char **tokenize(char *line) {
	char **tokens = (char **)malloc(256 * sizeof(char*));
	if(tokens == NULL) {
		printf("Memory not allocated for tokens. Abort!");
	}
	char *getToken;
	char delimiter[] = " \n";
	getToken = strtok(line, delimiter);
	int i = 0;
	while(getToken) {
		tokens[i] = getToken;
		i++;
		getToken = strtok(NULL, delimiter);
	}
	tokens[i] = NULL;
	return tokens;

}

//releases bg commands after being executed
void releaseBGProcess(){
	if(getSize(processList) > 0){
		pid_t pid = waitpid(0, NULL, WNOHANG);
		while(pid > 0){
			if(pid){
				struct bgProcess *item = processList;
				if(processList->pid == pid){
					printf("%d: %s has terminated.\n", item->pid, item->input);
					processList = processList->next;
					free(item);
				} else if(getSize(processList) > 1){
					struct bgProcess *endingProcess;
					if(item->next->pid == pid){
						endingProcess = item->next;
						printf("%d: %s has terminated.\n", item->next->pid, item->next->input);
						item->next = item->next->next;
					} else{
						for(;item->next->pid == pid; item = item->next);
							endingProcess = item->next;
							printf("%d: %s has terminated.\n", item->next->pid, item->next->input);
							item->next = item->next->next;
					}
				     free(endingProcess);	
				}
			}
			pid = waitpid(0, NULL, WNOHANG);
		}   
	}
}


int main(int argc, char *argv[])
{
	 char *line;
	 char **tokens;
	 int pidStatus;

	 if(line == NULL) {
		 printf("Memory not allocated to CL. Abort!");
		 exit(0);
	 }
	 
	 char host[256];
	 char cwd[256];
	 gethostname(host, sizeof(host));
	 
	 //loop
	 while (1) {
	 	printf("SSI: %s@%s: %s > ", getlogin(), host, getcwd(cwd, sizeof(cwd)));
	 	
		line = readInput();
		
		tokens = tokenize(line);

		executeCommand(tokens);

		free(line);
		free(tokens);

		releaseBGProcess();
			
	 }

	 return 0;	   
		   
}
