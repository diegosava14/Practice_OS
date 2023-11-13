#define _GNU_SOURCE
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>

#define OPT_CONNECT "CONNECT"
#define OPT_CHECK_DOWNLOADS1 "CHECK"
#define OPT_CHECK_DOWNLOADS2 "DOWNLOADS"
#define OPT_LIST_SONGS1 "LIST"
#define OPT_LIST_SONGS2 "SONGS"
#define OPT_DOWNLOAD "DOWNLOAD"
#define OPT_LIST_PLAYLISTS1 "LIST"
#define OPT_LIST_PLAYLISTS2 "PLAYLISTS"
#define OPT_LOGOUT "LOGOUT"
#define OPT_CLEAR_DOWNLOADS1 "CLEAR"
#define OPT_CLEAR_DOWNLOADS2 "DOWNLOADS"

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
    write(1, "\n", 1);
    exit(EXIT_SUCCESS);
}

void main_menu(){
    int inputLength;
    char *printBuffer;
    char buffer[100];

    while (1) {
        int wordCount = 0;
        int spaceCount = 0;
        write(1, "$ ", 2);

        inputLength = read(STDIN_FILENO, buffer, 100);
        buffer[inputLength - 1] = '\0';

        if (inputLength > 0) {
            for (int j = 0; j < inputLength; j++) {
                if (buffer[j] == ' ') {
                    spaceCount++;
                }
            }
        }

        char *input[spaceCount + 1];
        char *token = strtok(buffer, " \t");

        while (token != NULL && wordCount < spaceCount + 1) {
            input[wordCount] = token;
            token = strtok(NULL, " \t");
            wordCount++;
        }

        if((strcasecmp(input[0], OPT_CONNECT) == 0)&&(wordCount == 1)){
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

        else if((strcasecmp(input[0], OPT_LOGOUT) == 0)&&(wordCount == 1)){
            if(connected){

            }else{
                asprintf(&printBuffer, "Cannot Logout, you are not connected to HAL 9000\n");
                write(1, printBuffer, strlen(printBuffer));
                free(printBuffer);
            }
        }

        else if((strcasecmp(input[0], OPT_LIST_SONGS1) == 0)&&(strcasecmp(input[1], OPT_LIST_SONGS2) == 0)
        &&(wordCount == 2)){
            if(connected){

            }else{
                asprintf(&printBuffer, "Cannot List Songs, you are not connected to HAL 9000\n");
                write(1, printBuffer, strlen(printBuffer));
                free(printBuffer);
            }
        }

        else if((strcasecmp(input[0], OPT_LIST_PLAYLISTS1) == 0)&&(strcasecmp(input[1], OPT_LIST_PLAYLISTS2) == 0)
        &&(wordCount == 2)){
            if(connected){

            }else{
                asprintf(&printBuffer, "Cannot List Playlist, you are not connected to HAL 9000\n");
                write(1, printBuffer, strlen(printBuffer));
                free(printBuffer);
            }
        }

        else if((strcasecmp(input[0], OPT_DOWNLOAD) == 0)&&(wordCount == 2)){
            if(connected){

            }else{
                asprintf(&printBuffer, "Cannot Download, you are not connected to HAL 9000\n");
                write(1, printBuffer, strlen(printBuffer));
                free(printBuffer);
            }
        }

        else if((strcasecmp(input[0], OPT_CHECK_DOWNLOADS1) == 0)&&(strcasecmp(input[1], OPT_CHECK_DOWNLOADS2) == 0)
        &&(wordCount == 2)){
            if(connected){

            }else{
                asprintf(&printBuffer, "Cannot Check Downloads, you are not connected to HAL 9000\n");
                write(1, printBuffer, strlen(printBuffer));
                free(printBuffer);
            }
        }

        else if((strcasecmp(input[0], OPT_CLEAR_DOWNLOADS1) == 0)&&(strcasecmp(input[1], OPT_CLEAR_DOWNLOADS2) == 0)
        &&(wordCount == 2)){
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
    int numAmpersand = 0;
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
    size_t length = strlen(line);

    for (size_t i = 0; i < length; i++) {
        if (line[i] == '&') {
            numAmpersand++;
        } else {
            line[i - numAmpersand] = line[i];
        }
    }

    line[length - numAmpersand] = '\0';
    bowman.name = malloc(sizeof(char) * (length - numAmpersand + 1));
    strcpy(bowman.name, line);
    bowman.name[length - numAmpersand] = '\0';
    free(line);

    line = read_until(fd_bowman, '\n');
    bowman.folder = malloc(sizeof(char) * (strlen(line) + 1));
    strcpy(bowman.folder, line);
    bowman.folder[strlen(line)] = '\0';
    free(line);

    line = read_until(fd_bowman, '\n');
    bowman.ip = malloc(sizeof(char) * (strlen(line) + 1));
    strcpy(bowman.ip, line);
    bowman.ip[strlen(line)] = '\0';
    free(line);

    line = read_until(fd_bowman, '\n');
    bowman.port = atoi(line);
    free(line);
    
    close(fd_bowman);

    asprintf(&buffer, "\n%s user initialized.\n", bowman.name);
    write(1, buffer, strlen(buffer));
    free(buffer);

    asprintf(&buffer, "\nFile read correctly: \n");
    write(1, buffer, strlen(buffer));
    free(buffer);

    asprintf(&buffer, "User - %s\n", bowman.name);
    write(1, buffer, strlen(buffer));
    free(buffer);

    asprintf(&buffer, "Directory - %s\n", bowman.folder);
    write(1, buffer, strlen(buffer));
    free(buffer);

    asprintf(&buffer, "IP - %s\n", bowman.ip);
    write(1, buffer, strlen(buffer));
    free(buffer);

    asprintf(&buffer, "Port - %d\n\n", bowman.port);
    write(1, buffer, strlen(buffer));
    free(buffer);

    main_menu();
    return 0;
}