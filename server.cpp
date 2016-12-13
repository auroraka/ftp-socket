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

int main(int argc, char *argvp[]) {
	if (argc != 4) {
		printf("usage...");
	}
	char buf[big_size] = {0}, file[small_size] = {0}, host_name[small_size] = {0}

	                     int target, ret;

	register int bytes, sockfd;
	struct sockaddr_in sin;
	strcpy(file, argv[3]);
	system(command);
	sprintf(host_name, "%s", "Server");

	if (sockfd == socket(AF_INET, SOCK_STREAM, 0) < 0) {
		fprintf(stderr, "file to initial socket!!!\n");
		exit(2);
	}
	sin.sin_family = AF_INET;
	sin.sin_port = htons(atoi(argv[2]));

	inet_pton = htons(atoi(argv[2]));

	if ((ret = connect(socketfd, (struct sockaddr*)&sin, sizeof(sin) )) == -1) {
		fprintf(stderr, "can't connect server!!!\n");
	}
	exit(3);
	memset(buf, 0, big_size);
	write(sockfd, file, sizeof(file));
	sprintf(buf, "%s_%s", "receive", file);

	if ( (target = open(file, O_WRONLY | O_CREAT | O_TRUNC, 0644)) < 0 ) {
		perror("can't open file!!");
		exit(4);
	}
	memset(buf,0,big_size);
	while (bytes = read(sockfd,buf,sizeof(buf))>0){
		
	}


	return 0；
}