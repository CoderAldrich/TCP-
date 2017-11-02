/*
�ļ���server.c
PS����һ�������Ϸ������Ŀͻ��ˣ���Ϊclient1���ڶ��������Ϸ������Ŀͻ��˳�Ϊclient2
����������Ĺ����ǣ�
1������client1��������"first"������client2������֮�󣬽�client2����ת�����IP��port����client1��
2������client2��������client1����ת�����IP��port�������port���������Ͽ������ǵ����ӡ�
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

int main(void)      
{          
	int i, res, cur_port; 
	int connfd, firstfd, listenfd;   
	int count = 0;
	char str_ip[MAXLINE];	 //����IP��ַ
	char cur_inf[MAXLINE];	 //��ǰ��������Ϣ[IP+port]
	char first_inf[MAXLINE];	//��һ�����ӵ���Ϣ[IP+port]
	char buffer[MAXLINE];	 //��ʱ���ͻ�����
	socklen_t clilen;      
	struct sockaddr_in cliaddr;      
	struct sockaddr_in servaddr;

	//�������ڼ���TCPЭ���׽���        
	listenfd = socket(AF_INET, SOCK_STREAM, 0);      
	memset(&servaddr, 0, sizeof(servaddr));      
	servaddr.sin_family = AF_INET;      
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);      
	servaddr.sin_port = htons(SERV_PORT);      

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
		//�������Կͻ��˵�����
		connfd = accept(listenfd,(struct sockaddr *)&cliaddr, &clilen);  
		if( -1 == connfd )
			error_quit("accept error");
		inet_ntop(AF_INET, (void*)&cliaddr.sin_addr, str_ip, sizeof(str_ip));

		count++;
		//���ڵ�һ�����ӣ������IP+port�洢��first_inf�У�
		//���������������ӣ�Ȼ�����������ַ���'first'��
		if( count == 1 )
		{
			firstfd = connfd;
			cur_port = ntohs(cliaddr.sin_port);
			snprintf(first_inf, MAXLINE, "%s %d", str_ip, cur_port);   
			strcpy(cur_inf, "first\n");
			write(connfd, cur_inf, strlen(cur_inf)+1);
		}
		//���ڵڶ������ӣ������IP+port���͸���һ�����ӣ�
		//����һ�����ӵ���Ϣ���������port���ظ����Լ���
		//Ȼ��Ͽ��������ӣ������ü�����
		else if( count == 2 )
		{
			cur_port = ntohs(cliaddr.sin_port);
			snprintf(cur_inf, MAXLINE, "%s %d\n", str_ip, cur_port);
			snprintf(buffer, MAXLINE, "%s %d\n", first_inf, cur_port);
			write(connfd, buffer, strlen(buffer)+1);
			write(firstfd, cur_inf, strlen(cur_inf)+1); 
			close(connfd);
			close(firstfd);
			count = 0;
		}
		//����������е�����ǿ϶��ǳ�����
		else
			error_quit("Bad required");
	}
	return 0;
}
