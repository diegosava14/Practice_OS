#include "common.h"

typedef struct{
    char *nameServer;
    char *folder;
    char *ipDiscovery;
    int portDiscovery;
    char *ipPoole;
    int portPoole;
}Poole;

typedef struct{
    int sockfd;
    char *file_path;
    int id;
} DownloadThread;

Frame frame;
Poole poole;
int discoverySockfd, serverSockfd;
int *clients;
int num_clients = 0; 
fd_set rfds;

void ksigint(){
    char *buffer;
    asprintf(&buffer, "SIGINT received. Closing server...\n");
    write(1, buffer, strlen(buffer));
    free(buffer);

    FD_ZERO(&rfds);

    free(poole.nameServer);
    free(poole.folder);
    free(poole.ipDiscovery);
    free(poole.ipPoole);

    for (int i = 0; i < num_clients; i++)
    {
        close(clients[i]);
    }
    
    free(clients);

    close(discoverySockfd);
    close(serverSockfd);

    freeFrame(frame);

    exit(0);
}

///Quick functionalities start///
void listSongs(char *desired_path, int sockfd){
    char *buffer;
    DIR *dir;
    struct dirent *ent;

    char **toSend = NULL; 
    int num_toSend = 0;
    char **frameData = NULL;
    int num_frames = 0;

    const char *path = desired_path;

    dir = opendir(path);

    if (dir != NULL) {
        while ((ent = readdir(dir)) != NULL) {
            char *song_name = strdup(ent->d_name);
            if(strcmp(song_name, ".") != 0 && strcmp(song_name, "..") != 0){
                toSend = realloc(toSend, sizeof(char *) * (num_toSend + 1));
                toSend[num_toSend] = strdup(song_name);
                num_toSend++;
            }
            free(song_name);
        }

        closedir(dir);
    } else {
        perror("Unable to open directory");
    }

    int total_size = 0;
    size_t available = 256 - (3 + strlen(HEADER_SONGS_RESPONSE) + 1);

    for(int i=0; i<num_toSend; i++){
        total_size += strlen(toSend[i])+1;
    }

    num_frames = (int)(total_size / available) + 1;
    frameData = malloc(sizeof(char *) * num_frames);
    size_t current = available; 
    int index = 0;

    for(int i=0; i<num_toSend; i++){
        if(strlen(toSend[i])+1 < current){
            if(current == available){
                //first one
                current -= strlen(toSend[i]);
                asprintf(&buffer, "%s", toSend[i]);
            }else if(i == num_toSend - 1){
                //last one
                int bufferSize = strlen(buffer); 
                char currentBuffer[bufferSize];
                strcpy(currentBuffer, buffer);
                free(buffer);

                asprintf(&buffer, "%s&%s", currentBuffer, toSend[i]);
                frameData[index] = strdup(buffer);
            }else{
                //general
                current -= strlen(toSend[i])+1;

                int bufferSize = strlen(buffer); 
                char currentBuffer[bufferSize];
                strcpy(currentBuffer, buffer);
                free(buffer);

                asprintf(&buffer, "%s&%s", currentBuffer, toSend[i]);
            }
        }else{
            //doesn't fit in current frame
            if(i == num_toSend - 1){
                frameData[index] = strdup(buffer);
                index++;
                current = available - strlen(toSend[i]);

                free(buffer);
                asprintf(&buffer, "%s", toSend[i]);
                frameData[index] = strdup(buffer);

            }else{
                frameData[index] = strdup(buffer);
                index++;
                current = available - strlen(toSend[i]);

                free(buffer);
                asprintf(&buffer, "%s", toSend[i]);
            }
        }
    }

    free(buffer);

    char *data; 
    asprintf(&data, "%d&%d", num_frames, num_toSend);
    sendMessage(sockfd, 0x02, strlen(HEADER_SONGS_RESPONSE), HEADER_SONGS_RESPONSE, data);
    free(data);

    freeFrame(frame);
    frame = receiveMessage(sockfd);
    if(strcasecmp(frame.header, HEADER_ACK) != 0){
        perror("Error receiving message");
        ksigint();
    }

    for(int i=0; i<num_frames; i++){
        sendMessage(sockfd, 0x02, strlen(HEADER_SONGS_RESPONSE), HEADER_SONGS_RESPONSE, frameData[i]);
        freeFrame(frame);
        frame = receiveMessage(sockfd);
        if(strcasecmp(frame.header, HEADER_ACK) != 0){
            perror("Error receiving message");
            ksigint();
        }
    }

    for(int i=0; i<num_toSend; i++){
        free(toSend[i]);
    }

    for (int i = 0; i < num_frames; i++)
    {
        free(frameData[i]);
    }
    
    free(toSend);
    free(frameData);

    num_toSend = 0;
    num_frames = 0;
}

