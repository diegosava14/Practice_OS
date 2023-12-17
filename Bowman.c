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

typedef struct {
    int socket_fd;         
    char file_name[256];    
    int totalFileSize;   
} DownloadArgs;

Frame frame;
Bowman bowman;
PooleToConnect pooleToConnect;
int discoverySockfd, pooleSockfd;

void ksigint(){
    write(1, "\n", 1);

    free(bowman.name);
    free(bowman.folder);
    free(bowman.ip);

    free(pooleToConnect.name);
    free(pooleToConnect.ip);

    close(discoverySockfd);
    close(pooleSockfd);

    freeFrame(frame);

    exit(EXIT_SUCCESS);
}

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

PooleToConnect connectToDiscovery(){
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
    discoverySockfd = socket (AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (discoverySockfd < 0)
    {
        perror ("socket TCP");
        exit (EXIT_FAILURE);
    }

    struct sockaddr_in s_addr;
    bzero (&s_addr, sizeof (s_addr));
    s_addr.sin_family = AF_INET;
    s_addr.sin_port = htons (port);
    s_addr.sin_addr = ip_addr;

    if (connect (discoverySockfd, (void *) &s_addr, sizeof (s_addr)) < 0)
    {
        perror ("connect");
        exit (EXIT_FAILURE);
    }
 
    char *data;
    asprintf(&data, "%s", bowman.name);
    sendMessage(discoverySockfd, 0x01, strlen(HEADER_NEW_BOWMAN), HEADER_NEW_BOWMAN, data);
    free(data);

    freeFrame(frame);
    frame = receiveMessage_CON_OK_Discovery(discoverySockfd);
    if(strcmp(frame.header, HEADER_CON_OK) != 0){
        perror("Connection refused");
        ksigint();
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

int connectToPoole(PooleToConnect poole){
    uint16_t port;
    int aux = poole.port;
    if (aux < 1 || aux > 65535)
    {
        perror ("port");
        exit (EXIT_FAILURE);
    }
    port = aux;

    struct in_addr ip_addr;
    if (inet_aton (poole.ip, &ip_addr) == 0)
    {
        perror ("inet_aton");
        exit (EXIT_FAILURE);
    }


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

    if (connect (sockfd, (void *) &s_addr, sizeof (s_addr)) < 0)
    {
        perror ("connect");
        exit (EXIT_FAILURE);
    }

    return sockfd;
}

void logout(){
    sendMessage(pooleSockfd, 0x06, strlen(HEADER_EXIT), HEADER_EXIT, bowman.name);

    freeFrame(frame);
    frame = receiveMessage(pooleSockfd);
    if (strcmp(frame.header, HEADER_OK_DISCONNECT) != 0){
        perror("Error disconnecting from poole\n");
        ksigint();
    }
    
    sendMessage(discoverySockfd, 0x06, strlen(HEADER_EXIT), HEADER_EXIT, pooleToConnect.name);

    freeFrame(frame);
    frame = receiveMessage(discoverySockfd);
    if (strcmp(frame.header, HEADER_OK_DISCONNECT) != 0){
        perror("Error disconnecting from discovery\n");
        ksigint();
    }
}

void listSongs(){
    char *buffer; 

    freeFrame(frame);
    frame = receiveMessage(pooleSockfd);
    if(strcasecmp(frame.header, HEADER_SONGS_RESPONSE) != 0){
        perror("Error receiving songs\n");
        ksigint();
    }

    int info[2];
    int i = 0;

    char *token = strtok(frame.data, "&");
    while (token != NULL) {
        info[i] = atoi(token);
        i++;
        token = strtok(NULL, "&");
    }

    sendMessage(pooleSockfd, 0x02, strlen(HEADER_ACK), HEADER_ACK, "");

    asprintf(&buffer, "There are %d songs available for download:\n", info[1]);
    write(1, buffer, strlen(buffer));
    free(buffer);

    int num_Frames = info[0];
    int song_count = 0;

    for(int i = 0; i < num_Frames; i++){
        freeFrame(frame);
        frame = receiveMessage(pooleSockfd);

        int y = 0;
        char *token = strtok(frame.data, "&");

        while (token != NULL) {
            asprintf(&buffer, "%d. %s\n", song_count+1, token);
            write(1, buffer, strlen(buffer));
            free(buffer);
            song_count++;
            y++;
            token = strtok(NULL, "&");
        }

        sendMessage(pooleSockfd, 0x02, strlen(HEADER_ACK), HEADER_ACK, "");
    }
}

void listPlaylists(){
    char *buffer; 
    sendMessage(pooleSockfd, 0x02, strlen(HEADER_LIST_PLAYLISTS), HEADER_LIST_PLAYLISTS, bowman.name);

    freeFrame(frame);
    frame = receiveMessage(pooleSockfd);
    if(strcasecmp(frame.header, HEADER_PLAYLISTS_RESPONSE) != 0){
        perror("Error receiving playlists\n");
        ksigint();
    }

    int num_playlists = atoi(frame.data);

    asprintf(&buffer, "There are %d lists available for download:\n", num_playlists);
    write(1, buffer, strlen(buffer));
    free(buffer);

    sendMessage(pooleSockfd, 0x02, strlen(HEADER_ACK), HEADER_ACK, "");

    for(int i=0; i<num_playlists; i++){
        freeFrame(frame);
        frame = receiveMessage(pooleSockfd);
        if(strcasecmp(frame.header, HEADER_PLAYLISTS_RESPONSE) != 0){
            perror("Error receiving playlists\n");
            ksigint();
        }

        asprintf(&buffer, "%d. %s\n", i+1, frame.data);
        write(1, buffer, strlen(buffer));
        free(buffer);

        sendMessage(pooleSockfd, 0x02, strlen(HEADER_ACK), HEADER_ACK, "");

        freeFrame(frame);
        frame = receiveMessage(pooleSockfd);
        if(strcasecmp(frame.header, HEADER_SONGS_RESPONSE) != 0){
            perror("Error receiving playlists\n");
            ksigint();
        }

        int info[2];
        int i = 0;

        char *token = strtok(frame.data, "&");
        while (token != NULL) {
            info[i] = atoi(token);
            i++;
            token = strtok(NULL, "&");
        }

        sendMessage(pooleSockfd, 0x02, strlen(HEADER_ACK), HEADER_ACK, "");

        int num_Frames = info[0];
        char song_count = 'a';

        for(int i = 0; i < num_Frames; i++){
            freeFrame(frame);
            frame = receiveMessage(pooleSockfd);

            int y = 0;
            char *token = strtok(frame.data, "&");

            while (token != NULL) {
                asprintf(&buffer, "\t%c. %s\n", song_count, token);
                write(1, buffer, strlen(buffer));
                free(buffer);
                song_count++;
                y++;
                token = strtok(NULL, "&");
            }

            sendMessage(pooleSockfd, 0x02, strlen(HEADER_ACK), HEADER_ACK, "");
        }
    }
}


// void update_progress_bar(int bytesReceived, int totalFileSize) {
//     int percentage = (100 * bytesReceived) / totalFileSize;
//     char progressBar[51];
//     memset(progressBar, '=', percentage / 2);
//     progressBar[percentage / 2] = '\0';
//     char output[100];
//     snprintf(output, sizeof(output), "\r[%-50s] %d%%", progressBar, percentage);
//     write(STDOUT_FILENO, output, strlen(output)); // Use write() to STDOUT
// }


void download(){

    freeFrame(frame);
    frame = receiveMessage(pooleSockfd);

    if (strcmp(frame.header, HEADER_FILE_NOT_FOUND) == 0) {
        write(1,frame.data, strlen(frame.data));
        return;
    }

    if(strcasecmp(frame.header, HEADER_NEW_FILE) != 0){
        perror("Error receiving file\n");
        ksigint();
    } 
    //printf("Frame: %s\n", frame.data);

    int i = 0;
    char *info[4];
    char *token = strtok(frame.data, "&");

    while (token != NULL) {
        info[i] = token;
        i++;
        token = strtok(NULL, "&");
    }

    char *file_name = info[0];
    int file_size = atoi(info[1]);
    char *MD5SUM = info[2];
    int id = atoi(info[3]);

    printf("File name: %s\n", file_name);
    printf("File size: %d\n", file_size);
    printf("MD5SUM: %s\n", MD5SUM);
    printf("ID: %d\n", id);


    // while (1) {
    //     frame = receiveMessage(pooleSockfd);
    //     printf("Frame: %s\n", frame.data);

    //     if (read(pooleSockfd, frame.data, 256) == 0) {
    //         break;
    //     }
    // }


}



void main_menu(){
    int run = 1; 
    int connected = 0;
    int inputLength;
    char *printBuffer;
    char buffer[200];

    while (run) {
        int wordCount = 0;
        int spaceCount = 0;
        write(1, "\n$ ", 2);

        inputLength = read(STDIN_FILENO, buffer, 200);
        buffer[inputLength - 1] = '\0';

        if (inputLength > 0) {
            for (int j = 0; j < inputLength; j++) {
                if (buffer[j] == ' ') {
                    spaceCount++;
                }
            } 
        }

        char *input[spaceCount + 1];    

        if (strcasestr(buffer, "DOWNLOAD") == NULL) { 
            char *token = strtok(buffer, " \t");
            while (token != NULL && wordCount < spaceCount + 1) {
                input[wordCount] = token;
                token = strtok(NULL, " \t");
                wordCount++;
            }
        } else {
            spaceCount = 1;
            input[0] = malloc(sizeof(char) * 9);
            input[1] = malloc(sizeof(char) * (inputLength - 9));
            strcpy(input[0], "DOWNLOAD");
            strcpy(input[1], buffer + 9);
        }


        if((strcasecmp(input[0], OPT_CONNECT) == 0)&&(wordCount == 1)){
            if(connected){
                asprintf(&printBuffer, "%s is already connected.\n", bowman.name); //ERROR: Bowman name does not show
                write(1, printBuffer, strlen(printBuffer));
                free(printBuffer);
            }else{
                connected = 1;
                pooleSockfd = connectToPoole(pooleToConnect);

                sendMessage(pooleSockfd, 0x01, strlen(HEADER_NEW_BOWMAN), HEADER_NEW_BOWMAN, bowman.name);

                asprintf(&printBuffer, "%s connected to HAL 9000 system, welcome music lover!\n", bowman.name);
                write(1, printBuffer, strlen(printBuffer));
                free(printBuffer);
            }
        }

        else if((strcasecmp(input[0], OPT_LOGOUT) == 0)&&(wordCount == 1)){
            if(connected){
                asprintf(&printBuffer, "Thanks for using HAL 9000, see you soon, music lover!\n");
                write(1, printBuffer, strlen(printBuffer));
                free(printBuffer);

                logout();
                run = 0;
                connected = 0;
                ksigint();
            }else{
                asprintf(&printBuffer, "Cannot Logout, you are not connected to HAL 9000\n");
                write(1, printBuffer, strlen(printBuffer));
                free(printBuffer);
            }
        }

        else if((strcasecmp(input[0], OPT_LIST_SONGS1) == 0)&&(strcasecmp(input[1], OPT_LIST_SONGS2) == 0)
        &&(wordCount == 2)){
            if(connected){
                sendMessage(pooleSockfd, 0x02, strlen(HEADER_LIST_SONGS), HEADER_LIST_SONGS, bowman.name);
                listSongs();
            }else{
                asprintf(&printBuffer, "Cannot List Songs, you are not connected to HAL 9000\n");
                write(1, printBuffer, strlen(printBuffer));
                free(printBuffer);
            }
        }

        else if((strcasecmp(input[0], OPT_LIST_PLAYLISTS1) == 0)&&(strcasecmp(input[1], OPT_LIST_PLAYLISTS2) == 0)
        &&(wordCount == 2)){
            if(connected){
                listPlaylists();
            }else{
                asprintf(&printBuffer, "Cannot List Playlist, you are not connected to HAL 9000\n");
                write(1, printBuffer, strlen(printBuffer));
                free(printBuffer);
            }
        }

        else if((strcasecmp(input[0], OPT_DOWNLOAD) == 0)){
            if(connected){

                if (strstr(input[1], ".mp3") != NULL) {
                    printf("Downloading %s\n", input[1]);
                    sendMessage(pooleSockfd, 0x03, strlen(HEADER_DOWNLOAD_SONG), HEADER_DOWNLOAD_SONG, input[1]);

                    download();
                } else {
                    //download playlist
                    // sendMessage(pooleSockfd, 0x03, strlen(HEADER_DOWNLOAD_PLAYLIST), HEADER_DOWNLOAD_PLAYLIST, input[1]);
                    // printf("Downloading %s playlist\n", input[1]);

                }

                
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

    pooleToConnect = connectToDiscovery();

    main_menu();
    return 0;
}