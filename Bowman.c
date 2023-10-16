#define _GNU_SOURCE
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>

#define OPT_CONNECT "CONNECT"
#define OPT_CHECK_DOWNLOADS "CHECK DOWNLOADS"
#define OPT_LIST_SONGS "LIST SONGS"
#define OPT_DOWNLOAD "DOWNLOAD"
#define OPT_LIST_PLAYLISTS "LIST PLAYLISTS"
#define OPT_LOGOUT "LOGOUT"
#define OPT_CLEAR_DOWNLOADS "CLEAR DOWNLOADS"

typedef struct{
    char *name;
    char *folder;
    char *ip;
    int port;
}Bowman;

int connected = 0;
Bowman bowman;

char * read_until(int fd, char end) {
	char *string = NULL;
	char c;
	int i = 0, size;

	while (1) {
		size = read(fd, &c, sizeof(char));
		if(string == NULL){
			string = (char *) malloc(sizeof(char));
		}
		if(c != end && size > 0){
			string = (char *) realloc(string, sizeof(char)*(i + 2));
			string[i++] = c;
		}else{
			break;
		}
	}
	string[i] = '\0';
	return string;
}

void ksigint(){
    exit(EXIT_SUCCESS);
}

void main_menu(){
    char *printBuffer;
    int inputLength;
    char *input;
    char *toDownload;
    char buffer[100];

    while(1){
        int split = 0;
        inputLength = read(STDIN_FILENO, buffer, 100);
        buffer[inputLength - 1] = '\0';

        if(inputLength > 0){
            for(int i=0; i<inputLength; i++){
                if(buffer[i] == ' '){
                    //split = 1;
                    break;
                }
            }

            if(split){
                input = strtok(buffer, " ");
                toDownload = strtok(NULL, "\n");
                printf("input: %s\n", input);
                printf("toDownload: %s\n", toDownload);
            }else{
                input = (char *)malloc(inputLength);
                strncpy(input, buffer, inputLength);
                input[inputLength - 1] = '\0';
            }
        }

        if(strcasecmp(input, OPT_CONNECT) == 0){
            if(connected){
                asprintf(&printBuffer, "%s is already connected.\n", bowman.name);
                write(1, printBuffer, strlen(printBuffer));
                free(printBuffer);
            }else{
                connected = 1;
                asprintf(&printBuffer, "%s connected to HAL 9000 system, welcome music lover!\n", bowman.name);
                write(1, printBuffer, strlen(printBuffer));
                free(printBuffer);
            }
        }

        else if(strcasecmp(input, OPT_LOGOUT) == 0){
            if(connected){

            }else{
                asprintf(&printBuffer, "Cannot Logout, you are not connected to HAL 9000\n");
                write(1, printBuffer, strlen(printBuffer));
                free(printBuffer);
            }
        }

        else if(strcasecmp(input, OPT_LIST_SONGS) == 0){
            if(connected){

            }else{
                asprintf(&printBuffer, "Cannot List Songs, you are not connected to HAL 9000\n");
                write(1, printBuffer, strlen(printBuffer));
                free(printBuffer);
            }
        }

        else if(strcasecmp(input, OPT_LIST_PLAYLISTS) == 0){
            if(connected){

            }else{
                asprintf(&printBuffer, "Cannot List Playlist, you are not connected to HAL 9000\n");
                write(1, printBuffer, strlen(printBuffer));
                free(printBuffer);
            }
        }

        else if(strcasecmp(input, OPT_DOWNLOAD) == 0){
            if(connected){

            }else{
                asprintf(&printBuffer, "Cannot Download, you are not connected to HAL 9000\n");
                write(1, printBuffer, strlen(printBuffer));
                free(printBuffer);
            }
        }

        else if(strcasecmp(input, OPT_CHECK_DOWNLOADS) == 0){
            if(connected){

            }else{
                asprintf(&printBuffer, "Cannot Check Downloads, you are not connected to HAL 9000\n");
                write(1, printBuffer, strlen(printBuffer));
                free(printBuffer);
            }
        }

        else if(strcasecmp(input, OPT_CLEAR_DOWNLOADS) == 0){
            if(connected){

            }else{
                asprintf(&printBuffer, "Cannot Clear Downloads, you are not connected to HAL 9000\n");
                write(1, printBuffer, strlen(printBuffer));
                free(printBuffer);
            }
        }

        else{
            asprintf(&printBuffer, "Invalid option.\n");
            write(1, printBuffer, strlen(printBuffer));
            free(printBuffer);
        }
    }
}

int main(int argc, char *argv[]){
    char *buffer;
    char *line;
    //Bowman bowman;

    asprintf(&buffer, "\nPID: %d\n", getpid());
    write(STDOUT_FILENO, buffer, strlen(buffer));
    free(buffer);

    signal(SIGINT, ksigint);

    if (argc != 2) {
        asprintf(&buffer, "ERROR: Expecting one parameter.\n");
        write(1, buffer, strlen(buffer));
        free(buffer);
        return -1;
    }

    int fd_bowman = open(argv[1], O_RDONLY);

    if(fd_bowman == -1){
        perror("Error opening bowman file");
        exit(EXIT_FAILURE);
    }

    line = read_until(fd_bowman, '\n');
    bowman.name = malloc(sizeof(char) * (strlen(line) + 1));
    strcpy(bowman.name, line);
    free(line);

    line = read_until(fd_bowman, '\n');
    bowman.folder = malloc(sizeof(char) * (strlen(line) + 1));
    strcpy(bowman.folder, line);
    free(line);

    line = read_until(fd_bowman, '\n');
    bowman.ip = malloc(sizeof(char) * (strlen(line) + 1));
    strcpy(bowman.ip, line);
    free(line);

    line = read_until(fd_bowman, '\n');
    bowman.port = atoi(line);
    free(line);
    

    asprintf(&buffer, "\n%s user initialized.\n", bowman.name);
    write(1, buffer, strlen(buffer));
    free(buffer);

    main_menu();
    return 0;
}