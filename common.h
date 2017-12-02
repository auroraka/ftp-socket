#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <unistd.h>
#include <fstream>
#include <dirent.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <signal.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <assert.h>
#include <string>
#include <netinet/in.h>
#include <netdb.h>

using namespace std;

// ------------ Defines  -------------
#define BUF_SIZE 256
enum DataType {TYPE_ASCII, TYPE_IMAGE};
enum ModeType {MODE_STREAM, MODE_BLOCK, MODE_COMPRESS};
enum StructureType {STRUCTURE_FILE, STRUCTURE_RECORD, STRUCTURE_PAGE};

// ------------ Params -------------
static int PORT = 21;
static char ADDRESS[20] = "127.0.0.1";
extern char MSG[100];
extern const string ROOT_DIR;
// ----------- Function -----------


void Info(string msg) {
    #ifdef SERVER
    puts(("[info] " + msg).c_str());
    #endif
}
void Debug(string msg) {
    #ifdef SERVER
    puts(("[debug] " + msg).c_str());
    #endif
}
void Receive(string msg) {
    #ifdef SERVER
    puts(("[receive] " + msg).c_str());
    #endif
}
void Error(string msg) {
    puts(("[ERROR] " + msg).c_str());
    exit(1);
}

//get command from socket
string readLine(int sockfd) {
    string buf;
    char _tmp[BUF_SIZE];
    while (1) {
        int _len = read(sockfd, _tmp, BUF_SIZE - 1);
        for (int i = 0 ; i < _len ; i += 1) buf += _tmp[i];
        if (_len == 0 || _tmp[_len - 1] == '\n') break;
    }
    while (buf.length() != 0 && (buf.back() == '\n' || buf.back() == '\r'))
        buf.pop_back();
    return buf;
}

//get from command line
string getCommand() {
    string cmd;
    std::getline(std::cin, cmd);
    while (cmd.back() == '\n' || cmd.back() == '\r' || cmd.back() == ' ')
        cmd.pop_back();
    return cmd;
}

//connect to server
int connectSocket(const char * host, int portno) {
    //sprintf(MSG,"Connecting to %s:%d\n", host, portno);
    //Info(MSG);
    int sockfd = -1;
    struct sockaddr_in serv_addr;
    struct hostent *server;
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    server = gethostbyname(host);
    if (server == NULL) {
        Error("No such host.");
        return -1;
    }
    serv_addr.sin_family = AF_INET;
    memcpy(&serv_addr.sin_addr.s_addr, server->h_addr, server->h_length);
    serv_addr.sin_port = htons(portno);
    if (connect(sockfd, (struct sockaddr *) & serv_addr, sizeof(serv_addr)) < 0) {
        Error("Connect error.");
        return -1;
    }
    Info("Connect OK.");
    return sockfd;
}


int send_message(int sockfd, int code, const string & msg) {
    string buf = std::to_string(code);
    buf += ' ';
    buf += msg;
    Info("Send: " + buf);
    buf += "\r\n";
    return write(sockfd, buf.c_str(), buf.length());
}

void send_data(int sockfd, int &datasock,
               const void * buf, int buf_len) {
    if (datasock < 0) {
        send_message(sockfd, 150, "File status okay. About to open data connection.");
        return ;
    }
    send_message(sockfd, 125, "Data connection already open. Transfer starting.");
    write(datasock, buf, buf_len);
    close(datasock);
    datasock = -1;
    send_message(sockfd, 226, "Transfer complete.");
}

void receive_data(int sockfd, int &datasock, string & content) {
    content = "";
    if (datasock < 0) {
        send_message(sockfd, 150, "File status okay. About to open data connection.");
        return ;
    }
    send_message(sockfd, 125, "Data connection already open. Transfer starting.");
    char _tmp[BUF_SIZE];
    while (1) {
        int _len = read(datasock, _tmp, BUF_SIZE - 1);
        for (int i = 0 ; i < _len ; i += 1) content += _tmp[i];
        if (_len <= 0) break;
    }
    close(datasock);
    datasock = -1;
    send_message(sockfd, 226, "Transfer complete.");
}

void receive_data(int &datasock, string & content) {
    content = "";
    char _tmp[BUF_SIZE];
    while (1) {
        int _len = read(datasock, _tmp, BUF_SIZE - 1);
        for (int i = 0 ; i < _len ; i += 1) content += _tmp[i];
        if (_len <= 0) break;
    }
    close(datasock);
    datasock = -1;
}


int readFile(const string & filename, string & content) {
    std::sprintf(MSG, "Reading file: %s\n", (ROOT_DIR + filename).c_str());
    Info(MSG);
    std::ifstream in(ROOT_DIR + filename, std::ios::in | std::ios::binary);
    if (in) {
        content = "";
        in.seekg(0, std::ios::end);
        content.resize(in.tellg());
        in.seekg(0, std::ios::beg);
        in.read(&content[0], content.size());
        in.close();
        std::sprintf(MSG, "Read ok\n");
        Info(MSG);
        return 0;
    }
    Info("Read error.\n");
    return -1;
}

int storeFile(const string & filename, string & content) {
    Info("Writing to file: " + filename);
    std::ofstream out(ROOT_DIR + filename, std::ios::out | std::ios::binary);
    if (out) {
        out << content;
        out.close();
        Info("Write ok. Size: " + std::to_string(content.size()));
        return 0;
    }
    Info("Write file error.");
    return -1;
}



#endif