void listPlaylists(int sockfd){
    char *buffer;
    DIR *dir;
    struct dirent *ent;

    char **toSend = NULL; 
    int num_toSend = 0;

    asprintf(&buffer, "%s/playlists", poole.folder);
    const char *path = buffer;

    dir = opendir(path);

    if (dir != NULL) {
        while ((ent = readdir(dir)) != NULL) {
            char *song_name = strdup(ent->d_name);
            if(strcmp(song_name, ".") != 0 && strcmp(song_name, "..") != 0){
                toSend = realloc(toSend, sizeof(char *) * (num_toSend + 1));
                toSend[num_toSend] = strdup(song_name);
                num_toSend++;
            }
            free(song_name);
        }

        closedir(dir);
    } else {
        perror("Unable to open directory");
    }

    free(buffer);

    char *data; 
    asprintf(&data, "%d", num_toSend);
    sendMessage(sockfd, 0x02, strlen(HEADER_PLAYLISTS_RESPONSE), HEADER_PLAYLISTS_RESPONSE, data);
    free(data);

    freeFrame(frame);
    frame = receiveMessage(sockfd);
    if(strcasecmp(frame.header, HEADER_ACK) != 0){
        perror("Error receiving message");
        ksigint();
    }

    for(int i=0; i<num_toSend; i++){
        sendMessage(sockfd, 0x02, strlen(HEADER_PLAYLISTS_RESPONSE), HEADER_PLAYLISTS_RESPONSE, toSend[i]);
        freeFrame(frame);
        frame = receiveMessage(sockfd);
        if(strcasecmp(frame.header, HEADER_ACK) != 0){
            perror("Error receiving message");
            ksigint();
        }

        char *desired_path;
        asprintf(&desired_path, "%s/playlists/%s", poole.folder, toSend[i]);
        listSongs(desired_path, sockfd);
        free(desired_path);
    }

    for(int i=0; i<num_toSend; i++){
        free(toSend[i]);
    }
    
    free(toSend);

    num_toSend = 0;
}


void removeClient(int sockfd) {
    int index = -1;

    for (int i = 0; i < num_clients; i++) {
        if (clients[i] == sockfd) {
            index = i;
            break;
        }
    }

    if (index != -1) {
        for (int i = index; i < num_clients - 1; i++) {
            clients[i] = clients[i + 1];
        }

        num_clients--;
        clients = realloc(clients, sizeof(int) * num_clients);
    }
    FD_CLR(sockfd, &rfds);
}

///Quick functionalities end///


/// File sending functionalities///
void* downloadThread(void* args){
    DownloadThread *arguments = (DownloadThread *) args;
    char *file_path = arguments->file_path;
    int id = arguments->id;

    int fd = open(file_path, O_RDONLY);
    if (fd == -1){
        perror("Error opening file");
        ksigint();
    }

    ///File reading and message sending///
    ssize_t bytesRead;
    int id_net = htonl(id);
    char ampersand = '&';
    char dataBuffer[256 - (3 + strlen(HEADER_FILE_DATA) + 1 + sizeof(id_net) + 1)]; 
    char message[256];

    while ((bytesRead = read(fd, dataBuffer, sizeof(dataBuffer))) > 0) {
        uint8_t type = 0x04;
        
        if (bytesRead == -1){
            perror("Error reading file");
            ksigint();
        }

        int headerLength = strlen(HEADER_FILE_DATA); 

        message[0] = type;
        message[1] = headerLength & 0xFF;
        message[2] = (headerLength >> 8) & 0xFF;

        memcpy(&message[3], HEADER_FILE_DATA, strlen(HEADER_FILE_DATA));
        message[3 + strlen(HEADER_FILE_DATA)] = '\0';

        memcpy(&message[3 + strlen(HEADER_FILE_DATA) + 1], &id_net, sizeof(id_net));
        // printf("id_net: %d\n", id_net);
        memcpy(&message[3 + strlen(HEADER_FILE_DATA) + 1 + sizeof(id_net)], &ampersand, 1);
        memcpy(&message[3 + strlen(HEADER_FILE_DATA) + 1 + sizeof(id_net) + 1], dataBuffer, bytesRead);

        size_t totalLength = 3 + strlen(HEADER_FILE_DATA) + 1 + sizeof(id_net) + 1 + bytesRead;
        while (totalLength < 256) {
            message[totalLength] = '\0';
            totalLength++;
        }
        write(arguments->sockfd, message, totalLength);

        // printf("\nRaw message data: ");
        // for (long unsigned int i = 0; i < totalLength; i++){
        //     printf("%02x ", (unsigned char)message[i]);
        // }
        // printf("\n");
    }

   ///File reading and message sending///

    Frame recieveCheck = receiveMessage(arguments->sockfd);  // (ERRASE LATER) gets confiramtion that the file is recieved well
    char b[256];
    strcpy(b, arguments->file_path);
    if (strcmp(recieveCheck.header, HEADER_CHECK_OK) == 0){
        write(1, "File sent successfully\n", 24);
        write(1, b, strlen(b));
    } else {
        write(1, "Error sending file\n", 20);
        write(1, b, strlen(b));
    }
    write(1, "\n\n", 2);  //Errase later end

    close(fd);
    free(arguments);
    return NULL;    
}


