/********************************************************************/
/* client.cpp : Programme client pour l'envoi de requêtes vers un   */
/*              serveur Modbus/TCP                                  */
/********************************************************************/

#include <stdio.h>
#include <winsock.h>
#include "tcp.h"

#pragma comment(lib, "wsock32.lib")
 
#define MBPORT  502   // port Modbus/TCP serveur

int idtrans = 1;      // Identifiant de transaction Modbus/TCP: variable globale

int menu ();

int readWordI ( SOCKET sock,
                unsigned char slave,
                unsigned short addr,
                unsigned short number,
                char *apdu );
int readWordO ( SOCKET sock,
                unsigned char slave,
                unsigned short addr,
                unsigned short number,
                char *apdu );
int writeWordO ( SOCKET sock,
                 unsigned char slave,
                 unsigned short addr,
                 unsigned short number,
                 unsigned short *value,
                 char *apdu );

void main() {

    SOCKET sock;            // socket du client, initialisée plus tard
    char serv_addr[255];    // adresse IP du serveur
    char apdu[260];         // APDU émission et réception
    int len;                // longueur APDU
    int choix, nbytes, i;
    unsigned char slave;
    unsigned short addr, number, valeur[250];
    
    /* Vérifier d'abord la présence de la DLL gérant les Sockets */
    static WORD wVersionRequested;
    WSADATA wsaData;
    int err;
 
    wVersionRequested = MAKEWORD( 1, 1 );
 
    err = WSAStartup( wVersionRequested, &wsaData );
    if ( err != 0 ) {
        /* Tell the user that we could not find a usable */
        /* WinSock DLL.                                  */
        printf("Impossible de trouver la DLL WinSock.\n");
        exit(-1);
    }
 
    /* Confirm that the WinSock DLL supports 1.1.*/
    /* Note that if the DLL supports versions greater    */
    /* than 1.1 in addition to 1.1, it will still return */
    /* 1.1 in wVersion since that is the version we      */
    /* requested.                                        */

    if ( LOBYTE( wsaData.wVersion ) != 1 || HIBYTE( wsaData.wVersion ) != 1 ) {
        /* Tell the user that we could not find a usable */
        /* WinSock DLL.                                  */
        printf("Version incorrecte de WinSock.\n");
        WSACleanup( );
        exit(-1);
    }

    /* The WinSock DLL is acceptable. Proceed. */
    
    /* Affichage du menu */    
    do {
        choix = menu( );
                
        switch (choix) {
            case 1:
                // A COMPLETER POUR .................

				// - renseigner l'adresse IP serveur avec serv_addr
				printf("Entrez l'adresse IP du serveur : "); scanf("%s", serv_addr);

				 // - ouvrir une connexion TCP avec le serveur
				err = tcp_open(&sock,serv_addr,MBPORT);
				if( err == -1 ){
					//Erreur à la connexion
					printf("Erreur de connexion au serveur %s \n",serv_addr);
				}else{
					//Connexion effectué
					printf("Connexion au serveur réussie a l'adresse : %s \n", serv_addr);

				}                         

                
                break;
            case 2: // lecture de mots d'entrée
                printf("Requete de lecture de mots d'entree.\n");
                printf("ID esclave  : "); scanf("%d", &slave);
                printf("Adresse mot : "); scanf("%d", &addr);
                printf("Nombre mots : "); scanf("%d", &number);
                nbytes = readWordI( sock, slave, addr, number, apdu);
                break;

            case 3: // lecture de mots de sortie
                printf("Requete de lecture de mots de sortie.\n");
                printf("ID esclave  : "); scanf("%d", &slave);
                printf("Adresse mot : "); scanf("%d", &addr);
                printf("Nombre mots : "); scanf("%d", &number);
                nbytes = readWordO( sock, slave, addr, number, apdu);
				
                break;
            case 4: // écriture de mots
                printf("Ecriture de mots.\n");
                printf("ID esclave  : "); scanf("%d", &slave);
                printf("Adresse mot : "); scanf("%d", &addr);
                printf("Nombre mots : "); scanf("%d", &number);
				printf("Valeur des mots : "); 
				for(int i = 0; i < number;i++){
					printf("Valeur[%d] = ",i);
					scanf("%d" , &valeur[i]);					
				}
				nbytes = writeWordO( sock, slave, addr, number, valeur, apdu);
                break;
            case 5:
                // Compléter pour fermer la connexion ......
				
				printf("Fermeture de la connexion ! \n");
				tcp_close(sock);
				
                WSACleanup( );
                break;
            default :
                printf("Choix non disponible !\n");
        }
                
        if ( (choix>=2) && (choix<=4) ) {
            printf("%d octets recus :\n", nbytes);
            if ( nbytes>0 ) {
                for (i=0; i<nbytes; i++)
                    printf("%02x ", (unsigned char)apdu[i]);
                printf("\n**END**\n");
            }
        }
    } while ( choix != 5);
    exit(0);
}

