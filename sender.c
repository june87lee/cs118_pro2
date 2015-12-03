/*
  Project 2: Simple Window-based Reliable Data Transfer
  COM SCI 118
  Eric Chiang
  June Lee
  The project comprises of building a reliable transport layer protocol
  built over the UDP transport layer.  We will build receiver/client and
  sender/server.
  Primary file for server/sender.
  ./sender <portnumber>
*/

#include "udpGBN.h"

struct pack pWin[CWIN_SIZE];

int main(int argc, char *argv[])
{
	 int sockfd, newsockfd, portno, pid;
     socklen_t clilen;
     struct sockaddr_in serv_addr, cli_addr;
     struct sigaction sa;          // for signal SIGCHLD
     //First we check if it has port number
     if(argc<2)
     {
         fprintf(stderr,"ERROR, no port provided\n");
         exit(1);
     }
     //Set up socket file descriptor with UDP protocol.
     /*Personal Notes
       AF_INET is and address family, specifically IPv4
       SOCK_DGRAM for UDP packets
     */
     sockfd = socket(AF_INET, SOCK_DGRAM, 0);
     if (sockfd < 0) 
        error("ERROR opening socket");
     //Zeroing out memory segment for serv_addr
     bzero((char *) &serv_addr, sizeof(serv_addr));
     //This is the source port number of server sender.
     portno = atoi(argv[1]);
     serv_addr.sin_family = AF_INET;
     serv_addr.sin_addr.s_addr = INADDR_ANY;
     serv_addr.sin_port = htons(portno);
     //binds serv_addr to a particular socket request when executing
     if (bind(sockfd, (struct sockaddr *) &serv_addr,
              sizeof(serv_addr)) < 0) 
              error("ERROR on binding");
     clilen = sizeof(cli_addr);

}