/*
 ============================================================================
 Name        : socket-uart.c
 Author      : Eric
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include <fcntl.h>
#include <termios.h>
#include <pthread.h>

#include "serial.h"
#include "tcp.h"

#define  DEVICE      "/dev/ttyS1"
#define  BUF_SIZE    256

#define  SOCKET_USRT_NUMBER		2
#define  SOCKET_USRT_0		0
#define  SOCKET_USRT_1		1

#define PORT0             8899
#define PORT1             9988

#define QUEUE   1
#define BUFFER_SIZE 1024


typedef struct _trasnfer_t{
	int    		usrt_fd;
	char			com[200];
	pthread_t 	usrt_thr;

	int    		sock_fd;
	pthread_t 	sock_thr;
}trasnfer_t;


trasnfer_t socket_usrt[SOCKET_USRT_NUMBER];

void usrt_to_socket_0 (void);
void usrt_to_socket_1 (void);
void port1_socket(void);
void port0_socket(void);

/* start main */
int main(int argc,char **argv)
{
    int    ret;
    char   *dev = DEVICE;
    get_local_ip();
    memcpy(socket_usrt[SOCKET_USRT_0].com, dev, 20);
    memcpy(socket_usrt[SOCKET_USRT_1].com, dev, 20);
//    socket_usrt[SOCKET_USRT_0].sock_fd = socket_init();

    socket_usrt[SOCKET_USRT_0].usrt_fd = open_dev(socket_usrt[SOCKET_USRT_0].com);
    if(socket_usrt[SOCKET_USRT_0].usrt_fd < 0){
    	printf("socket_usrt[SOCKET_USRT_0].usrt_fd error, main exit...\n");
    	exit(1);
    }
    set_parity(socket_usrt[SOCKET_USRT_0].usrt_fd,8,1,'N'); // 设置串口数据位、停止位、校验位
    set_speed(socket_usrt[SOCKET_USRT_0].usrt_fd,9600); // 设置串口波特率

    socket_usrt[SOCKET_USRT_1].usrt_fd = open_dev(socket_usrt[SOCKET_USRT_1].com);
    if(socket_usrt[SOCKET_USRT_1].usrt_fd < 0){
        	printf("socket_usrt[SOCKET_USRT_1].usrt_fd error, main exit...\n");
        	exit(1);
        }
    set_parity(socket_usrt[SOCKET_USRT_1].usrt_fd,8,1,'N'); // 设置串口数据位、停止位、校验位
    set_speed(socket_usrt[SOCKET_USRT_1].usrt_fd,9600); // 设置串口波特率


    ret = pthread_create(&socket_usrt[SOCKET_USRT_0].sock_thr,NULL,(void *)port0_socket,NULL); // 创建sock线程接收socket数据
    if(ret < 0){
        perror("phread_create");
        exit(1);
    }

    ret = pthread_create(&socket_usrt[SOCKET_USRT_0].usrt_thr,NULL,(void *)usrt_to_socket_0,NULL); // 创建usrt线程接收串口数据
    if(ret < 0){
        perror("phread_create");
        exit(1);
    }

    ret = pthread_create(&socket_usrt[SOCKET_USRT_1].sock_thr,NULL,(void *)port1_socket,NULL); // 创建sock线程接收socket数据
    if(ret < 0){
        perror("phread_create");
        exit(1);
    }

    ret = pthread_create(&socket_usrt[SOCKET_USRT_1].usrt_thr,NULL,(void *)usrt_to_socket_1,NULL); // 创建usrt线程接收串口数据
    if(ret < 0){
        perror("phread_create");
        exit(1);
    }
    pthread_join(socket_usrt[SOCKET_USRT_0].sock_thr,NULL); // 等待线程退出
    pthread_join(socket_usrt[SOCKET_USRT_0].usrt_thr,NULL);
    pthread_join(socket_usrt[SOCKET_USRT_1].sock_thr,NULL); // 等待线程退出
    pthread_join(socket_usrt[SOCKET_USRT_1].usrt_thr,NULL);

    close(socket_usrt[SOCKET_USRT_0].sock_fd);
    close(socket_usrt[SOCKET_USRT_0].usrt_fd);
    close(socket_usrt[SOCKET_USRT_1].sock_fd);
    close(socket_usrt[SOCKET_USRT_1].usrt_fd);

    printf("main exit...\n");
    return 0;
}



void port0_socket(void)
{
    ///定义sockfd
    int server_sockfd = socket(AF_INET,SOCK_STREAM, 0);
    socket_usrt[SOCKET_USRT_0].sock_fd = server_sockfd;

    ///定义sockaddr_in
    struct sockaddr_in server_sockaddr;

    ///客户端套接字
    char buffer[BUFFER_SIZE];
    struct sockaddr_in client_addr;
    int len;

    socklen_t length;
    int conn;
    server_sockaddr.sin_family = AF_INET;
    server_sockaddr.sin_port = htons(PORT0);
    server_sockaddr.sin_addr.s_addr = htonl(INADDR_ANY);

    ///bind，成功返回0，出错返回-1
    if(bind(server_sockfd,(struct sockaddr *)&server_sockaddr,sizeof(server_sockaddr))==-1)
    {
        perror("bind failed");
        exit(1);
    }
    printf("port0_socket created\n");
    ///listen，成功返回0，出错返回-1
    if(listen(server_sockfd,QUEUE) == -1)
    {
        perror("listen");
        exit(1);
    }
    //Accept and incoming connection
	 printf("Waiting for incoming connections...");
    ///成功返回非负描述字，出错返回-1
    length = sizeof(client_addr);
    while(1){
		conn = accept(server_sockfd, (struct sockaddr*)&client_addr, &length);
		if(conn<0)
		{
			perror("accept failed");
			exit(1);
		}

		while(1)
		{
			memset(buffer,0,sizeof(buffer));
			len = recv(conn, buffer, sizeof(buffer),0);
	//        if(strcmp(buffer,"exit\n")==0)
	//            break;
			fputs(buffer, stdout);
			if(len <= 0){
				continue;
			}
			write(socket_usrt[SOCKET_USRT_0].usrt_fd, buffer, len);
		}
    }
    close(conn);
    close(server_sockfd);
    pthread_exit(0);
}

