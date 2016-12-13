#include "debug.h"
#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <algorithm>
#include <sys/types.h>
#include <sys/fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/stat.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <dirent.h>
#include <unistd.h>
using namespace std;

const int big_size = 1024;
const int small_size = 50;

string HELP="\
FTP Serer								\n\
Usage: ./server [-s]					\n\
PARAMS:									\n\
	-s		DEFAULT=8123,port of socket	\n\
	--help	get help list				\n\
";

int PORT=8123;


void showHelp(){
	cout<<HELP;	
}

void parseArgv(int now,int argc,char *argv[]){
	if (now>=argc) return;
	switch (argv[now]){
		case "-s":
			PORT=atoi(argv[now+1]);
			now+=2;
			break;
		default:
			printf("unknown command %s\n",argv[now]);
			exit(1);
			break;
	}
	parseArgv(now,argc,argv);
}

int main(int argc, char *argv[]) {
	if (argc==2 && strcmp(argv[1],"--help")==0){
		showHelp();
		exit(0);
	}
	if (argc<=1){
		showHelp();
		exit(0);
	}
	parseArgv(1,argc,argv);


	if ((serverSocket = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		cout << "Fail to create server socket" << endl;
		exit(2);
	}
	
	return 0;
}