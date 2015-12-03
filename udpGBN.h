/*
  Project 2: Simple Window-based Reliable Data Transfer
  COM SCI 118
  Eric Chiang
  June Lee
  The project comprises of building a reliable transport layer protocol
  built over the UDP transport layer.  We will build receiver/client and
  sender/server.
  This header file contains all basic stuff that all files would share.
*/
#include <stdio.h>
#include <sys/types.h>   // definitions of a number of data types used in socket.h and netinet/in.h
#include <sys/socket.h>  // definitions of structures needed for sockets, e.g. sockaddr
#include <netinet/in.h>  // constants and structures needed for internet domain addresses, e.g. sockaddr_in
#include <stdlib.h>
#include <strings.h>
#include <sys/wait.h>	/* for the waitpid() system call */
#include <signal.h>	/* signal name macros, and the kill() prototype */
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>

#define RTT 7777 //Initial RTT, updated dynamically
#define CWIN_SIZE 5 //a constant window size

const unsigned int MAX_DATA_SIZE = 256; //Maximum allowed is 1k bytes, so 256 + 16
										//(due to size of four ints) is still within
										//1k.

//The signal we are sending
enum Signal {ACK, PACK, REQ, CLO}

//The header structure
struct pHeader
{
	Signal sig; //ACK, PACK, REQ
	int seqNo; //sequence number, range 0-4
	int sPortNo; //Source Port number
	int dPortNo; //Destination Port number
	int totalSize; //Total Size of data
};

//The data structure of packet
struct pack
{
	struct pHeader head; //the header segment
	char data[MAX_DATA_SIZE]; //actual segment of data, and file requested
							  //not surpass 256 characters.
};
