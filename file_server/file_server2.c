/*--------------------------------------------------------------------------------------------------------
파일명 : file_server2.c 
기  능 : file 전송 서비스를 수행하는 서버
컴파일 : cc -w -o file_server2 file_server2.c
사용법 : file_server2 port
--------------------------------------------------------------------------------------------------------*/
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>

#define BUF_LEN 128
int main(int argc, char *argv[]) {
	struct sockaddr_in server_addr, client_addr;
	int server_fd, client_fd;			/* 소켓번호 */
	int len, msg_size,j;
	char buf[BUF_LEN+1];
	

	unsigned int set = 1;

	if(argc != 2) {
		printf("usage: %s port\n", argv[0]);
		exit(0);
	}
	/* 소켓 생성 */
	if((server_fd = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
		printf("Server: Can't open stream socket.");
		exit(0);
	}

	printf("server_fd = %d\n", server_fd);

	setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, (char *)&set, sizeof(set));

	/* server_addr을 '\0'으로 초기화 */
	bzero((char *)&server_addr, sizeof(server_addr));
	/* server_addr 세팅 */
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	server_addr.sin_port = htons(atoi(argv[1]));

	/* bind() 호출 */
	if(bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
		printf("Server: Can't bind local address.\n");
		exit(0);
	}

	/* 소켓을 수동 대기모드로 세팅 */
	listen(server_fd, 5);

	/* iterative  echo 서비스 수행 */
	printf("Server : waiting connection request.\n");
	len = sizeof(client_addr);

	/* 연결요청을 기다림 */
	while (1) 
	{
		char buf_f[128];
		char buf_o[128];
	
		FILE *fp;

		client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &len);
		printf("client_fd = %d\n", client_fd);
		if(client_fd < 0) {
			printf("Server: accept failed.\n");
			exit(0);
		}

		printf("Server : A client connected.\n");
		printf("From %s : %d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
		
		//put, get 구분하기 ==> 공백체크, 공백까지 버퍼 복붙, 보내기 두번
		//서버는 받기 두번. 첫 버퍼에서 put, get 체크. 그후 파일 보내기또는 받기

		if (recv(client_fd, buf_o, 128, 0)<=0) {
			printf("order error\n");
			exit(0);
		}
		if (recv(client_fd, buf_f, 128, 0) <= 0) {
			printf("filename error\n");
			exit(0);
		}
		printf("filename : %s\n", buf_f);

		if (strcmp("put", buf_o) == 0) {
			j = 0;
			if ((fp = fopen(buf_f, "wb")) == NULL) {//서버. 쓰기모드 오픈
				printf("file open error\n");
				exit(0);
			}
		}
		else if (strcmp("get", buf_o) == 0) {
			j = 1;
			fp = fopen(buf_f, "rb"); // read mode open
			if (fp == NULL) {
				printf("File open error\n");
				exit(0);
			}
		}
		else {
			j = 2;
			printf("order error\n");
		}

		if (j != 2) {
			switch (j)
			case 0: {
				while (1) {
					int i, len;
					msg_size = recv(client_fd, buf, BUF_LEN, 0);
					if (msg_size <= 0) // end of file
						break;
					printf("read data = %d bytes : %s", msg_size, buf);
					if (fwrite(buf, msg_size, 1, fp) == NULL) {
						printf("fwrite error\n");
						break;
					}
				}
			}
			case 1: {
				while (1) {
					int n;
					n = fread(buf, 1, 128, fp); // 파일을 읽어서
					if (n <= 0) // End of file
						break;
					if (send(client_fd, buf, n, 0) <= 0) { // 네트워크로 보낸다.
						printf("send error\n");
						break;
					}
				}
			}
		}
		fclose(fp);
		close(client_fd);
	}
	close(server_fd);
	return(0);
}

