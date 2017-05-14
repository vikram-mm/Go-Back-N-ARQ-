#include<netinet/in.h>
#include<errno.h>
#include<netdb.h>
#include<stdio.h> //For standard things
#include<stdlib.h> //malloc
#include<string.h> //strlen

#include<netinet/ip_icmp.h> //Provides declarations for icmp header
#include<netinet/udp.h> //Provides declarations for udp header
#include<netinet/tcp.h>  //Provides declarations for tcp header
#include<netinet/ip.h>   //Provides declarations for ip header
#include<netinet/if_ether.h>  //For ETH_P_ALL
#include<net/ethernet.h>  //For ether_header
#include<sys/socket.h>
#include<arpa/inet.h>
#include<sys/ioctl.h>
#include<sys/time.h>
#include<sys/types.h>
#include<unistd.h>
#include "send_raw.h"

void ProcessPacket(unsigned char* , int);
void print_ip_header(unsigned char* , int);
void print_tcp_packet(unsigned char * , int );
void PrintData(unsigned char*,int);
void printpayload(unsigned char*,int);
int checkmac(unsigned char*);
int checkack(unsigned char*);

FILE *logfile;
struct sockaddr_in source,dest;
int tcp=0,i,j;
long get_seq(unsigned char*);
int main()
{
int saddr_size,data_size,i;
struct sockaddr saddr;

unsigned char *buffer = (unsigned char *) malloc(65536);//Its Big!
int ack_num=2000;

    logfile=fopen("logGV.txt","w");
    if(logfile==NULL)
    {
    printf("Unable to create log.txt file.");
    }
    printf("Starting...\n");

    int sock_raw=socket(AF_PACKET,SOCK_RAW,htons(ETH_P_ALL));
    //setsockopt(sock_raw , SOL_SOCKET , SO_BINDTODEVICE , "eth0" , strlen("eth0")+1);

    if(sock_raw < 0)
    {
    //Print the error with proper message
    perror("Socket Error");
    return 1;
    }
	int internal_seq=0;
    while(1)
    {
        saddr_size = sizeof saddr;
        //Receive a packet
        data_size = recvfrom(sock_raw,buffer,65536,0,&saddr,(socklen_t*)&saddr_size);
        if(data_size<0)
        {
            printf("Recvfrom error , failed to get packets\n");
            return 1;
        }
        else if (checkmac(buffer))
          {  printpayload(buffer,data_size);
            printf("Sending Acknowledge\n");
                int seq=get_seq(buffer);
                printf("sent Acknowledgement %d",seq);
                      send_frame(0,internal_seq,seq+1);
			internal_seq++;
                     // ++ack_num;
          }


        //Now process the packet
        ProcessPacket(buffer,data_size);
    }
    close(sock_raw);
    printf("Finished");
    return 0;
}
long get_seq(unsigned char* Buffer)
{
  struct ethhdr *eth = (struct ethhdr *)Buffer;
        //printf( "   |-Source Address      : %.2X-%.2X-%.2X-%.2X-%.2X-%.2X \n", eth->h_source[0] , eth->h_source[1] , eth->h_source[2] , eth->h_source[3] , eth->h_source[4], eth->h_source[5] );
      struct iphdr *iph = (struct iphdr *)( Buffer + sizeof(struct ethhdr) );
    int iphdrlen = iph->ihl*4;
    
    struct tcphdr *tcph=(struct tcphdr*)(Buffer + iphdrlen + sizeof(struct ethhdr));
       int header_size = sizeof(struct ethhdr) + iphdrlen + tcph->doff*4;

       return ntohl(tcph->seq);
}
int checkmac(unsigned char* Buffer)
{
        struct ethhdr *eth = (struct ethhdr *)Buffer;
        //printf( "   |-Source Address      : %.2X-%.2X-%.2X-%.2X-%.2X-%.2X \n", eth->h_source[0] , eth->h_source[1] , eth->h_source[2] , eth->h_source[3] , eth->h_source[4], eth->h_source[5] );
      struct iphdr *iph = (struct iphdr *)( Buffer + sizeof(struct ethhdr) );
    int iphdrlen = iph->ihl*4;
    
    struct tcphdr *tcph=(struct tcphdr*)(Buffer + iphdrlen + sizeof(struct ethhdr));
       int header_size = sizeof(struct ethhdr) + iphdrlen + tcph->doff*4;

  
        
        if(eth->h_source[0]!=0x14)
            return 0;
        if(eth->h_source[1]!=0x2D)
            return 0;
        if(eth->h_source[2]!=0x27)
            return 0;
        if(eth->h_source[3]!=0xD6)
            return 0;
        if(eth->h_source[4]!=0xC5)
            return 0;
        if(eth->h_source[5]!=0x8F)
            return 0;



        return 1;

}



