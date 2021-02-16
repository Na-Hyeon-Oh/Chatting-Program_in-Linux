#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <pthread.h>

#define MAXBUF 256//max length of message
#define MAX 100//max number of client
#define ALL 1
#define USER 20//max length of user name

char login[]="접속되었습니다";
char not_login[]="접속에 실패하셨습니다";

struct client{
	int fd;//client file descriptor
	char name[USER];//client name
};

struct message{
	char toUser[USER];//message receiver
	char msg[MAXBUF];//message content
};

void writeMessage(void* client_msg, void* num, int base_fd, int max_fd);

int main(int argc, char argv[]){
	int port=argv[1];
	int option_value=1;
	int listenfd, connectfd;
	struct sockaddr_in server_addr, client_addr;
	int caddr_len, data_len;
	fd_set readset, copyset;
	int fd;
	
	struct client clients[MAX];
	struct message msg_info;
	int fdmax;
	int i;

	
	//클라이언트와 연결할 소켓 생성
	if((listenfd=socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0){
		perror("socket error\n");
		exit(1);
	}

	//소켓 정보
	setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &option_value, sizeof(option_value));

	memset(clients, 0, sizeof(clients));
	
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family=AF_INET;
	server_addr.sin_addr.s_addr=htonl(INADDR_ANY);
	server_addr.sin_port=htons(port);

	//소켓을 해당 주소로 연결
	if(bind(listenfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0){
		perror("bind error\n");
		exit(1);
	}

	//클라이언트 접속을 wait
	if(listen(listenfd, 5)<0){
		perror("listen error\n");
		exit(1);
	}

	caddr_len=sizeof(client_addr);

	FD_ZERO(&readset);
	FD_SET(listenfd, &readset);

	fdmax=listenfd;


	while(1){
		copyset=readset;

		//for 입력에 대한 즉각적인 대응(concurrent i/o multiplexing programming)
		if(select(fdmax+1, &copyset, 0, 0, (struct timeval*)0) < 1){
			perror("select err\n");
			exit(1);
		}

		for(fd=0; fd<fdmax+1; fd++){
			if(FD_ISSET(fd, &copyset)){

				//new connection
				if(fd==listenfd){
					if(connectfd=accept(listenfd, (struct socket_addr*)&client_addr, &caddr_len)){
						perror("accept error\n");
						exit(0);
					}

					FD_SET(connectfd, &readset);
					//if(fdmax < connectfd) fdmax=connectfd;
					printf("New Client at fd %d\n", connectfd);

					for(i=0; i<MAX;i++){
						if(clients[i].fd==0){
							clients[i].fd=connectfd;
							fdmax++;
							break;
						}
					}

					if(fdmax<connectfd) fdmax=connectfd;
					
				}

				//new message
				else{
					memset(&msg_info, 0, sizeof(msg_info));

					data_len=read(fd, (struct message*)&msg_info, sizeof(msg_info));
					
					if(data_len>0) writeMessage((void*)&msg_info, (void*)clients, fd, fdmax);
					
					else if(data_len==0){
						for(i=0;i<MAX;i++){
							if(clients[i].fd==fd){
								clients[i].fd=0;
								strcpy(clients[i].name,"");
								break;
							}
						}


						close(fd);

						FD_CLR(fd, &readset);

						if(fdmax==fd) fdmax--;

						printf("Client at fd %d is out\n", fd);
					}

					else if(data_len<0){
						perror("read error\n");
						exit(1);
					}
				}
			}
		}
	}

	return 0;
}


void writeMessage(void* client_msg, void* num, int base_fd, int max_fd){
	int index;
	struct message* msg;
	struct client* index_num;
	char all[]="ALL";

	msg=(struct message*)client_msg;
	index_num=(struct client*)num;

	//클라이언트가 접속했다는 메시지
	if(strcmp(msg->msg,"")==0){
		printf("등록하겠습니다\n");
		for(index=0;index<MAX;index++){
			if(((index_num+index)->fd)==base_fd){
				strcpy((index_num+index)->name, (msg->toUser));
			}
		}
		write(base_fd, login, sizeof(login));
	}

	//클라이언트가 다른 클라이언트에 메시지 전송 시
	else{
		all[strlen(all)]="\0";
		
		//to 모든 사용자
		if(strcmp(msg->toUser, all)==0){
			for(index=0;index<max_fd;index++){
				if((index_num+index)->fd!=0) write(((index_num+index)->fd), msg->msg, MAXBUF);
			}
		}

		//to 지정 사용자
		else{
			for(index=0;index<MAX;index++){
				if(strcmp(((index_num+index)->name), msg->toUser)==0){
					if(write(((index_num+index)->fd), msg->msg, MAXBUF)<0){
						perror("write error: ");
						exit(1);
					}
					break;
				}

				//유저가 존재하지 않을 시
				if(index+1==MAX) write(base_fd, not_login, sizeof(not_login));
			}
		}
	
	}
}
