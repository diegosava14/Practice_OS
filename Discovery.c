#define _GNU_SOURCE
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <stdint.h>
#include <strings.h>

//PooleServer **servers;
//int num_servers = 0;

typedef struct{
    char *ipPoole;
    int portPoole;
    char *ipBowman;
    int portBowman;
}Discovery;

typedef struct{
    char *name;
    char *ip;
    int port;
}PooleServer; 

typedef struct{
    uint8_t type;
    uint16_t headerLength;
    char *header;
    char *data;
}Frame; 

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

void *discovery_poole(void *arg) {
    Discovery *discovery = (Discovery *)arg;

    uint16_t port;
    int aux = discovery->portPoole;
    if (aux < 1 || aux > 65535) {
        perror("port");
        exit(EXIT_FAILURE);
    }
    port = aux;

    int sockfd;
    sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sockfd < 0) {
        perror("socket TCP");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in s_addr;
    bzero(&s_addr, sizeof(s_addr));
    s_addr.sin_family = AF_INET;
    s_addr.sin_port = htons(port);
    s_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(sockfd, (void *)&s_addr, sizeof(s_addr)) < 0) {
        perror("bind");
        exit(EXIT_FAILURE);
    }

    listen(sockfd, 5);

    struct sockaddr_in c_addr;
    socklen_t c_len = sizeof(c_addr);

    printf("Waiting for connections on port %hu\n", ntohs(s_addr.sin_port));
    int newsock = accept(sockfd, (void *)&c_addr, &c_len);
    if (newsock < 0) {
        perror("accept");
        exit(EXIT_FAILURE);
    }

    printf("New connection from %s:%hu\n", inet_ntoa(c_addr.sin_addr), ntohs(c_addr.sin_port));

    /*
    Frame *newFrame = malloc(sizeof(Frame));
    read(newsock, &newFrame, 24);
    printf("%s\n", newFrame->data);
    free(newFrame);*/

    close(newsock);

    close(sockfd);

    return NULL;
}

void newFramePoole(){

}

void *discovery_bowman(void *arg){
    Discovery *discovery = (Discovery*)arg;
    int aux = discovery->portBowman;
    aux --;
    return NULL;
}

int main(int argc, char *argv[]){
    char *buffer;
    char *line;
    Discovery discovery;

    if (argc != 2) {
        asprintf(&buffer, "ERROR: Expecting one parameter.\n");
        write(1, buffer, strlen(buffer));
        free(buffer);
        return -1;
    }

    //READING DISCOVERY FILE

    int fd_discovery = open(argv[1], O_RDONLY);

    if(fd_discovery == -1){
        perror("Error opening poole file");
        exit(EXIT_FAILURE);
    }

    line = read_until(fd_discovery, '\n');
    discovery.ipPoole = malloc(sizeof(char) * (strlen(line) + 1));
    strcpy(discovery.ipPoole , line);
    discovery.ipPoole [strlen(line)] = '\0';
    free(line);

    line = read_until(fd_discovery, '\n');
    discovery.portPoole = atoi(line);
    free(line);

    line = read_until(fd_discovery, '\n');
    discovery.ipBowman = malloc(sizeof(char) * (strlen(line) + 1));
    strcpy(discovery.ipBowman , line);
    discovery.ipBowman [strlen(line)] = '\0';
    free(line);

    line = read_until(fd_discovery, '\n');
    discovery.portBowman = atoi(line);
    free(line);

    close(fd_discovery);

    pthread_t thread_ids[2];
    int status[2];

    status[0] = pthread_create(&thread_ids[0], NULL, discovery_poole, (void*)&discovery);

    if(status[0] != 0){
        perror("Error creating thread");
        exit(EXIT_FAILURE);
    }

    status[1] = pthread_create(&thread_ids[1], NULL, discovery_bowman, (void*)&discovery);

    if(status[1] != 0){
        perror("Error creating thread");
        exit(EXIT_FAILURE);
    }

    free(discovery.ipPoole);
    free(discovery.ipBowman);

    for(int i = 0; i < 2; i++){
        pthread_join(thread_ids[i], NULL);
    }

    return 0;
}