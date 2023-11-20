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

PooleServer **servers;
int num_servers = 0;

void pooleMenu(Frame frame, int sockfd){
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

        printf("New poole server: %s %s %d\n", newServer->name, newServer->ip, newServer->port);
        sendMessage(sockfd, 0x01, strlen(HEADER_CON_OK), HEADER_CON_OK, "");
    }
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

    while(1){
        struct sockaddr_in c_addr;
        socklen_t c_len = sizeof(c_addr);

        printf("Waiting for connections on port %hu\n", ntohs(s_addr.sin_port));
        int newsock = accept(sockfd, (void *)&c_addr, &c_len);
        if (newsock < 0) {
            perror("accept");
            exit(EXIT_FAILURE);
        }

        printf("New connection from %s:%hu\n", inet_ntoa(c_addr.sin_addr), ntohs(c_addr.sin_port));
        
        Frame frame = receiveMessage(newsock);
        
        pooleMenu(frame, newsock);

        close(newsock);
    }

    close(sockfd);

    return NULL;
}

void bowmanMenu(Frame frame, int sockfd){
    if(strcmp(frame.header, HEADER_NEW_BOWMAN) == 0){
        printf("%d\n", frame.type);
        printf("%d\n", frame.headerLength);
        printf("%s\n", frame.header);
        printf("%s\n", frame.data);

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
        printf("Sending %s to bowman\n", data);
        sendMessage(sockfd, 0x01, strlen(HEADER_CON_OK), HEADER_CON_OK, data);
        free(data);
    }
}

void *discovery_bowman(void *arg){
    Discovery *discovery = (Discovery*)arg;
    
    uint16_t port;
    int aux = discovery->portBowman;
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

    while(1){
        struct sockaddr_in c_addr;
        socklen_t c_len = sizeof(c_addr);

        printf("Waiting for connections on port %hu\n", ntohs(s_addr.sin_port));
        int newsock = accept(sockfd, (void *)&c_addr, &c_len);
        if (newsock < 0) {
            perror("accept");
            exit(EXIT_FAILURE);
        }

        printf("New connection from %s:%hu\n", inet_ntoa(c_addr.sin_addr), ntohs(c_addr.sin_port));
        
        Frame frame = receiveMessage(newsock);

        bowmanMenu(frame, newsock);

        close(newsock);
    }

    close(sockfd);

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