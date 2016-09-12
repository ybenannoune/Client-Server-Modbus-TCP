/********************************************************************/
/* server.cpp : Programme serveur Modbus/TCP                        */
/********************************************************************/

#include <stdio.h>
#include <winsock.h>
#include <iostream>

#pragma comment(lib, "wsock32.lib")

#define MBPORT  502	// port Modbus/TCP serveur

#define BUF_SIZE 65535

/* Définition des variables globales */
unsigned short holdregisterRO[10] = {0x1ff0, 0x1ff1, 0x1ff2, 0x1ff3, 0x1ff4, 
	0x1ff5, 0x1ff6, 0x1ff7, 0x1ff8, 0x1ff9};
unsigned short outputregisterRO[10] = {0x2ff0, 0x2ff1, 0x2ff2, 0x2ff3, 0x2ff4, 
	0x2ff5, 0x2ff6, 0x2ff7, 0x2ff8, 0x2ff9};
unsigned short outputregisterRW[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};


void afficheVariablesGlobales(){
	system("cls");
	printf("Adr.  Entrees Sorties\n");
	printf("----------------------\n");

	for(int i = 0; i < 10; i++){
		printf("%00.2X    0x%04.4X   0x%04.4X \n",i,holdregisterRO[i],outputregisterRO[i]);
	}

	for(int i = 100; i < 110; i++){
		printf("%00.2X             0x%04.4X \n",i,outputregisterRW[i]);
	}
}


