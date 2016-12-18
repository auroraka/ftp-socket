#include "common.h"
#include "debug.h"
#include "tools.h"
using namespace std;

const string ROOT_DIR = "./serverdata/";

const string HELP_CLIENT = "Usage: sudo ./server <port=21>";

static int AUTHORIZED = 0;

char MSG[100];

int authorize(string username, string password) {
    if (username == "root" && password == "123456") {
        AUTHORIZED = 1;
        return 1;
    }
    return 0;
}

int listDir(string cwd, string & ret) {
    ret = "";
    DIR * dp;
    struct dirent *ep;
    dp = opendir((ROOT_DIR + cwd).c_str());
    if (dp == NULL) return -1;
    while (ep = readdir(dp)) {
        ret += ep->d_name;
        ret += "\r\n";
    }
    closedir(dp);
    return 0;
}


int openPasvSocket(int sockfd) {
    struct sockaddr_in localaddr, dataserv_addr, datacli_addr;
    socklen_t localaddrlen = sizeof(localaddr),
              datacli_addr_len = sizeof(datacli_addr),
              dataserv_addr_len = sizeof(datacli_addr);

    //选择一个未占用的端口绑定
    getsockname(sockfd, (struct sockaddr*)&localaddr, &localaddrlen);
    char_buf ret_str = inet_ntoa(localaddr.sin_addr);
    for (int i = 0 ; i < ret_str.length() ; i += 1)
        if (ret_str[i] == '.') ret_str[i] = ',';
    dataserv_addr.sin_family = AF_INET;
    dataserv_addr.sin_addr.s_addr = INADDR_ANY;
    dataserv_addr.sin_port = 0;
    int dataserve_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (bind(dataserve_sock, (struct sockaddr *)&dataserv_addr,
             sizeof(dataserv_addr)) < 0) {
        Error("Bind PASV socket error.");
        send_message(sockfd, 421, "Service not available");
        return -1;
    }
    listen(dataserve_sock, 1);

    //监听该端口
    getsockname(dataserve_sock, (struct sockaddr *)&dataserv_addr, &datacli_addr_len);
    unsigned int dataserv_port = ntohs(dataserv_addr.sin_port);
    Info((char_buf("PASV socket port: ") +
          std::to_string(dataserv_port)).c_str());
    ret_str += ",";
    ret_str += std::to_string(dataserv_port >> 8) + ",";
    ret_str += std::to_string(dataserv_port & 0xff) + ").";
    ret_str = "Entering Passive Mode (" + ret_str;
    send_message(sockfd, 227, ret_str);
    Info("Listening PASV socket.");
    int newfd = accept(dataserve_sock,
                       (struct sockaddr *)&datacli_addr,
                       (socklen_t *)&datacli_addr_len);
    Info("PASV socket accepted.");
    close(dataserve_sock);
    return newfd;
}

