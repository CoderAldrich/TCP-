�ļ���client.c
PS����һ�������Ϸ������Ŀͻ��ˣ���Ϊclient1���ڶ��������Ϸ������Ŀͻ��˳�Ϊclient2
�������Ĺ����ǣ��������Ϸ����������ݷ������ķ��ؾ�������client1����client2��
����client1�����ʹӷ������ϵõ�client2��IP��Port��������client2,
����client2�����ʹӷ������ϵõ�client1��IP��Port������ת�����port��
�ڳ���������һ��client1�����������ʧ�ܣ���Ȼ����ݷ��������ص�port���м�����
�����Ժ󣬾����������ͻ���֮����е�Ե�ͨ���ˡ�
*/

#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <arpa/inet.h>

#define MAXLINE 128
#define SERV_PORT 8877

typedef struct
{
	char ip[32];
	int port;
}server;

//���������������˳�����
void error_quit(const char *str)    
{    
	fprintf(stderr, "%s", str); 
	//��������˴���ţ����������ԭ��
	if( errno != 0 )
		fprintf(stderr, " : %s", strerror(errno));
	printf("\n");
	exit(1);    
}   

int main(int argc, char **argv)     
{          
	int i, res, port;
	int connfd, sockfd, listenfd; 
	unsigned int value = 1;
	char buffer[MAXLINE];      
	socklen_t clilen;        
	struct sockaddr_in servaddr, sockaddr, connaddr;  
	server other;

	if( argc != 2 )
		error_quit("Using: ./client <IP Address>");

	//�����������ӣ��������������׽���        
	sockfd = socket(AF_INET, SOCK_STREAM, 0); 
	memset(&sockaddr, 0, sizeof(sockaddr));      
	sockaddr.sin_family = AF_INET;      
	sockaddr.sin_addr.s_addr = htonl(INADDR_ANY);      
	sockaddr.sin_port = htons(SERV_PORT);      
	inet_pton(AF_INET, argv[1], &sockaddr.sin_addr);
	//���ö˿ڿ��Ա�����
	setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &value, sizeof(value));

	//������������
	res = connect(sockfd, (struct sockaddr *)&sockaddr, sizeof(sockaddr)); 
	if( res < 0 )
		error_quit("connect error");

	//�����������ж�ȡ����Ϣ
	res = read(sockfd, buffer, MAXLINE);
	if( res < 0 )
		error_quit("read error");
	printf("Get: %s", buffer);

	//�����������ص���first����֤���ǵ�һ���ͻ���
	if( 'f' == buffer[0] )
	{
		//�ӷ������ж�ȡ�ڶ����ͻ��˵�IP+port
		res = read(sockfd, buffer, MAXLINE);
		sscanf(buffer, "%s %d", other.ip, &other.port);
		printf("ff: %s %d\n", other.ip, other.port);

		//�������ڵ��׽���        
		connfd = socket(AF_INET, SOCK_STREAM, 0); 
		memset(&connaddr, 0, sizeof(connaddr));      
		connaddr.sin_family = AF_INET;      
		connaddr.sin_addr.s_addr = htonl(INADDR_ANY);      
		connaddr.sin_port = htons(other.port);    
		inet_pton(AF_INET, other.ip, &connaddr.sin_addr);

		//����ȥ���ӵڶ����ͻ��ˣ�ǰ���ο��ܻ�ʧ�ܣ���Ϊ��͸��û�ɹ���
		//�������10�ζ�ʧ�ܣ���֤����͸ʧ���ˣ�������Ӳ����֧�֣�
		while( 1 )
		{
			static int j = 1;
			res = connect(connfd, (struct sockaddr *)&connaddr, sizeof(connaddr)); 
			if( res == -1 )
			{
				if( j >= 10 )
					error_quit("can't connect to the other client\n");
				printf("connect error, try again. %d\n", j++);
				sleep(1);
			}
			else 
				break;
		}

		strcpy(buffer, "Hello, world\n");
		//���ӳɹ���ÿ��һ������Է����ͻ���2������һ��hello, world
		while( 1 )
		{
			res = write(connfd, buffer, strlen(buffer)+1);
			if( res <= 0 )
				error_quit("write error");
			printf("send message: %s", buffer);
			sleep(1);
		}
	}
	//�ڶ����ͻ��˵���Ϊ
	else
	{
		//�������������ص���Ϣ��ȡ���ͻ���1��IP+port���Լ�����ӳ����port
		sscanf(buffer, "%s %d %d", other.ip, &other.port, &port);

		//��������TCPЭ����׽���        
		sockfd = socket(AF_INET, SOCK_STREAM, 0); 
		memset(&connaddr, 0, sizeof(connaddr));      
		connaddr.sin_family = AF_INET;      
		connaddr.sin_addr.s_addr = htonl(INADDR_ANY);      
		connaddr.sin_port = htons(other.port);      
		inet_pton(AF_INET, other.ip, &connaddr.sin_addr);
		//���ö˿�����
		setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &value, sizeof(value));

		//�������ӿͻ���1���϶���ʧ�ܣ���������·���������¼�¼��
		//�԰�æ�ͻ���1�ɹ���͸���������Լ� 
		res = connect(sockfd, (struct sockaddr *)&connaddr, sizeof(connaddr)); 
		if( res < 0 )
			printf("connect error\n");

		//�������ڼ������׽���        
		listenfd = socket(AF_INET, SOCK_STREAM, 0); 
		memset(&servaddr, 0, sizeof(servaddr));      
		servaddr.sin_family = AF_INET;      
		servaddr.sin_addr.s_addr = htonl(INADDR_ANY);      
		servaddr.sin_port = htons(port);
		//���ö˿�����
		setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &value, sizeof(value));

		//��socket��socket��ַ�ṹ��ϵ���� 
		res = bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr));    
		if( -1 == res )
			error_quit("bind error");

		//��ʼ�����˿�       
		res = listen(listenfd, INADDR_ANY);    
		if( -1 == res )
			error_quit("listen error");

		while( 1 )
		{
			//�������Կͻ���1������
			connfd = accept(listenfd,(struct sockaddr *)&sockaddr, &clilen);  
			if( -1 == connfd )
				error_quit("accept error");

			while( 1 )
			{
				//ѭ����ȡ�����ڿͻ���1����Ϣ
				res = read(connfd, buffer, MAXLINE);
				if( res <= 0 )
					error_quit("read error");
				printf("recv message: %s", buffer);
			}
			close(connfd);
		}
	}

	return 0;
}