int main () {
	
	SOCKET sock_listen; 		// identifiants de la socket d'écoute
	SOCKET sock_connect;		// identifiant de la socket de connexion
	SOCKADDR_IN addr_client; 	// structure d'infos lié à l'écoute
	char buffer[BUF_SIZE];	// tampon de données reçues ou envoyées par le serveur
	int buffer_len; 	// nombre de caractères envoyés ou reçus

	char buffer_reponse[BUF_SIZE]; // Buffer de la réponse
	int  len_reponse;			   // Taille de la réponse			
	unsigned short nbrOctets;	   // Nombre d'octets lus ou à ecrire
	unsigned short longueur;       // Longueur de la trame Modbus
	unsigned short nombreMots;	   // Nombre de mots à lire ou écrire
	unsigned short adresseMot;	   // Adresse du 1er mot
	unsigned short valeurMot;      // Valeur mot à écrire
	int i;						   // Pour les boucles for
	int temp;					   // variable temporaire
	
	
	/* Vérifier d'abord la présence de la DLL gérant les Sockets */
	static WORD wVersionRequested;
	WSADATA wsaData;
	int err;
 
	wVersionRequested = MAKEWORD( 1, 1 );
 
	err = WSAStartup( wVersionRequested, &wsaData );
	if ( err != 0 ) {
		printf("Impossible de trouver la DLL WinSock.\n");
		exit(-1);
	}

	if ( LOBYTE( wsaData.wVersion ) != 1 || HIBYTE( wsaData.wVersion ) != 1 ) {
		printf("Version incorrecte de WinSock.\n");
		WSACleanup( );
		exit(-1);
	}
	
	/* Ouverture de la socket */
	sock_listen=socket(AF_INET,SOCK_STREAM,0);
	if (sock_listen==INVALID_SOCKET)
		printf("Impossible de creer la socket du a l'erreur : %d\n",
			WSAGetLastError());
	else
		printf("socket      : OK\n");

	/* Activation de l'algorithme de Nagle */
	temp=1;
	err=setsockopt(sock_listen,IPPROTO_TCP,TCP_NODELAY,(char *)&temp,sizeof(temp));
	if (err!=0)
		printf("Impossible de configurer cette options du à l'erreur : %d %d",
		err,WSAGetLastError());
	else
		printf("setsockopt  : OK\n");

	/* Lier la socket à une adresse IP et un port d'écoute */
	addr_client.sin_family=AF_INET;
	addr_client.sin_addr.s_addr=INADDR_ANY; // Ecoute sur toutes les IP locales  
	addr_client.sin_port=htons(MBPORT); 	// Ecoute sur le port Modbus=502
	err=bind(sock_listen,(struct sockaddr*)&addr_client,sizeof(addr_client));
	if (err!=0)
		printf("Impossible d'ecouter ce port : %d %d\n",err,WSAGetLastError());
	else
		printf("bind        : OK\n");

	/* Affichage des services offerts par le serveur */
	printf("\nBonjour, vous etes du cote serveur Modbus/TCP\n");
	printf("Cet utilitaire vous propose :\n");
	printf("  - 10 mots d'entree a l'adresse 0 (lecture) de 0x1ff0 à 0x1ff9\n");
	printf("  - 10 mots de sortie a l'adresse 0 (lecture) de 0x2ff0 à 0x2ff9\n");
	printf("  - 10 mots de sortie a l'adresse 100 (lecture/ecriture) mis à 0\n");

	// ... A COMPLETER ...............................................
    // Ecoute du port 502, connexion, réponses aux requêtes du client
	// Ecoute du port 502 on limite à la connexion à 1 seule.

	if(listen(sock_listen, 1) == SOCKET_ERROR)
	{
	    perror("listen()");
		exit(errno);
	}
	else
	{
		printf("Ecoute du port 502 ! \n");
	}

	
	int sinsize = sizeof addr_client;

	//appel bloquant attendant la connexion
	sock_connect = accept(sock_listen, (SOCKADDR *)&addr_client, &sinsize);

	if(sock_connect == INVALID_SOCKET)
	{
		perror("accept()");
		exit(errno);
	}
	else
	{
		printf("Connecte au client ! \n");
	}
		
	afficheVariablesGlobales();

	// A partir de ce point, utiliser la socket "sock_listen" puis "sock_connect" 
	// pour envoyer/recevoir des données
	
	while(true)
	{		
		if((buffer_len = recv(sock_connect, buffer, sizeof buffer - 1, 0)) < 0)
		{
			//Fermeture du client
			perror("recv()");
			exit(errno);
		}

		if(buffer_len != 0)
		{		
			printf("%d octets recus :\n", buffer_len);
            if ( buffer_len>0 ) {
                for (int i=0; i<buffer_len; i++)
                    printf("%02x ", (unsigned char)buffer[i]);
                printf("\n**END**\n");
            }					

			//Trame Modbus lecture de mots entrée, nbr mot 3
			// 01 00 00 00 06 00 04 00 00 00 03	
			char idEsclave = buffer[06];
			char codeFonction = buffer[07];

			//Formatage de la réponse			
			buffer_reponse[00] = buffer[00];	// ID Transaction
			buffer_reponse[01] = buffer[01];	// ID Transaction
			buffer_reponse[02] = 0;				// Protocole Modbus 0
			buffer_reponse[03] = 0;				// Protocole Modbus 0	
			buffer_reponse[06] = idEsclave;	//Recopie de l'adresse de l'esclave
			buffer_reponse[07] = codeFonction;	//Recopie du code fonction 

			adresseMot = MAKEWORD(buffer[9],(buffer[8]));		
			nombreMots = MAKEWORD(buffer[11],(buffer[10]));
		
			switch (codeFonction)
			{
			//Le code fonction 3 et 4 sont gérés de la même manière, ce sont juste les données retournées qui changent
			case 4:
			case 3:						
				nbrOctets = nombreMots * 2; // Nombre d'octets lus			
				for(i = 0; i < nombreMots; i ++)
				{
					if( codeFonction == 3)
					{
						//On renvois les valeurs de sorties
						buffer_reponse[9 + i * 2] = HIBYTE(outputregisterRO[adresseMot + i]);
						buffer_reponse[9 + i * 2 + 1] = LOBYTE(outputregisterRO[adresseMot + i]);
					}
					else
					{
						//On revois les valeurs d'entrées
						buffer_reponse[9 + i * 2] = HIBYTE(holdregisterRO[adresseMot + i]);
						buffer_reponse[9 + i * 2 + 1] = LOBYTE(holdregisterRO[adresseMot + i]);
					}
				
				}			
				longueur = 3 + nbrOctets;			
				len_reponse = 9 + nbrOctets;
						

				buffer_reponse[8] = nbrOctets; 

				break;

			case 16:
				longueur = 6;

				nbrOctets = buffer[12]; //Récupere le nombre de mots à écrire
				for(i = 0; i < nombreMots; i ++)
				{
					valeurMot = MAKEWORD(buffer[14 + i * 2],buffer[13 + i * 2]);			
					outputregisterRW[adresseMot + i] = valeurMot;
				}	

				buffer_reponse[8] = HIBYTE(adresseMot);
				buffer_reponse[9] = LOBYTE(adresseMot);
				buffer_reponse[10] = HIBYTE(nombreMots);
				buffer_reponse[11] = LOBYTE(nombreMots);
				len_reponse = 12;

				afficheVariablesGlobales(); 
				break;

			default:
				printf("Le code fonction est incorrect \n");	
				break;
			}

			
			buffer_reponse[04] = HIBYTE(longueur);
			buffer_reponse[05] = LOBYTE(longueur);

			//Renvois la réponse
			send(sock_connect,buffer_reponse,len_reponse,0);			
		}			

	}
	
	// ... A COMPLETER ...............................................




	/* Fermeture de la session TCP Correspondant à la commande connect() */
	err=shutdown(sock_connect,2); 	// 2 signifie socket d'émission et d'écoute
	if (err!=0)
		printf("Impossible de fermer la session TCP du a l'erreur : %d %d\n",
		err,WSAGetLastError());
	else
		printf("shutdown    : OK\n");

	/* Fermeture des deux socket correspondant à la commande socket() et accept() */
	err=closesocket(sock_connect);
	if (err!=0)
		printf("Impossible de liberer la socket du a l'erreur : %d %d\n",
			err,WSAGetLastError());
	else
		printf("closesocket : OK\n");

	err=closesocket(sock_listen);
	if (err!=0)
		printf("Impossible de liberer la socket du a l'erreur : %d %d\n",
			err,WSAGetLastError());
	else
		printf("closesocket : OK\n");

	/* Quitte proprement le winsock ouvert avec la commande WSAStartup */
	err=WSACleanup();
	if (err!=0)
		printf("Impossible de liberer winsock du a l'erreur : %d %d\n",
			err,WSAGetLastError());
	else
		printf("WSACleanup  : OK\n");
	
	return (0);
}