//Ftp通信主函数
void * thread_do(void * arg) {
    signal(SIGPIPE, SIG_IGN);

    //params
    int sockfd = *(int *)arg;
    string buf;
    string username, password;
    string cwd = "/";
    DataType datatype = TYPE_ASCII;
    ModeType modetype = MODE_STREAM;
    int datasock = -1;

    send_message(sockfd, 220, "Connect success, Welcome to use");
    while (1) {
        buf = readLine(sockfd);
        if (buf.length() == 0) {
            Info("Read error. exit.");
            break;
        }
        char * buf_str = new char[buf.length() + 1];
        strcpy(buf_str, buf.c_str());
        char * token = strtok(buf_str, " \r\n");

        Receive(token);
        if (strcmp(token, "USER") == 0) {
            username = strtok(NULL, " \r\n");
            send_message(sockfd, 331, "Username ok, need password");

        } else if (strcmp(token, "PASS") == 0) {
            password = strtok(NULL, " \r\n");
            if (authorize(username, password))
                send_message(sockfd, 230, "Authorition success.");
            else
                send_message(sockfd, 530, "Failed to login: Username/Password incorrect.");
        } else if (strcmp(token, "PASV") == 0) {
            datasock = openPasvSocket(sockfd);
        } else if (strcmp(token, "QUIT") == 0) {
            send_message(sockfd, 221, "You can quit now.");
            return NULL;
        } else if (AUTHORIZED) {
            if (strcmp(token, "PWD") == 0) {
                string _tmp = '"' + cwd + '"';
                send_message(sockfd, 257, _tmp + " is the current dir.");
            } else if (strcmp(token, "TYPE") == 0) {
                token = strtok(NULL, " \r\n");
                if (strcmp(token, "A") == 0) {
                    datatype = TYPE_ASCII;
                    send_message(sockfd, 200, "Type set to ASCII.");
                    continue;
                } else if (strcmp(token, "I") == 0) {
                    datatype = TYPE_IMAGE;
                    send_message(sockfd, 200, "Type set to Binary.");
                    continue;
                }
                send_message(sockfd, 504, "Unsupport type.");
            } else if (strcmp(token, "MODE") == 0) {
                token = strtok(NULL, " \r\n");
                if (strcmp(token, "S") == 0) {
                    modetype = MODE_STREAM;
                    send_message(sockfd, 200, "Mode set to STREAM.");
                    continue;
                } else if (strcmp(token, "B") == 0) {
                    modetype = MODE_BLOCK;
                    send_message(sockfd, 200, "Mode set to Block.");
                    continue;
                } else if (strcmp(token, "C") == 0) {
                    modetype = MODE_COMPRESS;
                    send_message(sockfd, 200, "Mode set to Compress.");
                    continue;
                }
                send_message(sockfd, 504, "Unsupport type.");
            } else if (strcmp(token, "NLST") == 0 || strcmp(token, "LIST") == 0) {
                string _tmp ;
                token = strtok(NULL, " \r\n");
                if (token == NULL) listDir(cwd, _tmp);
                else listDir(cwd + token, _tmp);
                send_data(sockfd, datasock, _tmp.c_str(), _tmp.length());
            } else if (strcmp(token, "RETR") == 0) {
                token = strtok(NULL, " \r\n");
                string _tmp;
                if (readFile(cwd + token, _tmp) == 0)
                    send_data(sockfd, datasock, _tmp.c_str(), _tmp.length());
                else send_message(sockfd, 550, "Open file error.");
            } else if (strcmp(token, "CWD") == 0) {
                token = strtok(NULL, " \r\n");
                string _tmp;
                if (listDir(cwd + token, _tmp) == 0) {
                    cwd += token;
                    cwd += '/';
                    send_message(sockfd, 200, "Changed dir to " + cwd + ".");
                }
                else send_message(sockfd, 550, "Dir not exist.");
            } else if (strcmp(token, "STOR") == 0) {
                token = strtok(NULL, " \r\n");
                string _tmp;
                read_data(sockfd, datasock, _tmp);
                storeFile(cwd + token, _tmp);
            } else if (strcmp(token, "ALLO") == 0) {
                send_message(sockfd, 202, "No storage allocation necessary.");
            } else if (strcmp(token, "") == 0) {
            } else if (strcmp(token, "") == 0) {
            } else if (strcmp(token, "") == 0) {
            } else if (strcmp(token, "") == 0) {
            } else if (strcmp(token, "") == 0) {
            } else {
                send_message(sockfd, 500, "Unknown or not finished command.");
            }
        } else {
            send_message(sockfd, 530, "Please login.");
        }
    }

    return NULL;
}

int main(int argc, char *argv[]) {
    Debug("hello world");

    if (argc >= 2) {
        PORT = atoi(argv[1]);
    }
    Debug("./server");

    Debug("port is " + to_string(PORT));
    //申请socket资源
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    assert(sockfd >= 0);
    Debug("get socket");


    //绑定对应端口
    struct sockaddr_in serv_addr, cli_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(PORT);
    if (!(bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) >= 0)) {
        Error("bind port [" + to_string(PORT) + "] error, are you sudo?");
    }
    Debug("bind port");

    //监听
    listen(sockfd, 5);

    pthread_t th;
    Info("Bind Address 127.0.0.1:" + to_string(PORT));
    Info("Listening...");

    while (1) {
        //新连接建立
        int cli_addr_len = sizeof(cli_addr);
        int newfd = accept(sockfd, (struct sockaddr *)&cli_addr, (socklen_t *)&cli_addr_len);

        //新开进程处理此连接
        pthread_create(&th, NULL, thread_do, (void *)&newfd);
        std::sprintf(MSG, "Accept connection from %s:%d, new thread %d\n",
                     inet_ntoa(cli_addr.sin_addr), ntohs(cli_addr.sin_port), th);
        Info(MSG);
    }

    return 0;
}