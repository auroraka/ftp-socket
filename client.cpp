#include <sys/types.h>
#include <sys/fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <algorithm>
using namespace std;

#define MAXLINE 4096
int dataSocket , commandSocket , ret; // ���� ָ��socket 
char recvLine[MAXLINE] , sendLine[MAXLINE]; // ���͡����ܵ��ַ���
string recvMsg; // ���յ�����ϢתΪString �Ӷ�ʹ��substr����
struct sockaddr_in serverAddr;  // ָ������б���IP��ַ���˿ںŵ���Ϣ��sockaddr���͵�ָ�� 

int main(int argc , char** argv)
{	
	if(argc != 3) // �ж������Ƿ�Ϸ�
	{
		cout << "usage: ./client <ipaddress> port" << endl;
		exit(0);
	}
	memset(&serverAddr , 0 , sizeof(sockaddr_in));
	serverAddr.sin_family = AF_INET; //IPV4
	serverAddr.sin_addr.s_addr = htonl(INADDR_ANY); //IP��ַ���ó�INADDR_ANY,��ϵͳ�Զ���ȡ������IP��
	serverAddr.sin_port = htons(atoi(argv[2])); // �˿ں� Host to Network Short
	if(inet_aton(argv[1], &serverAddr.sin_addr) == 0) //�ж�IP�����Ƿ���ȷ
    {
        cout << "Sever IP address failed" << endl;
		exit(0);
	}	
	if((commandSocket = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
	{
		cout << "Fail to create command socket" << endl;
		exit(0);
	}
	if((dataSocket = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		cout << "Fail to create data socket" << endl;
		exit(0);
	}
	if ((ret = connect(commandSocket, (struct sockaddr*) &serverAddr, sizeof(serverAddr))) == -1) // ?��2��??��?socket��?��?��??��3��1? 
	{
		cout << "Command socket fails to connect server" << endl;
		exit(0);	
	}
	if(ret = connect(dataSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == -1) // ?��2����y?Ysocket��?��?��??��3��1? 
	{
		cout << "Data socket fails to connect server" << endl;
		exit(0);
	}
	cout << "send msg to server" << endl;
	while(1) 
	{
		cout << "UserCommand: ";
		// ����command
		fgets(sendLine, MAXLINE, stdin);
		write(commandSocket , sendLine , strlen(sendLine));
		// ���ܷ��������ص����� ��ת��Ϊstring
		int count = 0;
		memset(recvLine , '\0' , sizeof(recvLine));
		recvMsg = "";
		while ((count = read(commandSocket, recvLine, MAXLINE)) == 0); // ��������
		for(int i = 0;i < count;i++)
		{
			recvMsg += recvLine[i];
		}
		//�����Ӧ��ָ��
		if(recvMsg.substr(0 , 4) == "help")
		{
			cout << recvMsg.substr(5) << endl;
		}
		else if(recvMsg.substr(0 , 4) == "quit")
		{
			cout << recvMsg.substr(5) << endl;
			break;
		}
		else if(recvMsg.substr(0 , 4) == "get ")
		{
			if(recvMsg.substr(4) == "fail")
				cout << "No such file!" << endl;
			else
			{
				int count = 0;
				char buffer[MAXLINE]; // ���ļ�����
				string fileName = recvMsg.substr(4);
				cout << "fileName: " << fileName << endl;
				string fileContent = "";
				while ((count = read(dataSocket, buffer, MAXLINE)) == 0);
				for (int i = 0; i < count; i ++)
					fileContent += buffer[i];
				FILE *fout = fopen(fileName.c_str(), "wb"); // д���ļ�
				fwrite(fileContent.c_str(), sizeof(char), count, fout); // д���ļ�
				fclose(fout);
			}
		}
		else if(recvMsg.substr(0 , 4) == "put ")
		{
			string fileName = recvMsg.substr(4);
			string response = "";
			FILE *fin = fopen(fileName.c_str(), "rb");
			if(fin == NULL)
			{
				cout << "Local no such file" << endl;
				response = "fail";
				write(dataSocket, response.c_str(), strlen(response.c_str()));
			}
			else
			{
				response = "";
				int fileSize = 0;
				char buf[MAXLINE]; // ����
				while((fileSize = fread(buf, sizeof(char), MAXLINE , fin)) == 0); // ���ļ�
				response = string(buf);
				write(dataSocket , response.c_str() , strlen(response.c_str()));
			}
			fclose(fin);
		}
		else if(recvMsg.substr(0 , 3) == "pwd")
		{
			cout << recvMsg.substr(4) << endl;
		}
		else if(recvMsg.substr(0 , 3) == "dir")
		{
			cout << recvMsg.substr(4) << endl;
		}
		else if(recvMsg.substr(0 , 2) == "cd")
		{
			cout << recvMsg.substr(3) << endl;
		}
		else
		{
			cout << recvMsg << endl;
		}
			
	}

	return 0;

}