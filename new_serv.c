/*  Copyright (C) 2012-2015  P.D. Buchan (pdbuchan@yahoo.com)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

// Send an IPv4 HTTP GET packet via raw TCP socket at the link layer (ethernet frame).
// Need to have destination MAC address.
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
#include <stdio.h>
#include<pthread.h>
     // errno, perror()
#include "send_raw.h"
// Define some constants.


// Function prototypes

void* recieve_frame();
int checkack(unsigned char*);
long get_ack(unsigned char*);
   int received_array[1000];
   void send_slideWindow(int);
   int f;
int main (int argc, char **argv)
{
  int i;

  printf("enter the number of frames\n");
   scanf("%d",&f);



   pthread_t rthread;

pthread_create(&rthread,NULL,recieve_frame,NULL);
  
//   int k=0;
//   l1:
//   for(i=k;i<k+5;++i)
//   {
//     //alpha9(i);

//     send_frame(1000+i,i,0);
//     printf("sending frame sequence number %d \n",i);

//     // if((i+1)%5==0)
//     //   sleep(1);
    
//  }
//  l2:
//  if(received_array[k]==1)
//  { k++;

//   if(k==f-1)
//     goto l4;

//    goto l3;
//    }
//    else
//     goto l1;

//    l3:
//    send_frame(1000+k+4,k+4,0);
//    goto l2;

// l4:

int k=0;
send_slideWindow(0);

while(k<f)
{
  if(received_array[k]==1)
  {
    //printf("have received %d \n",k);
         if(k+5<f)
         {   send_frame(1000+k+5,k+5,0);
              printf("sending frame sequence number %d \n",k+5);
              sleep(1);}
          k++;
  }
  else
  {
    int i;
    printf("current recived status :\n");
    for(i=0;i<f;++i)
      printf("%d ",received_array[i]);
    printf("have not received acknowledgement : %d\n",k+1);
    printf("\nresending from %d \n",k);
    for(i=k;i<k+5;++i)
      received_array[i]=0;
    send_slideWindow(k);
    //k++;
  }
}
pthread_join(rthread,NULL);

}

void send_slideWindow(int start)
{
  printf("sent window starting from %d \n",start);
  int i;
  int minx;
  if(start+5<f)
    minx=start+5;
  else
    minx=f;

  for(i=start;i<minx;++i)
  {
     send_frame(1000+i,i,0);
    printf("sending frame sequence number %d \n",i);
  }

  sleep(1);
}



void* recieve_frame()
{
  int saddr_size,data_size,i;
struct sockaddr saddr;

unsigned char *buffer = (unsigned char *) malloc(65536);//Its Big!
   int sock_raw=socket(AF_PACKET,SOCK_RAW,htons(ETH_P_ALL));
    //setsockopt(sock_raw , SOL_SOCKET , SO_BINDTODEVICE , "eth0" , strlen("eth0")+1);

    if(sock_raw < 0)
    {
    //Print the error with proper message
    perror("Socket Error");
    //return 1;
    }
    while(1)
    {
        saddr_size = sizeof saddr;
        //Receive a packet
        data_size = recvfrom(sock_raw,buffer,65536,0,&saddr,(socklen_t*)&saddr_size);
        if(data_size<0)
        {
            printf("Recvfrom error , failed to get packets\n");
            
        }
        else if (checkack(buffer))
          {  fprintf(stderr,"acknowledgement %ld received\n",get_ack(buffer));
              if(get_ack(buffer)<1000&&get_ack(buffer)<f+1)
                received_array[get_ack(buffer)-1]=1;
          }
        //Now process the packet
        //ProcessPacket(buffer,data_size);
    }
    close(sock_raw);
}
long get_ack(unsigned char* Buffer)
{
  struct ethhdr *eth = (struct ethhdr *)Buffer;
        //printf( "   |-Source Address      : %.2X-%.2X-%.2X-%.2X-%.2X-%.2X \n", eth->h_source[0] , eth->h_source[1] , eth->h_source[2] , eth->h_source[3] , eth->h_source[4], eth->h_source[5] );
      struct iphdr *iph = (struct iphdr *)( Buffer + sizeof(struct ethhdr) );
    int iphdrlen = iph->ihl*4;
    
    struct tcphdr *tcph=(struct tcphdr*)(Buffer + iphdrlen + sizeof(struct ethhdr));
       int header_size = sizeof(struct ethhdr) + iphdrlen + tcph->doff*4;

       return ntohl(tcph->ack_seq);
}
int checkack(unsigned char* Buffer)
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