void startSending(int sockfd, char *song_name){

    int id = rand() % 1000;

    char *desired_path;
    asprintf(&desired_path, "%s/songs/%s", poole.folder, song_name);
    int fd = open(desired_path, O_RDONLY);

    ssize_t file_size = lseek(fd, 0, SEEK_END);
    // printf("File size: %ld\n", file_size);
    char *file_size_str;
    asprintf(&file_size_str, "%ld", file_size);

    //md5sum
    char* md5sum = malloc(33 * sizeof(char));
    int md5 = calculate_md5sum(desired_path, md5sum);
    if (md5 == -1){
        perror("Error calculating md5sum");
        ksigint();
    }

    // printf("MD5SUM: %s\n", md5sum);

    char *data;
    asprintf(&data, "%s&%s&%s&%d", song_name, file_size_str, md5sum, id);
    // printf("Data: %s\n", data);

    sendMessage(sockfd, 0x04, strlen(HEADER_NEW_FILE), HEADER_NEW_FILE, data);
        
    free(file_size_str);
    free(data);
    free(md5sum);

    pthread_t thread;

    DownloadThread *args = malloc(sizeof(DownloadThread));

    if (args == NULL){
        perror("Error allocating memory for download thread");
        ksigint();
    }

    args->sockfd = sockfd;
    args->file_path = malloc(sizeof(char) * (strlen(desired_path) + 1));
    strcpy(args->file_path, desired_path);
    args->id = id;

    // printf("Sending first frame: %s\n", args->file_path);

    if (pthread_create(&thread, NULL, downloadThread, args) != 0){
        perror("Error creating download thread");
        free(args);
        ksigint();
    }

    pthread_detach(thread);

}
/// File sending functionalities end///

bool fileCheck(char *song){
    char *desired_path;
    asprintf(&desired_path, "%s/songs/%s", poole.folder, song);
    return access(desired_path, F_OK) == 0;
}

void handleFrames(Frame frame, int sockfd){
    char *buffer;

    if (strcmp(frame.header, HEADER_NEW_BOWMAN) == 0) {
        asprintf(&buffer, "New user connected: %s\n\n", frame.data);
        write(1, buffer, strlen(buffer));
        free(buffer);

    } else if(strcmp(frame.header, HEADER_LIST_SONGS) == 0){
        asprintf(&buffer, "New request - %s requires the list of songs.\n", frame.data);
        write(1, buffer, strlen(buffer));
        free(buffer);
        asprintf(&buffer, "Sending song list to %s\n\n", frame.data);
        write(1, buffer, strlen(buffer));
        free(buffer);

        char *desired_path;
        asprintf(&desired_path, "%s/songs", poole.folder);
        listSongs(desired_path, sockfd);
        free(desired_path);

    } else if(strcmp(frame.header, HEADER_LIST_PLAYLISTS) == 0){
        asprintf(&buffer, "New request - %s requires the list of playlists.\n", frame.data);
        write(1, buffer, strlen(buffer));
        free(buffer);
        asprintf(&buffer, "Sending playlist list to %s\n\n", frame.data);
        write(1, buffer, strlen(buffer));
        free(buffer);

        listPlaylists(sockfd);

    }else if(strcmp(frame.header, HEADER_EXIT) == 0){
        removeClient(sockfd);

        sendMessage(sockfd, 0x06, strlen(HEADER_OK_DISCONNECT), HEADER_OK_DISCONNECT, "");

        close(sockfd);

        asprintf(&buffer, "User %s disconnected.\n\n", frame.data);
        write(1, buffer, strlen(buffer));
        free(buffer);

    } else if (strcmp(frame.header, HEADER_DOWNLOAD_SONG) == 0) {

        asprintf(&buffer, "New request - *user* wants to download %s.\n", frame.data);  //*user* should be asssigned the bowman name
        write(1, buffer, strlen(buffer));
        free(buffer);

        if (!fileCheck(frame.data)) {
            asprintf(&buffer, "No %s song found.\n", frame.data);
            write(1, buffer, strlen(buffer));
            free(buffer);

            asprintf(&buffer, "Please input a valid download.\n");
            sendMessage(sockfd, 0x04, strlen(HEADER_FILE_NOT_FOUND), HEADER_FILE_NOT_FOUND, buffer);
            free(buffer);
        } else {
            asprintf(&buffer, "Sending %s to *user*\n", frame.data); //*user* should be asssigned the bowman name
            startSending(sockfd, frame.data);
        }
        
    }
}


