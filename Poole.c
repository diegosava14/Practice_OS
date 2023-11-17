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

#define HEADER_NEW_POOLE "NEW_POOLE"
#define HEADER_NEW_BOWMAN "NEW_BOWMAN"
#define HEADER_CON_OK "CON_OK"
#define HEADER_CON_KO "CON_KO"

typedef struct{
    char *nameServer;
    char *folder;
    char *ipDiscovery;
    int portDiscovery;
    char *ipPoole;
    int portPoole;
}Poole;

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

void sendMessage(int sockfd, uint8_t type, uint16_t headerLength, const char *constantHeader, char *data){
    char message[256]; 
    headerLength ++;

    message[0] = type;

    message[1] = headerLength & 0xFF;
    message[2] = (headerLength >> 8) & 0xFF;

    memcpy(&message[3], constantHeader, strlen(constantHeader));
    message[3 + strlen(constantHeader)] = '\0';

    memcpy(&message[3 + strlen(constantHeader) + 1], data, strlen(data));
    message[3 + strlen(constantHeader) + 1 + strlen(data)] = '\0';

    write(sockfd, message, 256);
}

Frame frameTranslation(char message[256]){
    Frame frame;

    frame.type = message[0];

    frame.headerLength = (message[2] << 8) | message[1];

    frame.header = malloc(frame.headerLength + 1);
    strncpy(frame.header, &message[3], frame.headerLength);
    frame.header[frame.headerLength] = '\0';

    if(strcmp(frame.header, HEADER_CON_OK) == 0 || strcmp(frame.header, HEADER_CON_KO) == 0){
        frame.data = NULL;
        return frame;
    }else{
        frame.data = strdup(&message[3 + frame.headerLength]);
    }

    return frame;
}

Frame receiveMessage(int sockfd){
    char message[256];
    int size = read(sockfd, message, 256);
    if(size == 0){
        Frame frame;
        frame.type = 0;
        return frame;
    }
    return frameTranslation(message);
}

void connectToDiscovery(Poole poole){
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
 
    char *data;
    asprintf(&data, "%s&%s&%d", poole.nameServer, poole.ipPoole, poole.portPoole);
    sendMessage(sockfd, 0x01, strlen(HEADER_NEW_POOLE), HEADER_NEW_POOLE, data);
    free(data);

    Frame frame = receiveMessage(sockfd);
    if(strcmp(frame.header, HEADER_CON_OK) == 0){
        printf("Connection accepted\n");
        close(sockfd);
    }else{
        printf("Connection refused\n");
        close(sockfd);
    }
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

    connectToDiscovery(poole);

    return 0;
}
