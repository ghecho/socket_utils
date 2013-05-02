//
//  socket_utils.h
//  multiUserChat
//
//
//  Created by Diego on 3/10/13.
//  Copyright (c) 2013 Diego. All rights reserved.
//
//  Licenced under The MIT License
//
//  Permission is hereby granted, free of charge, to any person obtaining a copy
//  of this software and associated documentation files (the "Software"), to deal
//  in the Software without restriction, including without limitation the rights
//  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//  copies of the Software, and to permit persons to whom the Software is
//  furnished to do so, subject to the following conditions:
//
//  The above copyright notice and this permission notice shall be included in
//  all copies or substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
//  THE SOFTWARE.
//

#ifndef socket_utils_h
#define socket_utils_h

#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#define BUFLEN 512
#define SERVER_PORT 26662 //THE SERVER PORT TO LISTEN

#define SERVER_IP "127.0.0.1" //INSERT_THE_SERVER_IP

void diep(char *s)
{
    perror(s);
    exit(1);
}

void getString(char *destinationString)
{
    char *auxPtr;
    fgets(destinationString, BUFLEN, stdin);
    for (auxPtr = destinationString; *auxPtr != '\n' && *auxPtr != '\0'; ++auxPtr);
    *auxPtr = '\0';
}

typedef struct STRUCT_SOCKET
{
    struct sockaddr_in si;
    int s;
    unsigned slen;
}SOCKET;

typedef enum
{
    TEXT_MESSAGE,
    REGISTER_USER,
    TERMINATE_SESSION_REQUEST,
    TERMINATE_SESSION_ORDER
} MESSAGE_TYPE;

void imprimeTipo(MESSAGE_TYPE m)
{
    if(m==TEXT_MESSAGE)
    {
        printf("tipo: TEXT_MESSAGE\n");
    }
    else if(m==REGISTER_USER)
    {
        printf("tipo: REGISTER_USER\n");
    }
    else if(m==TERMINATE_SESSION_ORDER)
    {
        printf("tipo: TERMINATE_SESSION_ORDER\n");
    }
    else if(m==TERMINATE_SESSION_REQUEST)
    {
        printf("tipo: TERMINATE_SESSION_REQUEST\n");
    }
    else
    {
        printf("tipo: DESCONOCIDO\n");
    }
}

/*

THIS IS USED FOR A GAME BUT CAN BE EASILY ADAPTED TO WORK WITH ANY DATA

JUST CHANGE THE WRAPPER STRUCT AND CHANGE IT IN STRUCT_UDP_DATA

*/

#define TAM 5

typedef struct STRUCT_JUEGO
{
    int turno;
    char symbol;
    char tablero[TAM][TAM];
    int casillasLlenas;
    int acabo;
    int empate;
}JUEGO;

typedef struct STRUCT_UDP_DATA
{
    MESSAGE_TYPE type;
    int client_id;

    //CHANGE THIS TO WORK WITH OTHER TYPE OF DATA
    char data[BUFLEN];//this can be a message to/from the client/server
    JUEGO juego;//data to send

}UDP_DATA;

#define MAX_CLIENTS 10

typedef struct STRUCT_CLIENT
{
    char used;
    int client_id;
    SOCKET socket;
}CLIENT;

int SetSocketBlockingEnabled(int fd, int blocking)
{
   if (fd < 0) return 0;
  int flags = fcntl(fd, F_GETFL, 0);
  if (flags < 0) return 0;
  flags = blocking ? (flags&~O_NONBLOCK) : (flags|O_NONBLOCK);
  return (fcntl(fd, F_SETFL, flags) == 0) ? 1 : 0;
}


void init_socket_client(SOCKET *_socket,UDP_DATA *_packet)
{
    _socket->slen=sizeof(struct sockaddr_in);
    
    if ((_socket->s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP))==-1)
        diep("socket");
    
    memset((char *) &(_socket->si), 0, sizeof(struct sockaddr_in));
    _socket->si.sin_family = AF_INET;
    _socket->si.sin_port = htons(SERVER_PORT);
    if (inet_aton(SERVER_IP, &(_socket->si.sin_addr))==0)
    {
        fprintf(stderr, "inet_aton() failed\n");
        exit(1);
    }
    
    _packet->type=REGISTER_USER;
    
    printf("Initialization of socket complete\n");
}

