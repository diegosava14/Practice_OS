#include "common.h"

typedef struct{
    char *name;
    char *folder;
    char *ip;
    int port;
}Bowman;

typedef struct{
    char *name;
    char *ip;
    int port;
}PooleToConnect; 

Frame frameTranslation_CON_OK_Discovery(char message[256]){
    Frame frame;

    frame.type = message[0];

    frame.headerLength = (message[2] << 8) | message[1];

    frame.header = malloc(frame.headerLength + 1);
    strncpy(frame.header, &message[3], frame.headerLength);
    frame.header[frame.headerLength] = '\0';

    frame.data = strdup(&message[3 + frame.headerLength]);

    return frame;
}

Frame receiveMessage_CON_OK_Discovery(int sockfd){
    char message[256];
    int size = read(sockfd, message, 256);
    if(size == 0){
        Frame frame;
        frame.type = 0;
        return frame;
    }
    return frameTranslation_CON_OK_Discovery(message);
}

void ksigint(){
    write(1, "\n", 1);
    exit(EXIT_SUCCESS);
}

PooleToConnect connectToDiscovery(Bowman bowman){
    PooleToConnect pooleToConnect;

    uint16_t port;
    int aux = bowman.port;
    if (aux < 1 || aux > 65535)
    {
        perror ("port");
        exit (EXIT_FAILURE);
    }
    port = aux;

    struct in_addr ip_addr;
    if (inet_aton (bowman.ip, &ip_addr) == 0)
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
    asprintf(&data, "%s", bowman.name);
    sendMessage(sockfd, 0x01, strlen(HEADER_NEW_BOWMAN), HEADER_NEW_BOWMAN, data);
    free(data);

    Frame frame = receiveMessage_CON_OK_Discovery(sockfd);
    if(strcmp(frame.header, HEADER_CON_OK) == 0){
        printf("Connection accepted\n");
        close(sockfd);
    }else{
        printf("Connection refused\n");
        close(sockfd);
    }

    int i = 0;
    char *info[3];
    char *token = strtok(frame.data, "&");

    while (token != NULL) {
        info[i] = token;
        i++;
        token = strtok(NULL, "&");
    }

    pooleToConnect.name = strdup(info[0]);
    pooleToConnect.ip = strdup(info[1]);
    pooleToConnect.port = atoi(info[2]);

    return pooleToConnect;
}

void main_menu(Bowman bowman){
    int connected = 0;
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
    Bowman bowman;

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

    PooleToConnect pooleToConnect = connectToDiscovery(bowman);

    printf("%s\n", pooleToConnect.name);
    printf("%s\n", pooleToConnect.ip);
    printf("%d\n", pooleToConnect.port);

    //main_menu(bowman);
    return 0;
}