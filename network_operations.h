#ifndef NETWORK_OPERATIONS_H
#define NETWORK_OPERATIONS_H

int SocketTCP();
int Connect(int socketid, char *ip, int port);
int Send(int socketid, char *buffer, int len);
int Receive(int socketid, char *buffer, int len);
int Close(int socketid);

#endif /* NETWORK_OPERATIONS_H */