void send_client(SOCKET *_socket,UDP_DATA *_packet)
{
    _socket->slen=sizeof(struct sockaddr_in);
    if (sendto(_socket->s, _packet, sizeof(UDP_DATA), 0, (struct sockaddr *)&(_socket->si), _socket->slen)==-1)
        diep("sendto()");
}

void recieve_client(SOCKET *_socket,UDP_DATA *_packet)
{
    _socket->slen=sizeof(struct sockaddr_in);
    if (recvfrom(_socket->s, _packet, sizeof(UDP_DATA), 0, (struct sockaddr *)&(_socket->si), &(_socket->slen))==-1)
        diep("recvfrom()");
    imprimeTipo(_packet->type);
    printf("Message from the server: %s\n\n",_packet->data);
}

void init_socket_server(SOCKET *_socket,UDP_DATA *_packet,CLIENT clients[MAX_CLIENTS])
{
    if ((_socket->s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP))==-1)
        diep("socket");
    
    memset((char *) &(_socket->si), 0, sizeof(struct sockaddr_in));
    _socket->si.sin_family = AF_INET;
    _socket->si.sin_port = htons(SERVER_PORT);
    _socket->si.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind(_socket->s, (struct sockaddr *)&(_socket->si), sizeof(struct sockaddr_in))==-1)
        diep("bind");
    
    _socket->slen=sizeof(struct sockaddr_in);
    
    _packet->data[0]='\0';
    
    int i;
    for(i=0;i<MAX_CLIENTS;i++)
    {
        clients[i].used=0;
    }
}

void send_server(SOCKET *_socket_me,SOCKET *_socket_other,UDP_DATA *_packet)
{
    if (sendto(_socket_me->s, _packet, sizeof(UDP_DATA), 0, (struct sockaddr *)&(_socket_other->si), _socket_other->slen)==-1)
        diep("sendto()");
}

void recieve_server(SOCKET *_socket_me,SOCKET *_socket_other,UDP_DATA *_packet)
{
    _socket_other->slen=sizeof(struct sockaddr_in);
    if (recvfrom(_socket_me->s, _packet, sizeof(UDP_DATA), 0, (struct sockaddr *)&(_socket_other->si), &(_socket_other->slen))==-1)
        diep("recvfrom()");
}

void insert_user(int client_id,CLIENT clients[MAX_CLIENTS],SOCKET *_socket)
{
    int i;
    for(i=0;i<MAX_CLIENTS;i++)
    {
        if(clients[i].used==0)
        {
            clients[i].used=1;
            clients[i].client_id=client_id;
            memcpy(&(clients[i].socket),_socket,sizeof(SOCKET));
            printf("user:%d registered\n",i);
            break;
        }
    }
}

void delete_user(int client_id,CLIENT clients[MAX_CLIENTS])
{
    int i;
    for(i=0;i<MAX_CLIENTS;i++)
    {
        if(clients[i].client_id==client_id)
        {
            clients[i].used=0;
            printf("user:%d deleted\n",client_id);
            break;
        }
    }
}

void send_message_to_all_except_user(SOCKET *_socket_me,CLIENT clients[MAX_CLIENTS],UDP_DATA *_packet)
{
    int i;
    for(i=0;i<MAX_CLIENTS;i++)
    {
        if(clients[i].used==1)
        {
            if(clients[i].client_id!=_packet->client_id)
            {
                usleep(3000);//timeout to avoid buffer mismatches
                send_server(_socket_me,&(clients[i].socket),_packet);
            }
        }
    }
}

void send_message_to_all(SOCKET *_socket_me,CLIENT clients[MAX_CLIENTS],UDP_DATA *_packet)
{
    int i;
    for(i=0;i<MAX_CLIENTS;i++)
    {
        if(clients[i].used==1)
        {
            usleep(3000);//timeout to avoid buffer mismatches
            send_server(_socket_me,&(clients[i].socket),_packet);
            printf("message sent to user:%d\n",i);
        }
    }
}

void close_socket(SOCKET *_socket)
{
    close(_socket->s);
}

#endif