void printpayload(unsigned char* Buffer, int Size)
{
    unsigned short iphdrlen;
    
    struct iphdr *iph = (struct iphdr *)( Buffer + sizeof(struct ethhdr) );
    iphdrlen = iph->ihl*4;
    
    struct tcphdr *tcph=(struct tcphdr*)(Buffer + iphdrlen + sizeof(struct ethhdr));
    int header_size = sizeof(struct ethhdr) + iphdrlen + tcph->doff*4;

    int i;
    

    if(checkmac(Buffer)&&ntohl(tcph->seq)<=4000&&ntohl(tcph->seq)>=2000)
   { 
    printf("data frame received\n");
    printf("sequence number : %u\n,payload : ",ntohl(tcph->seq));
    for(i=0;i<Size-header_size;++i)
        if(*(Buffer+header_size+i)!='\n')
           printf("%c",*(Buffer+header_size+i));
       printf("\n");
   }

}
void ProcessPacket(unsigned char* buffer, int size)
{
    //Get the IP Header part of this packet , excluding the ethernet header
    struct iphdr *iph = (struct iphdr*)(buffer + sizeof(struct ethhdr));
        if (iph->protocol == 6) //Check the TCP Protocol and do accordingly...
          {
             print_tcp_packet(buffer,size);
            }
    }

void print_ethernet_header(unsigned char* Buffer, int Size)
{
    struct ethhdr *eth = (struct ethhdr *)Buffer;
    
    fprintf(logfile,"\n");
    fprintf(logfile, "Ethernet Header\n");
    fprintf(logfile,"   |-Destination Address : %.2X-%.2X-%.2X-%.2X-%.2X-%.2X \n", eth->h_dest[0] , eth->h_dest[1] , eth->h_dest[2] , eth->h_dest[3] , eth->h_dest[4] , eth->h_dest[5]);
    fprintf(logfile, "   |-Source Address      : %.2X-%.2X-%.2X-%.2X-%.2X-%.2X \n", eth->h_source[0] , eth->h_source[1] , eth->h_source[2] , eth->h_source[3] , eth->h_source[4], eth->h_source[5] );
    fprintf(logfile, "   |-Protocol            : %u \n",(unsigned short)eth->h_proto);
}

void print_ip_header(unsigned char* Buffer, int Size)
{
    print_ethernet_header(Buffer , Size);

    unsigned short iphdrlen;
        
    struct iphdr *iph=(struct iphdr *)(Buffer+sizeof(struct ethhdr));
    iphdrlen =iph->ihl*4;
    
    memset(&source, 0, sizeof(source));
    source.sin_addr.s_addr = iph->saddr;
    
    memset(&dest, 0, sizeof(dest));
    dest.sin_addr.s_addr = iph->daddr;

    fprintf(logfile, "\n");
    fprintf(logfile, "IP Header\n");
    fprintf(logfile, "   |-IP Version        : %d\n",(unsigned int)iph->version);
    fprintf(logfile, "   |-IP Header Length  : %d DWORDS or %d Bytes\n",(unsigned int)iph->ihl,((unsigned int)(iph->ihl))*4);
    fprintf(logfile, "   |-Type Of Service   : %d\n",(unsigned int)iph->tos);
    fprintf(logfile, "   |-IP Total Length   : %d  Bytes(Size of Packet)\n",ntohs(iph->tot_len));
    fprintf(logfile, "   |-Identification    : %d\n",ntohs(iph->id));
    //fprintf(logfile ,"   |-Reserved ZERO Field   : %d\n",(unsigned int)iphdr->ip_reserved_zero);
    //fprintf(logfile ,"   |-Dont Fragment Field   : %d\n",(unsigned int)iphdr->ip_dont_fragment);
    //fprintf(logfile ,"   |-More Fragment Field   : %d\n",(unsigned int)iphdr->ip_more_fragment);
    fprintf(logfile, "   |-TTL      : %d\n",(unsigned int)iph->ttl);
    fprintf(logfile, "   |-Protocol : %d\n",(unsigned int)iph->protocol);
    fprintf(logfile, "   |-Checksum : %d\n",ntohs(iph->check));
    fprintf(logfile, "   |-Source IP        : %s\n",inet_ntoa(source.sin_addr));
    fprintf(logfile, "   |-Destination IP   : %s\n",inet_ntoa(dest.sin_addr));
}