// Fonction d'affichage du menu
int menu ()
{    
    int choix;

    do {
        printf("1. Connexion au serveur\n");
        printf("2. Lecture de mots d'entree\n");
        printf("3. Lecture de mots de sortie\n");
        printf("4. Ecrire des mots\n");
        printf("5. Deconnexion et sortie\n\n");

        printf("Votre choix :");
        scanf("%d",&choix);

    } while ( (choix<1) || (choix>5) );

    return choix;
}

// Fonction de lecture de mots d'entrée
int readWordI ( SOCKET sock,
                unsigned char slave,
                unsigned short addr,
                unsigned short number,
                char *apdu )
{
    int apdu_len, nbytes;

    apdu[0]=HIBYTE( idtrans );    // ID Transaction
    apdu[1]=LOBYTE( idtrans );
    apdu[2]=0;                    // ID Protocole (0=Modbus)
    apdu[3]=0;
    apdu[4]=0;                    // Longueur
    apdu[5]=6;
    apdu[6]=slave;                // ID Unité
    apdu[7]=4;                    // Code fonction
    apdu[8]=HIBYTE( addr );       // @ 1er mot
    apdu[9]=LOBYTE( addr );        
    apdu[10]=HIBYTE( number );    // Nombre de mots
    apdu[11]=LOBYTE( number );

    apdu_len=12;
    /* Envoi de la requête */
    send_request( sock, apdu, apdu_len);

    apdu_len=9 + 2*number;
    /* Attente confirmation */
    nbytes = get_confirmation( sock, apdu, apdu_len );
    if ( nbytes != apdu_len )        // erreur
        nbytes = -1;

    idtrans++;

    return nbytes;
}

// Fonction de lecture de mots de sortie
int readWordO ( SOCKET sock,
                unsigned char slave,
                unsigned short addr,
                unsigned short number,
                char *apdu )
{
    int apdu_len, nbytes;

    apdu[0]=HIBYTE( idtrans );    // ID Transaction
    apdu[1]=LOBYTE( idtrans );
    apdu[2]=0;                    // ID Protocole (0=Modbus)
    apdu[3]=0;
    apdu[4]=0;                    // Longueur
	apdu[5]=6;
    apdu[6]=slave;                // ID Unité
    apdu[7]=3;                    // Code fonction
    apdu[8]=HIBYTE( addr );       // @ 1er mot
    apdu[9]=LOBYTE( addr );        
    apdu[10]=HIBYTE( number );    // Nombre de mots
    apdu[11]=LOBYTE( number );
    
	apdu_len=12;

	/* Envoi de la requête */
    send_request( sock, apdu, apdu_len);

	apdu_len=9 + 2*number;
	/* Attente confirmation */
    nbytes = get_confirmation( sock, apdu, apdu_len );
    if ( nbytes != apdu_len )        // erreur
        nbytes = -1;

	idtrans++;

    return nbytes;
}

// Fonction d'écriture de mots de sortie
int writeWordO ( SOCKET sock,
                 unsigned char slave,
                 unsigned short addr,
                 unsigned short number,
                 unsigned short *value,
                 char *apdu )
{
    int apdu_len, nbytes;
   	int i;

    apdu[0]=HIBYTE( idtrans );    // ID Transaction
    apdu[1]=LOBYTE( idtrans );
    apdu[2]=0;                    // ID Protocole (0=Modbus)
    apdu[3]=0;
    apdu[4]=HIBYTE(7 + 2* number);// Longueur
    apdu[5]=LOBYTE(7 + 2* number);
    apdu[6]=slave;                // ID Unité
    apdu[7]=16;                   // Code fonction
    apdu[8]=HIBYTE( addr );       // @ 1er mot
    apdu[9]=LOBYTE( addr );        
    apdu[10]=HIBYTE( number );    // Nombre de mots
    apdu[11]=LOBYTE( number );
	apdu[12]=2*number;			  //nombre d'octets
		
	for(i = 0; i < 2*number;i++){                    //1er mot
		apdu[13 + i * 2] = HIBYTE(value[i]);	
		apdu[14 + i * 2] = LOBYTE(value[i]);
	}
   
	apdu_len= 13 +2*number;

	/* Envoi de la requête */
    send_request( sock, apdu, apdu_len);

	apdu_len= 12;
	/* Attente confirmation */
    nbytes = get_confirmation( sock, apdu, apdu_len );
    if ( nbytes != apdu_len )        // erreur
        nbytes = -1;

	idtrans++;

    return nbytes;
}