#include <net/socket.h>
#include <string.h>
#include "network_operations.h"

#define MAX_SOCKETS 20

int sockets[MAX_SOCKETS];

int SocketTCP() {
  int i, sockfd;

  // Find an available socket file descriptor
  for (i = 0; i < MAX_SOCKETS; i++) {
    if (sockets[i] == 0) {
      // Create a new TCP socket
      sockfd = socket(AF_INET, SOCK_STREAM, 0);

      if (sockfd == -1) {
        return -1; // Error creating socket
      }

      sockets[i] = sockfd;
      return sockfd; // Return file descriptor ID
    }
  }

  return -1; // No available sockets
}

int Connect(int socketid, char *ip, int port) {
  int sockfd = sockets[socketid];
  struct sockaddr_in serv_addr;
  memset(&serv_addr, 0, sizeof(serv_addr));

  // Configure server address
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(port);
  if (inet_pton(AF_INET, ip, &serv_addr.sin_addr) <= 0) {
    return -1; // Error converting IP address
  }

  // Connect to server
  if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
    return -1; // Error connecting to server
  }

  return 0; // Connection successful
}

int Send(int socketid, char *buffer, int len) {
  int sockfd = sockets[socketid];
  int bytes_sent = send(sockfd, buffer, len, 0);

  if (bytes_sent == -1) {
    return -1; // Error sending data
  }

  return bytes_sent; // Return number of bytes sent
}

int Receive(int socketid, char *buffer, int len) {
  int sockfd = sockets[socketid];
  int bytes_received = recv(sockfd, buffer, len, 0);

  if (bytes_received == 0) {
    return 0; // Connection closed
  }

  if (bytes_received == -1) {
    return -1; // Error receiving data
  }

  return bytes_received; // Return number of bytes received
}

int Close(int socketid) {
  int sockfd = sockets[socketid];

  if (close(sockfd) == -1) {
    return -1; // Error closing socket
  }

  sockets[socketid] = 0; // Free socket file descriptor
  return 0; // Socket closed successfully
}