/// Poole setup///
void pooleServer(){

    uint16_t port;
    int aux = poole.portPoole;
    if (aux < 1 || aux > 65535)
    {
        perror ("Error: invalid port");
        exit (EXIT_FAILURE);
    }
    port = aux;

    serverSockfd = socket (AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (serverSockfd < 0)
    {
        perror ("socket TCP");
        exit (EXIT_FAILURE);
    }

    struct sockaddr_in s_addr;
    bzero (&s_addr, sizeof (s_addr));
    s_addr.sin_family = AF_INET;
    s_addr.sin_port = htons (port);
    s_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind (serverSockfd, (void *) &s_addr, sizeof (s_addr)) < 0)
    {
        perror ("bind");
        exit (EXIT_FAILURE);
    }

    listen (serverSockfd, 5);

    FD_ZERO(&rfds);
    FD_SET(serverSockfd, &rfds);

    while(1){
        struct sockaddr_in c_addr;
        socklen_t c_len = sizeof (c_addr);

        select(512, &rfds, NULL, NULL, NULL);

        for(int i=0; i<512; i++){
            if(FD_ISSET(i, &rfds)){
                if (i == serverSockfd){                 
                    int newsock = accept (serverSockfd, (void *) &c_addr, &c_len);

                    if (newsock < 0)
                    {
                        perror ("accept");
                        exit (EXIT_FAILURE);
                    }

                    clients = realloc(clients, sizeof(int) * (num_clients + 1));
                    clients[num_clients] = newsock;
                    num_clients++;
                    FD_SET(newsock, &rfds);
                }else{
                    //handle bowman frames, menu(i) i is the fd of the bowman
                    freeFrame(frame);
                    frame = receiveMessage(i);
                    handleFrames(frame, i);
                }
            }
        }
    }
}


void connectToDiscovery(){
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

    // We can connect to the server casting the struct:
    // connect waits for a struct sockaddr* and we are passing a struct sockaddr_in*
    if (connect (discoverySockfd, (void *) &s_addr, sizeof (s_addr)) < 0)
    {
        perror ("connect");
        exit (EXIT_FAILURE);
    }
 
    char *data;
    asprintf(&data, "%s&%s&%d", poole.nameServer, poole.ipPoole, poole.portPoole);
    sendMessage(discoverySockfd, 0x01, strlen(HEADER_NEW_POOLE), HEADER_NEW_POOLE, data);
    free(data);

    freeFrame(frame);
    frame = receiveMessage(discoverySockfd);
    
    if(strcmp(frame.header, HEADER_CON_OK) != 0){
        perror("Connection refused");
        ksigint();
    }
}

void readPooleFile(char *file){
    char *buffer;
    char *line;
    
    int fd_poole = open(file, O_RDONLY);

    if(fd_poole == -1){
        perror("Error opening poole file");
        exit(EXIT_FAILURE);
    }

    asprintf(&buffer, "Reading configuration file...\n");
    write(1, buffer, strlen(buffer));
    free(buffer);

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
}
/// Poole setup end///


int main(int argc, char *argv[]){
    char *buffer;

    signal(SIGINT, ksigint);

    if (argc != 2) {
        asprintf(&buffer, "ERROR: Expecting one parameter.\n");
        write(1, buffer, strlen(buffer));
        free(buffer);
        return -1;
    }

    //READING POOLE FILE
    readPooleFile(argv[1]);

    //SOCKETS
    asprintf(&buffer, "Connecting to %s to the system...\n", poole.nameServer);
    write(1, buffer, strlen(buffer));
    free(buffer);

    connectToDiscovery();

    asprintf(&buffer, "Connected to HAL 9000 System, ready to listen to Bowmans petitions\n\n");
    write(1, buffer, strlen(buffer));
    free(buffer);

    pooleServer();

    return 0;
}
