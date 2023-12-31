#include "common.h"

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
    headerLength++;

    message[0] = type;

    message[1] = headerLength & 0xFF;
    message[2] = (headerLength >> 8) & 0xFF;

    memcpy(&message[3], constantHeader, strlen(constantHeader));
    message[3 + strlen(constantHeader)] = '\0';

    memcpy(&message[3 + strlen(constantHeader) + 1], data, strlen(data));
    message[3 + strlen(constantHeader) + 1 + strlen(data)] = '\0';

    size_t paddingToAdd = 256 - (3 + strlen(constantHeader) + 1 + strlen(data) + 1);
    memset(&message[3 + strlen(constantHeader) + 1 + strlen(data) + 1], 0, paddingToAdd);

    //printf("Total size of message sent: %ld\n", 3+ strlen(constantHeader) + 1 + strlen(data) + 1 + paddingToAdd);
    
    write(sockfd, message, 256);
}

Frame frameTranslation(char message[256]){
    Frame frame;

    frame.type = message[0];

    frame.headerLength = (message[2] << 8) | message[1];

    frame.header = malloc(frame.headerLength + 1);
    strncpy(frame.header, &message[3], frame.headerLength);
    // memcpy(frame.header, &message[3], frame.headerLength);
    // frame.header[frame.headerLength] = '\0';

    if(strcmp(frame.header, HEADER_CON_OK) == 0 || strcmp(frame.header, HEADER_CON_KO) == 0){
        frame.data = NULL;
        return frame;
    }else{
        frame.data = strdup(&message[3 + frame.headerLength]);
        // frame.data = malloc(256 - (3 + frame.headerLength));
        // memcpy(frame.data, &message[3 + frame.headerLength], 256 - (3 + frame.headerLength));
        // frame.data[256 - (3 + frame.headerLength)] = '\0';

        /*
        int dataSize = strlen(&message[3 + frame.headerLength+1]);
        frame.data = malloc(dataSize + 1);
        strncpy(frame.data, &message[3 + frame.headerLength+1], dataSize);
        frame.data[dataSize] = '\0';*/
    }

    return frame;
}

Frame receiveMessage(int sockfd){
    char message[256];
    int size = read(sockfd, message, 256);

    if(size == 0){
        Frame frame;
        frame.type = 0;
frame.headerLength = 0;
        frame.header = "NULL";
        frame.data = "NULL";
        return frame;
    }

    return frameTranslation(message);
}

void freeFrame(Frame frame) {
    if (frame.header != NULL) {
        free(frame.header);
        frame.header = NULL;
    }

    if (frame.data != NULL) {
        free(frame.data);
        frame.data = NULL;
    }
}


int calculate_md5sum(const char *file_path, char *md5sum) {
    int pipefd[2];
    pid_t pid;
    char buf[33];

    if (pipe(pipefd) == -1) {
        perror("pipe");
        return -1;
    }

    pid = fork();
    if (pid == -1) {
        perror("fork");
        return -1;
    }

    if (pid == 0) { // Child process
        close(pipefd[0]); // Close unused read end
        dup2(pipefd[1], STDOUT_FILENO); // Redirect stdout to pipe write end
        close(pipefd[1]); // Close the original write end of the pipe

        execlp("md5sum", "md5sum", file_path, (char *)NULL);
        perror("execlp"); // execlp() only returns on error
        exit(EXIT_FAILURE);
    } else { // Parent process
        close(pipefd[1]); // Close unused write end

        // Read from pipe, get only the MD5 checksum part
        if (read(pipefd[0], buf, 32) > 0) {
            buf[32] = '\0'; // Null-terminate the string
            strcpy(md5sum, buf);
        }
        close(pipefd[0]); // Close read end

        waitpid(pid, NULL, 0); // Wait for child process to finish
    }

    return 0;
}