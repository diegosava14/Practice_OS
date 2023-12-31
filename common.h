#ifndef _COMMON_H_
#define _COMMON_H_

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
#include <dirent.h>
#include <stdbool.h>
#include <sys/stat.h> 
#include <sys/types.h> 
#include <errno.h>

#define OPT_CONNECT "CONNECT"
#define OPT_CHECK_DOWNLOADS1 "CHECK"
#define OPT_CHECK_DOWNLOADS2 "DOWNLOADS"
#define OPT_LIST_SONGS1 "LIST"
#define OPT_LIST_SONGS2 "SONGS"
#define OPT_DOWNLOAD "DOWNLOAD"
#define OPT_LIST_PLAYLISTS1 "LIST"
#define OPT_LIST_PLAYLISTS2 "PLAYLISTS"
#define OPT_LOGOUT "LOGOUT"
#define OPT_CLEAR_DOWNLOADS1 "CLEAR"
#define OPT_CLEAR_DOWNLOADS2 "DOWNLOADS"

#define HEADER_ACK "ACK"
#define HEADER_NEW_POOLE "NEW_POOLE"
#define HEADER_NEW_BOWMAN "NEW_BOWMAN"
#define HEADER_CON_OK "CON_OK"
#define HEADER_CON_KO "CON_KO"
#define HEADER_EXIT "EXIT"
#define HEADER_OK_DISCONNECT "CONOK"

#define HEADER_LIST_SONGS "LIST_SONGS"
#define HEADER_SONGS_RESPONSE "SONGS_RESPONSE"
#define HEADER_LIST_PLAYLISTS "LIST_PLAYLISTS" 
#define HEADER_PLAYLISTS_RESPONSE "PLAYLISTS_RESPONSE"
#define HEADER_DOWNLOAD_SONG "DOWNLOAD_SONG"
#define HEADER_DOWNLOAD_PLAYLIST "DOWNLOAD_PLAYLIST"
#define HEADER_NEW_FILE "NEW_FILE"
#define HEADER_FILE_NOT_FOUND "FILE_NOT_FOUND"
#define HEADER_FILE_DATA "FILE_DATA"

#define HEADER_CHECK_OK "CHECK_OK"
#define HEADER_CHECK_KO "CHECK_KO"

typedef struct{
    uint8_t type;
    uint16_t headerLength;
    char *header;
    char *data;
}Frame; 

char *read_until(int fd, char end);

void sendMessage(int sockfd, uint8_t type, uint16_t headerLength, const char *constantHeader, char *data);

Frame frameTranslation(char message[256]);

Frame receiveMessage(int sockfd);

void freeFrame(Frame frame);

int calculate_md5sum(const char *file_path, char *md5sum);

#endif