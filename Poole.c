#define _GNU_SOURCE
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>

typedef struct{
    char *nameServer;
    char *folder;
    char *ipDiscovery;
    int portDiscovery;
    char *ipPoole;
    int portPoole;
}Poole;

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

int main(int argc, char *argv[]){
    char *buffer;
    char *line;
    Poole poole;

    if (argc != 2) {
        asprintf(&buffer, "ERROR: Expecting one parameter.\n");
        write(1, buffer, strlen(buffer));
        free(buffer);
        return -1;
    }

    //READING POOLE FILE

    int fd_poole = open(argv[1], O_RDONLY);

    if(fd_poole == -1){
        perror("Error opening poole file");
        exit(EXIT_FAILURE);
    }

    line = read_until(fd_poole, '\n');
    poole.nameServer = malloc(sizeof(char) * (strlen(line) + 1));
    strcpy(poole.nameServer, line);
    poole.nameServer[strlen(line)] = '\0';
    free(line);

    line = read_until(fd_poole, '\n');
    poole.folder = malloc(sizeof(char) * (strlen(line) + 1));
    strcpy(poole.folder, line);
    poole.folder[strlen(line)] = '\0';
    free(line);

    line = read_until(fd_poole, '\n');
    poole.ipDiscovery = malloc(sizeof(char) * (strlen(line) + 1));
    strcpy(poole.ipDiscovery, line);
    poole.ipDiscovery[strlen(line)] = '\0';
    free(line);

    line = read_until(fd_poole, '\n');
    poole.portDiscovery = atoi(line);
    free(line);

    line = read_until(fd_poole, '\n');
    poole.ipPoole = malloc(sizeof(char) * (strlen(line) + 1));
    strcpy(poole.ipPoole, line);
    poole.ipPoole[strlen(line)] = '\0';
    free(line);

    line = read_until(fd_poole, '\n');
    poole.portPoole = atoi(line);
    free(line);

    close(fd_poole);

    //SOCKETS

    uint16_t port;
    int aux = poole.portDiscovery;
    if (aux < 1 || aux > 65535)
    {
        perror ("port");
        exit (EXIT_FAILURE);
    }
    port = aux;

    struct in_addr ip_addr;
    if (inet_aton (poole.ipDiscovery, &ip_addr) == 0)
    {
        perror ("inet_aton");
        exit (EXIT_FAILURE);
    }

    // Create the socket
    int sockfd;
    sockfd = socket (AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sockfd < 0)
    {
        perror ("socket TCP");
        exit (EXIT_FAILURE);
    }

    struct sockaddr_in s_addr;
    bzero (&s_addr, sizeof (s_addr));
    s_addr.sin_family = AF_INET;
    s_addr.sin_port = htons (port);
    s_addr.sin_addr = ip_addr;

    // We can connect to the server casting the struct:
    // connect waits for a struct sockaddr* and we are passing a struct sockaddr_in*
    if (connect (sockfd, (void *) &s_addr, sizeof (s_addr)) < 0)
    {
        perror ("connect");
        exit (EXIT_FAILURE);
    }

    close(sockfd);

    return 0;
}
