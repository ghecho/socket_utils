#include <stdio.h>
#include <stdlib.h>
#define TAM 5

#include "../socket_utils.h"

int turno = 1;
int i,j;
int row,column;
UDP_DATA packet;


void imprimirTablero()
{
    printf("\n");
    for(i = 0; i < TAM ; i ++)
    {
        printf("%d ",i);
    }
    printf("\n\n");
    for (i = 0; i < TAM; i++)
    {
        for (j = 0; j < TAM; j++)
        {
            printf("%c ",packet.juego.tablero[i][j]);
        }
        printf("  %d",i);
        printf("\n");
    }
}

void imprimirTurno()
{
    system("clear");
    printf("\n Turno del jugador: %d\n\n",packet.juego.turno);
    imprimirTablero();
    int casillaValida = 0;
    while(!casillaValida)
    {
        printf("\nEscoja el renglon en el que desea tirar (0-%d): ",TAM-1);
        scanf("%d",&row);
        printf("\nEscoja la columna para el tiro (0-%d): ",TAM-1);
        scanf("%d",&column);
        if(row > TAM || row < 0 || column > TAM || column < 0 || packet.juego.tablero[row][column] != '@')
        {
            printf("\nTiro invalido, intente nuevamente");
            casillaValida = 0;
        }
        else
        {
            casillaValida = 1;
            packet.juego.tablero[row][column] = packet.juego.symbol;
            packet.juego.casillasLlenas = packet.juego.casillasLlenas + 1;
        }
    }
    
}

int validarGanador()
{
    int rowCopy = row + 1;
    int columnCopy = column + 1;
    int iguales = 0;
    for (i = 0; i < TAM; i++)
    {
        if (rowCopy == TAM)
            rowCopy = 0;
        if (packet.juego.tablero[rowCopy][column] == packet.juego.symbol)
        {
            iguales++;
            rowCopy++;
        }
        else
            break;
    }
    if (iguales >= 4)
        return 1;
    else
    {
        iguales = 0;
        for (i = 0; i < TAM - 1; i++)
        {
            if (columnCopy == TAM)
                columnCopy = 0;
            if (packet.juego.tablero[row][columnCopy]== packet.juego.symbol)
            {
                iguales++;
                columnCopy++;
            }
            else
                break;
        }
    }
    if (iguales >= 4)
        return 1;
    else
    {
        if (row == column)
        {
            iguales = 0;
            for(i = 0; i < TAM ; i++)
            {
                if(packet.juego.tablero[i][i] == packet.juego.symbol)
                    iguales++;
                else
                    break;
            }
            if(iguales >= 4)
                return 1;
        }
        else
        {
            iguales = 0;
            for(i = 0; i < TAM; i++)
            {
                if(packet.juego.tablero[i][(TAM -1)-i] == packet.juego.symbol)
                    iguales++;
                else
                    break;
                
            }
            if(iguales >= 4)
                return 1;
            else
                return 0;
        }
    }
    return 0;
}

int main(int argc, const char * argv[])
{
    pthread_t thread,thread_juego;
    SOCKET socket;
    
    init_socket_client(&socket, &packet);
    
    packet.juego.acabo=0;
    packet.juego.empate=0;
    packet.juego.casillasLlenas=0;
    
    packet.juego.turno=1;
    
    printf("numero de jugador(1,2,3):");
    scanf("%d",&turno);
    getchar();
    
    if(turno==1)
    {
        for (i = 0; i < TAM; i++)
        {
            for (j = 0; j < TAM; j++)
            {
                packet.juego.tablero[i][j]='@';
            }
        }
    }
    
    packet.client_id=turno;
    
    printf("\nsimbolo de jugador(X,O,G):");
    scanf("%c",&packet.juego.symbol);
    
    packet.type=REGISTER_USER;
    
    send_client(&socket,&packet);
    printf("\nInit done");
    
    packet.type=TEXT_MESSAGE;
    
    if(TAM < 4)
        printf("EL JUEGO NO PUEDE COMENZAR HAY UN ERROR CON EL TAMAÑO DEL TABLERO. DEBE SER AL MENOS DE 4X4");
    else
    {
        
        while(!packet.juego.acabo && !packet.juego.empate)
        {            
            packet.type=TEXT_MESSAGE;
            if(turno==packet.juego.turno)
            {
                imprimirTurno();
                if(validarGanador())
                {
                    packet.juego.acabo = 1;
                    printf("\n\n\t EL JUGADOR %d HA GANADO !!!!!!!!!!!!\n",packet.juego.turno);
                    imprimirTablero();
                }
                if(packet.juego.casillasLlenas == TAM * TAM)
                {
                    packet.juego.empate = 1;
                    printf("\n\n\t EL JUEGO TERMINO, ES UN EMPATE !!!!!!!!!\n");
                    imprimirTablero();
                }
                send_client(&socket,&packet);
                packet.juego.turno=-1;
            }
            else
            {
                recieve_client(&socket, &packet);
                usleep(3000);
                printf("\nturno propio:red %d:%d\n",turno,packet.juego.turno);
                
                imprimirTablero();
                if(validarGanador())
                {
                    packet.juego.acabo = 1;
                    printf("\n\n\t EL JUGADOR %d HA GANADO !!!!!!!!!!!!\n",packet.juego.turno);
                }
                if(packet.juego.casillasLlenas == TAM * TAM)
                {
                    packet.juego.empate = 1;
                    printf("\n\n\t EL JUEGO TERMINO, ES UN EMPATE !!!!!!!!!\n");
                }
            }
        }
    }
    return 0;
}




