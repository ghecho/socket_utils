//
//  server.c
//  multiUserChatServer
//
//  Created by Diego on 3/10/13.
//  Copyright (c) 2013 Diego. All rights reserved.
//

#include "../socket_utils.h"

SOCKET me, other;
CLIENT clients[MAX_CLIENTS];
UDP_DATA packet_from_client,packet_to_broadcast;

int main(int argc, char** argv)
{
    packet_from_client.juego.acabo=0;
    packet_from_client.juego.empate=0;
    packet_from_client.juego.turno=1;
    init_socket_server(&me, &packet_from_client,clients);
    
    packet_from_client.juego.acabo=0;
    packet_to_broadcast.juego.acabo=0;
    
    while(1)
    {
        recieve_server(&me, &other, &packet_from_client);
        imprimeTipo(packet_from_client.type);
        if(packet_from_client.type==REGISTER_USER)
        {
            printf("register new user\n");
            insert_user(packet_from_client.client_id,clients,&other);
        }
        else
        {
            memcpy(&packet_to_broadcast,&packet_from_client,sizeof(UDP_DATA));
            //server
            printf("pre turno: %d\n",packet_to_broadcast.juego.turno);
            if(packet_to_broadcast.juego.symbol == 'X')
                packet_to_broadcast.juego.symbol = 'O';
            else if(packet_to_broadcast.juego.symbol == 'O')
                packet_to_broadcast.juego.symbol = 'G';
            else
                packet_to_broadcast.juego.symbol = 'X';
            
            if(packet_to_broadcast.juego.turno == 1)
                packet_to_broadcast.juego.turno = 2;
            else if(packet_to_broadcast.juego.turno == 2)
                packet_to_broadcast.juego.turno = 3;
            else
                packet_to_broadcast.juego.turno = 1;
            
            printf("post turno: %d\n",packet_to_broadcast.juego.turno);
            
            printf("recieved text message\n");
            packet_to_broadcast.type=TEXT_MESSAGE;
            send_message_to_all(&me,clients,&packet_to_broadcast);
        }
    }

    close_socket(&me);
    return 0;
}