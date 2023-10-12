#define _GNU_SOURCE
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>

#define OPT_CONNECT "CONNECT"
#define OPT_CHECK_DOWNLOADS "CHECK DOWNLOADS"
#define OPT_DOWNLOAD "DOWNLOAD"
#define OPT_LIST_PLAYLISTS "LIST PLAYLISTS"
#define OPT_LOGOUT "LOGOUT"

int connected = 0;

typedef struct{
    char *name;
    char *folder;
    char *ip;
    int port;
}bowman;

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

void main_menu(){

}

int main(int argc, char *argv[]){
    char *buffer;
    char *line;
    bowman bowman;

    if (argc != 2) {
        asprintf(&buffer, "ERROR: Expecting on parameter\n");
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