void port1_socket(void)
{
    ///定义sockfd
    int server_sockfd = socket(AF_INET,SOCK_STREAM, 0);
    socket_usrt[SOCKET_USRT_1].sock_fd = server_sockfd;

    ///定义sockaddr_in
    struct sockaddr_in server_sockaddr;

    ///客户端套接字
    char buffer[BUFFER_SIZE];
    struct sockaddr_in client_addr;
    int len;

    socklen_t length;
    int conn;
    server_sockaddr.sin_family = AF_INET;
    server_sockaddr.sin_port = htons(PORT1);
    server_sockaddr.sin_addr.s_addr = htonl(INADDR_ANY);

    ///bind，成功返回0，出错返回-1
    if(bind(server_sockfd,(struct sockaddr *)&server_sockaddr,sizeof(server_sockaddr))==-1)
    {
        perror("bind failed");
        exit(1);
    }
    printf("port0_socket created\n");
    ///listen，成功返回0，出错返回-1
    if(listen(server_sockfd,QUEUE) == -1)
    {
        perror("listen");
        exit(1);
    }
    //Accept and incoming connection
	 printf("Waiting for incoming connections...");
    ///成功返回非负描述字，出错返回-1
    length = sizeof(client_addr);
    while(1){
		conn = accept(server_sockfd, (struct sockaddr*)&client_addr, &length);
		if(conn<0)
		{
			perror("accept failed");
			exit(1);
		}

		while(1)
		{
			memset(buffer,0,sizeof(buffer));
			len = recv(conn, buffer, sizeof(buffer),0);
	//        if(strcmp(buffer,"exit\n")==0)
	//            break;
			fputs(buffer, stdout);
			if(len <= 0){
				continue;
			}
			write(socket_usrt[SOCKET_USRT_1].usrt_fd, buffer, len);
		}
    }
    close(conn);
    close(server_sockfd);
    pthread_exit(0);
}
/**************************************************************************************
 *  Description:接收串口数据并通过socket转发
 *   Input Args:
 *  Output Args:
 * Return Value:
 *************************************************************************************/
void usrt_to_socket_0 (void)
{
    printf("start usrt_to_socket 0...\n");
    int     n = 0;
    int     m = 0;
    char    buf[BUF_SIZE];
    printf("usrt_to_socket_0 created\n");
    while(1)
    {
        memset(buf,0,BUF_SIZE);
        n = read(socket_usrt[SOCKET_USRT_0].usrt_fd,buf,BUF_SIZE);
        if(n>0)
        {
//            if(0==strncmp(buf,"exit",4)) // 收到exit时，线程退出
//            {
//                m = write(socket_usrt[SOCKET_USRT_0].sock_fd,buf,BUF_SIZE);
//                if(m<0)
//                {
//                    perror("write to socket_usrt[SOCKET_USRT_0].sock_thr");
//                }
//                break;
//            }

//            m = write(socket_usrt[SOCKET_USRT_0].sock_fd,buf,BUF_SIZE);
        	m = send(socket_usrt[SOCKET_USRT_0].sock_fd,buf,n,0);
            if(m<0)
            {
                perror("write to socket_usrt[SOCKET_USRT_0].sock_thr");
            }
        }
        else if(n<0)
        {
            perror("read from socket_usrt[SOCKET_USRT_0].usrt_thr");
        }
    }

    printf("usrt_to_socket 0 exit...\n");
    pthread_exit(0);
} /* ----- End of usrt_to_sock()  ----- */


/**************************************************************************************
 *  Description:接收串口数据并通过socket转发
 *   Input Args:
 *  Output Args:
 * Return Value:
 *************************************************************************************/
void usrt_to_socket_1 (void)
{
    printf("start usrt_to_socket 1...\n");
    int     n = 0;
    int     m = 0;
    char    buf[BUF_SIZE];
    printf("usrt_to_socket_1 created\n");

    while(1){
        memset(buf,0,BUF_SIZE);
        n = read(socket_usrt[SOCKET_USRT_1].usrt_fd,buf,BUF_SIZE);
        if(n>0){
//            if(0==strncmp(buf,"exit",4)) // 收到exit时，线程退出
//            {
//                m = write(socket_usrt[SOCKET_USRT_1].sock_fd,buf,BUF_SIZE);
//                if(m<0){
//                    perror("write to socket_usrt[SOCKET_USRT_1].sock_thr");
//                }
//                break;
//            }

//            m = write(socket_usrt[SOCKET_USRT_1].sock_fd,buf,BUF_SIZE);
        	m = send(socket_usrt[SOCKET_USRT_1].sock_fd,buf,n,0);
            if(m<0){
                perror("write to socket_usrt[SOCKET_USRT_1].sock_thr");
            }
        }
        else if(n<0){
            perror("read from socket_usrt[SOCKET_USRT_1].usrt_thr");
        }
    }

    printf("usrt_to_socket 1 exit...\n");
    pthread_exit(0);
} /* ----- End of usrt_to_sock()  ----- */

