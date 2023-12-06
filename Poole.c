#include "common.h"

typedef struct{
    char *nameServer;
    char *folder;
    char *ipDiscovery;
    int portDiscovery;
    char *ipPoole;
    int portPoole;
}Poole;

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
        //close(sockfd);
    }else{
        printf("Connection refused\n");
        //close(sockfd);
    }
}

void handleFrames(Frame frame, int sockfd){
    sockfd++;
    sockfd--;
    char *buffer;

    printf("Header: %s\n", frame.header);
    printf("Data: %s\n", frame.data);

    if (strcmp(frame.header, HEADER_NEW_BOWMAN) == 0) {
        asprintf(&buffer, "New user connected: %s", frame.data);
        write(1, buffer, strlen(buffer));
        free(buffer);

    } else if(strcmp(frame.header, HEADER_LIST_SONGS) == 0){

        asprintf(&buffer, "New request - %s requires the list of songs.", frame.data);
        write(1, buffer, strlen(buffer));
        free(buffer);
        asprintf(&buffer, "Sending song list to %s", frame.data);
        write(1, buffer, strlen(buffer));
        free(buffer);

    } else if(strcmp(frame.header, HEADER_LIST_PLAYLISTS) == 0){
        asprintf(&buffer, "New request - %s requires the list of playlists.", frame.data);
        write(1, buffer, strlen(buffer));
        free(buffer);
        asprintf(&buffer, "Sending playlist list to %s", frame.data);
        write(1, buffer, strlen(buffer));
        free(buffer);

    }
}

void pooleServer(Poole poole){
    char *buffer;
    fd_set rfds;

    uint16_t port;
    int aux = poole.portPoole;
    if (aux < 1 || aux > 65535)
    {
        perror ("Error: invalid port");
        exit (EXIT_FAILURE);
    }
    port = aux;

    // Create the socket
    int sockfd;
    sockfd = socket (AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sockfd < 0)
    {
        perror ("socket TCP");
        exit (EXIT_FAILURE);
    }

    // Specify the adress and port of the socket
    // We'll admit connexions to any IP of our machine in the specified port
    struct sockaddr_in s_addr;
    bzero (&s_addr, sizeof (s_addr));
    s_addr.sin_family = AF_INET;
    s_addr.sin_port = htons (port);
    s_addr.sin_addr.s_addr = INADDR_ANY;

    // When executing bind, we should add a cast:
    // bind waits for a struct sockaddr* and we are passing a struct sockaddr_in*
    if (bind (sockfd, (void *) &s_addr, sizeof (s_addr)) < 0)
    {
        perror ("bind");
        exit (EXIT_FAILURE);
    }

    // We now open the port (5 backlog queue, typical value)
    listen (sockfd, 5);

    FD_ZERO(&rfds);
    FD_SET(sockfd, &rfds);

    asprintf(&buffer, "Waiting for connections...\n");
    write(1, buffer, strlen(buffer));
    free(buffer);

    while(1){
        asprintf(&buffer, "We are waiting for a message or customer.\n");
        write(1, buffer, strlen(buffer));
        free(buffer);

        struct sockaddr_in c_addr;
        socklen_t c_len = sizeof (c_addr);

        select(512, &rfds, NULL, NULL, NULL);

        for(int i=0; i<512; i++){
            if(FD_ISSET(i, &rfds)){
                if (i == sockfd){                 
                    int newsock = accept (sockfd, (void *) &c_addr, &c_len);

                    if (newsock < 0)
                    {
                        perror ("accept");
                        exit (EXIT_FAILURE);
                    }
                    FD_SET(newsock, &rfds);
                }else{
                    //handle bowman frames, menu(i) i is the fd of the bowman
                    Frame frame = receiveMessage(i);
                    handleFrames(frame, i);
                }
            }
        }
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

    pooleServer(poole);

    return 0;
}
