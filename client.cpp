#include "common.h"
#include "debug.h"
#include "tools.h"

const string ROOT_DIR="./clientdata/";


string HELP_CLIENT = "Usage: sudo ./client <addr=127.0.0.1> <port=21>";

char MSG[100];

string getReply(int sockfd, string msg) {
    msg += "\r\n";
    write(sockfd, msg.c_str(), msg.length());
    string ret = readLine(sockfd);
    std::cout << ret << std::endl;
    return ret;
}

int connectPASV(int sockfd) {
    Info("Connecting PASV...");
    string tmp("PASV");
    tmp = getReply(sockfd, tmp);
    assert(strncmp(tmp.c_str(), "227", 3) == 0);
    tmp = tmp.substr(tmp.find('(') + 1);
    string _addr;
    for (int i = 0, dot_count = 0; i < tmp.length() ; i += 1) {
        if (tmp[i] == ',') {
            if (dot_count == 3) break;
            tmp[i] = '.';
            dot_count += 1;
        }
        _addr += tmp[i];
    }
    unsigned int portno = 0;
    tmp = tmp.substr(tmp.find(',') + 1);
    char * s = new char[tmp.length() + 1];
    strcpy(s, tmp.c_str());
    portno = atoi(strtok(s, ",) "));
    portno <<= 8;
    portno += atoi(strtok(NULL, ",) "));
    delete []s;
    return connectSocket(_addr.c_str(), portno);
}



int main(int argc, char *argv[]) {
    assert(argc > 0);

    if (argc >= 2) {
        strcpy(argv[1], ADDRESS);
    }
    if (argc >= 3) {
        PORT = atoi(argv[2]);
    }

    //产生SIGPIPE信号时不中断而是直接忽略,避免因不完全关闭导致的程序中断
    signal(SIGPIPE, SIG_IGN);

    //连接服务器指定地址和端口
    int sockfd = connectSocket(ADDRESS, PORT);
    if (sockfd >= 0) {
        std::sprintf(MSG, "Connect to %s:%d success", ADDRESS, PORT);
        Info(MSG);
    } else {
        std::sprintf(MSG, "Connect to %s:%d failed", ADDRESS, PORT);
        Error(MSG);
        exit(1);
    }

    string tmp = readLine(sockfd);
    std::cout << tmp << std::endl;
    int datasock = -1;

    while (1) {
        if (datasock < 0) datasock = connectPASV(sockfd);
        printf("> ");
        string cmd;
        cmd = getCommand();
        string reply = getReply(sockfd, cmd);

        if (strncmp("125", reply.c_str(), 3) == 0) {
            string filecontent;
            if(strncmp("STOR", cmd.c_str(), 4) == 0){
                tmp = cmd.substr(cmd.find(' ') + 1);
                assert(readFile(tmp, filecontent) >= 0);
                write(datasock, filecontent.c_str(), filecontent.size());
                close(datasock);
            }
            if(strncmp("RETR", cmd.c_str(), 4) == 0){
                tmp = cmd.substr(cmd.find(' ') + 1);
                receive_data(datasock, filecontent);
                assert(storeFile(tmp, filecontent) >= 0);
            }
            if(strncmp("LIST", cmd.c_str(), 4) == 0){
                receive_data(datasock, filecontent);
                std::cout << filecontent << std::endl;
            }
            datasock = -1;
            readLine(sockfd);
        } else if (strncmp("221", reply.c_str(), 3) == 0) {
            exit(0);
        }
    }


    return 0;
}