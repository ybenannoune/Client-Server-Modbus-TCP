#include <winsock.h>

int tcp_open( SOCKET *sock, char *servaddr, int servport )
{	
	int erreur;
    struct hostent *nom_serveur;
	SOCKADDR_IN sin;

	/* Transformation de l'@ip de caractères à structure hostent */
	nom_serveur = gethostbyname( servaddr );
	if(nom_serveur==NULL)
		return -1;
	
	/* Création de la socket */
	*sock = socket( AF_INET, SOCK_STREAM, 0 );
	if ( *sock == INVALID_SOCKET )
		return -1;

	/* Configuration de la socket serveur */
	sin.sin_family = AF_INET;
	sin.sin_port = htons( servport );
	memcpy( &sin.sin_addr, nom_serveur->h_addr, nom_serveur->h_length );

	/* Attacher le socket local au socket serveur */
	erreur = connect( *sock, (SOCKADDR *)&sin, sizeof(sin));
	if ( erreur == INVALID_SOCKET )
		return -1;

	return 0;
}

int tcp_close( SOCKET sock )
{	
	return closesocket( sock );
}

int send_request( SOCKET sock, char *apdu, int len )
{	
	return send( sock, apdu, len, 0 );
}

int get_confirmation( SOCKET sock, char *apdu, int max_len )
{	
	return recv( sock, apdu, max_len, 0 );
}