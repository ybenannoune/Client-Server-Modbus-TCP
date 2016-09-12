/*******************************************************************/
/* tcp.h : fonctions exploitant les Sockets Windows v. 1.1 et plus */
/*******************************************************************/

#include<winsock.h>

int tcp_open( SOCKET *sock, char *servaddr, int servport );
int tcp_close ( SOCKET sock );
int send_request (SOCKET sock, char *apdu, int len );
int get_confirmation (SOCKET sock, char *apdu, int max_len );