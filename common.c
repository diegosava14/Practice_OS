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
    frame.header[frame.headerLength] = '\0';

    if(strcmp(frame.header, HEADER_CON_OK) == 0 || strcmp(frame.header, HEADER_CON_KO) == 0 
        || strcmp(frame.header, HEADER_OK_DISCONNECT) == 0 || strcmp(frame.header, HEADER_ACK) == 0){
        frame.data = NULL;
        return frame;
    }else{
        frame.data = strdup(&message[3 + frame.headerLength]);

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