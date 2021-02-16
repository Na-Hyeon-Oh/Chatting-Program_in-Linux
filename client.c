#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <pthread.h>


#define MAXBUF 256
#define MAX 100
#define USER 20


struct message{
	char user[MAX];
	char buf[MAXBUF];
};


int main(int argc, char argv[]){
	struct hostent* h;
	char* host=argv[1];
	int port=atoi(argv[2]);	
	int server_sock;
	int caddr_len;
	struct sockaddr_in client_addr, server_addr;
	char buf[MAXBUF], pre[USER];
	fd_set readset, copyset;
	int fd;
	
	char name[USER];
	struct message msg;
	int re;


	if((server_sock=socket(AF_INET, SOCK_STREAM, 0))<0){
		perror("socket error : ");
		exit(1);
	}

	if((h=gethostbyname(host))==NULL){
		printf("invalid hostname %s\n", host);
		exit(2);
	}
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family=AF_INET;
	memcpy((char*)&server_addr.sin_addr.s_addr, (char*)h->h_addr, h->h_length);
	//server_addr.sin_addr.s_addr=inet_addr("127.0.0.1");
	server_addr.sin_port=htons(port);

	caddr_len=sizeof(server_addr);



	//사용자 입력 받기
	printf("사용자를 입력하시오 : ");
	fgets(name, USER, stdin);
	*(name+(strlen(name)-1))='\0';


	if(connect(server_sock, (struct sockaddr*)&server_addr, caddr_len)<0){
		perror("connect error : ");
		return -1;
	}


	memset(&msg, 0, sizeof(msg));
	strcpy(msg.user, name);
	strcpy(msg.buf,"");


	//메시지 전송
	if(write(server_sock, (struct message*)&msg, sizeof(msg))<0){
		perror("write error : ");
		exit(1);
	}

	FD_ZERO(&readset);
	FD_SET(server_sock, &readset);
	FD_SET(0, &readset);

	fd=server_sock;


	while(1){
		copyset=readset;

		if(select(fd+1, &copyset, 0, 0, 0)<0){
			perror("select error : ");
			exit(1);
		}


		//서버와 연결된 소켓의 변화
		if(FD_ISSET(server_sock, &copyset)){
			memset(buf, 0, MAXBUF);
			if((re=read(server_sock, buf, MAXBUF))<0){
				perror("read error : ");
				exit(1);
			}
			//클라이언ㅌ 서버의 접속이 끊김
			else if(re==0){
				printf("서버와의 접속이 끊어졌습니다.\n");
				break;
			}

			//받아온 메시지 출력
			printf("%s\n", buf);
		}
		
		//입력에 대한 변화
		else if(FD_ISSET(0, &copyset)){
			memset(&msg, 0, sizeof(msg));
			memset(buf, 0, MAXBUF);

			//보낼 유저 입력
			fgets(msg.user, USER, stdin);
			*(msg.user+(strlen(msg.user)-1))='\0';

			//보낼 메시지 입력
			printf("전송할 메시지를 입력해주세요 : ");
			fgets(buf, MAXBUF, stdin);
			*(buf+(strlen(buf)-1))='\0';
			sprintf(msg.buf, "[%s]: %s", name, buf);

			if(write(server_sock, (struct message*)&msg, sizeof(msg))<0){
				perror("write error : ");
				exit(1);
			}
		}
	}
	close(server_sock);
	return 0;
}
			
