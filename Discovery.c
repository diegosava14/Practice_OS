#include "common.h"

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
    int connnections;
}PooleServer; 

Frame frame; 
PooleServer **servers;
int num_servers = 0;
Discovery discovery;
int pooleSockfd, bowmanSockfd;
fd_set rfds;
int *clients;
int num_clients = 0;

void ksigint(){
    FD_ZERO(&rfds);

    close(pooleSockfd);
    close(bowmanSockfd);

    for(int i=0; i<num_servers; i++){
        free(servers[i]->name);
        free(servers[i]->ip);
        free(servers[i]);
    }

    for (int i = 0; i < num_clients; i++)
    {
        close(clients[i]);
    }
    
    free(clients);
    free(servers);

    free(discovery.ipPoole);
    free(discovery.ipBowman);

    freeFrame(frame);

    exit(0);
}

int initPooleSocket(){
    printf("Port: %d\n", discovery.portPoole);

    uint16_t port;
    int aux = discovery.portPoole;
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
    return sockfd;
}

int initBowmanSocket(){
    printf("Port: %d\n", discovery.portBowman);

    uint16_t port;
    int aux = discovery.portBowman;
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
    return sockfd;
}

void discoveryMenu(int sockfd){
    freeFrame(frame);
    frame = receiveMessage(sockfd);

    printf("\nType: %d\n", frame.type);
    printf("Header: %s\n", frame.header);
    printf("Data: %s\n\n", frame.data);

    if(strcmp(frame.header, HEADER_NEW_POOLE) == 0){
    PooleServer *newServer;
    newServer = malloc(sizeof(PooleServer));
    
    int i = 0;
    char *info[3];
    char *token = strtok(frame.data, "&");

    while (token != NULL) {
        info[i] = token;
        i++;
        token = strtok(NULL, "&");
    }

    newServer->name = strdup(info[0]);
    newServer->ip = strdup(info[1]);
    newServer->port = atoi(info[2]);
    newServer->connnections = 0;

    servers = realloc(servers, sizeof(PooleServer) * (num_servers + 1));
    servers[num_servers] = newServer;
    num_servers++;

    sendMessage(sockfd, 0x01, strlen(HEADER_CON_OK), HEADER_CON_OK, "");
    printf("New poole server added\n");

    }else if(strcmp(frame.header, HEADER_NEW_BOWMAN) == 0){

        int min = servers[0]->connnections;
        int index = 0;

        for(int i=0; i<num_servers; i++){
            if(servers[i]->connnections < min){
                min = servers[i]->connnections;
                index = i;
            }
        }

        servers[index]->connnections++;
        char *data;
        asprintf(&data, "%s&%s&%d", servers[index]->name, servers[index]->ip, servers[index]->port);
        //printf("Sending %s to bowman\n", data);
        sendMessage(sockfd, 0x01, strlen(HEADER_CON_OK), HEADER_CON_OK, data);
        free(data);

    }else if(strcmp (frame.header, HEADER_EXIT) == 0){
        for(int i = 0; i < num_servers; i++){
            if(strcmp(servers[i]->name, frame.data) == 0){
                servers[i]->connnections--;
                break;
            }
        }

        sendMessage(sockfd, 0x06, strlen(HEADER_OK_DISCONNECT), HEADER_OK_DISCONNECT, "");

        FD_CLR(sockfd, &rfds);
        close(sockfd);
        printf("Bowman disconnected\n");
    }
}

void discoveryServer() {
    pooleSockfd = initPooleSocket();
    bowmanSockfd = initBowmanSocket();

    FD_ZERO(&rfds);
    FD_SET(pooleSockfd, &rfds);
    FD_SET(bowmanSockfd, &rfds);

    printf("Poole socket: %d\n", pooleSockfd);
    printf("Bowman socket: %d\n", bowmanSockfd);

    while (1) {
        struct sockaddr_in c_addr;
        socklen_t c_len = sizeof(c_addr);

        fd_set tmp_fds = rfds;  // Copy the set for select

        select(512, &tmp_fds, NULL, NULL, NULL);

        for (int i = 0; i < 512; i++) {
            if (FD_ISSET(i, &tmp_fds)) {
                if (i == pooleSockfd || i == bowmanSockfd) {
                    printf("New connection\n");
                    int newsock = accept(i, (void *)&c_addr, &c_len);

                    printf("New client with socket %d\n", newsock);
                    if (newsock < 0) {
                        perror("accept");
                        exit(EXIT_FAILURE);
                    }

                    clients = realloc(clients, sizeof(int) * (num_clients + 1));
                    clients[num_clients] = newsock;
                    num_clients++;
                    FD_SET(newsock, &rfds);

                    if (i == pooleSockfd) {
                        printf("Poole connected\n");
                    } else {
                        printf("Bowman connected\n");
                    }
                } else {
                    printf("New message from %d\n", i);
                    discoveryMenu(i);
                }
            }
        }
    }
}

int main(int argc, char *argv[]){
    char *buffer;
    char *line;

    signal(SIGINT, ksigint);

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

    discoveryServer();

    return 0;
}