void print_tcp_packet(unsigned char* Buffer, int Size)
{
    unsigned short iphdrlen;
    
    struct iphdr *iph = (struct iphdr *)( Buffer + sizeof(struct ethhdr) );
    iphdrlen = iph->ihl*4;
    
    struct tcphdr *tcph=(struct tcphdr*)(Buffer + iphdrlen + sizeof(struct ethhdr));
            
    int header_size = sizeof(struct ethhdr) + iphdrlen + tcph->doff*4;
   
    fprintf(logfile , "\n\n***********************TCP Packet*************************\n");
        
    print_ip_header(Buffer,Size);
        
    fprintf(logfile, "\n");
    fprintf(logfile, "TCP Header\n");
    fprintf(logfile, "   |-Source Port      : %u\n",ntohs(tcph->source));
    fprintf(logfile, "   |-Destination Port : %u\n",ntohs(tcph->dest));
    fprintf(logfile, "   |-Sequence Number    : %u\n",ntohl(tcph->seq));
    fprintf(logfile, "   |-Acknowledge Number : %u\n",ntohl(tcph->ack_seq));
    fprintf(logfile, "   |-Header Length      : %d DWORDS or %d BYTES\n" ,(unsigned int)tcph->doff,(unsigned int)tcph->doff*4);
    //fprintf(logfile , "   |-CWR Flag : %d\n",(unsigned int)tcph->cwr);
    //fprintf(logfile , "   |-ECN Flag : %d\n",(unsigned int)tcph->ece);
    fprintf(logfile, "   |-Urgent Flag          : %d\n",(unsigned int)tcph->urg);
    fprintf(logfile, "   |-Acknowledgement Flag : %d\n",(unsigned int)tcph->ack);
    fprintf(logfile, "   |-Push Flag            : %d\n",(unsigned int)tcph->psh);
    fprintf(logfile, "   |-Reset Flag           : %d\n",(unsigned int)tcph->rst);
    fprintf(logfile, "   |-Synchronise Flag     : %d\n",(unsigned int)tcph->syn);
    fprintf(logfile, "   |-Finish Flag          : %d\n",(unsigned int)tcph->fin);
    fprintf(logfile, "   |-Window         : %d\n",ntohs(tcph->window));
    fprintf(logfile, "   |-Checksum       : %d\n",ntohs(tcph->check));
    fprintf(logfile, "   |-Urgent Pointer : %d\n",tcph->urg_ptr);
    fprintf(logfile, "\n");
    fprintf(logfile, "                        DATA Dump                         ");
    fprintf(logfile, "\n");
        
    fprintf(logfile, "IP Header\n");
    PrintData(Buffer,iphdrlen);
        
    fprintf(logfile, "TCP Header\n");
    PrintData(Buffer+iphdrlen,tcph->doff*4);
        
    fprintf(logfile, "Data Payload\n");
    PrintData(Buffer + header_size , Size - header_size );

        fprintf(logfile, "Data Payload\n");
    PrintData(Buffer + header_size , Size - header_size );

        fprintf(logfile, "Data Payload\n");
    PrintData(Buffer + header_size , Size - header_size );

        fprintf(logfile, "Data Payload\n");
    PrintData(Buffer + header_size , Size - header_size );
                        
    fprintf(logfile, "\n###########################################################");
}

void PrintData(unsigned char* data , int Size)
{
    int i , j;
    for(i=0 ; i < Size ; i++)
    {
        if(i!=0&&i%16==0)  //if one line of hex printing is complete...
        {
            fprintf(logfile,"        ");
            for(j=i-16;j<i;j++)
            {
                if(data[j]>=32&&data[j]<=128)
                    fprintf(logfile,"%c",(unsigned char)data[j]); //if its a number or alphabet
                
                else fprintf(logfile,"."); //otherwise print a dot
            }
            fprintf(logfile,"\n");
        }
        
        if(i%16==0) fprintf(logfile,"  ");
            fprintf(logfile,"%02X",(unsigned int)data[i]);
                
        if(i==Size-1) //print the last spaces
        {
            for(j=0;j<15-i%16;j++)
            {
              fprintf(logfile,"  "); //extra spaces
            }
            
            fprintf(logfile,"        ");
            
            for(j=i-i%16;j<=i;j++)
            {
                if(data[j]>=32&&data[j]<=128)
                {
                  fprintf(logfile,"%c",(unsigned char)data[j]);
                }
                else
                {
                  fprintf(logfile,".");
                }
            }
            
            fprintf(logfile,"\n");
        }
    }
}

