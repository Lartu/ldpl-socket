#include <stdio.h> 
#include <stdlib.h> 
#include "ldpl-types.h"
#include <sys/socket.h>
#include <sys/select.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>    /* For F_SETFL */

using namespace std;

#define BUF_SIZE 1024
#define FATAL(text) { perror(text); exit(1); }
#define DEBUG(text) { cout << "\e[33;1m" << text << "\e[0m" << endl; }

ldpl_text LDPL_SOCKET_MSG;
ldpl_text LDPL_SOCKET_IP;
ldpl_number LDPL_SOCKET_PORT;
ldpl_number LDPL_SOCKET_NUMBER; 

void socket_connected(unsigned int socket_number, string ip, unsigned int port){
	LDPL_SOCKET_IP = ip;
	LDPL_SOCKET_PORT = port;
	LDPL_SOCKET_NUMBER = socket_number;
}

void socket_closed(unsigned int socket_number){
	LDPL_SOCKET_NUMBER = socket_number;
}

void socket_onmessage(unsigned int socket_number, string message){
	LDPL_SOCKET_NUMBER = socket_number;
	LDPL_SOCKET_MSG = message;
}

void LDPL_SOCKET_CONNECT(){
    const string host = LDPL_SOCKET_IP.str_rep();
    int port = LDPL_SOCKET_PORT;
    int sock;
    if((sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
        FATAL("socket() failed");

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    struct hostent *h = gethostbyname(host.c_str());
    if(h == NULL){
        FATAL("bad hostname");
    }else if(h->h_length <= 0){
        FATAL("gethostbyname() failed");
    }
    char *ip = inet_ntoa(*(struct in_addr*)(h->h_addr_list[0]));
    addr.sin_family      = AF_INET;
    addr.sin_addr.s_addr = inet_addr(ip);
    addr.sin_port        = htons(port);

    int err;
    if((err = connect(sock,(struct sockaddr*)&addr,sizeof(addr))) < 0)
        FATAL("connect() failed");
    if(sock >= 0) {
        socket_connected(sock, host, port);
        LDPL_SOCKET_NUMBER = sock;
    }
}

void LDPL_SOCKET_CLOSE(){
    unsigned int sock = LDPL_SOCKET_NUMBER;
    if(sock < 0) return;
    socket_closed(sock);
    close(sock); 
}

void LDPL_SOCKET_SENDMESSAGE(){
    unsigned int sock = LDPL_SOCKET_NUMBER;
    const string msg = LDPL_SOCKET_MSG.str_rep();

    int sent = 0, bytes = 0;
    while(sent < msg.size()){
        if((bytes = send(sock, msg.c_str(), msg.size(), MSG_DONTWAIT)) < 0)
            FATAL("send() call failed");
        sent += bytes;
    }
    return;
}

void LDPL_SOCKET_READ(){
    int sock = LDPL_SOCKET_NUMBER;
    char buf[BUF_SIZE];
    int bytes = read(sock, buf, BUF_SIZE);
    if(bytes == 0) {
        socket_closed(sock);
        LDPL_SOCKET_MSG = "";
        return;
    }
    buf[bytes] = 0;
    LDPL_SOCKET_MSG = buf;
}

void LDPL_SOCKET_POLL(){
    int sock = LDPL_SOCKET_NUMBER;
    fcntl(sock, F_SETFL, O_NONBLOCK);
    char buf[BUF_SIZE];
    int bytes = read(sock, buf, BUF_SIZE);
    if(bytes < 0) {
        //socket_closed(sock);
        LDPL_SOCKET_MSG = "";
        return;
    }
    buf[bytes] = 0;
    LDPL_SOCKET_MSG = buf;
}


