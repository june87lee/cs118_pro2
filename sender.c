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
	 int sockfd, newsockfd, portno, pid, finished;
     socklen_t clilen;
     struct sockaddr_in serv_addr, cli_addr;
     struct sigaction sa;          // for signal SIGCHLD
     struct pack rcv_pack, rsp_pack;
     finished = 0;
     FILE *req_file; //the file to send out
     int f_size, numPacks, sentPacks, trkSeqNo; //total size of the file and total number of packets
     time_t setTime; //keeps track of time
     //Clearing out the memory for packets
     bzero((char *) &rsp_pack, sizeof(rsp_pack));
     bzero((char *) &rcv_pack, sizeof(rcv_packk));
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
        error("ERROR, opening socket\n");
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
              error("ERROR, on binding");
     clilen = sizeof(cli_addr);
     while(1)//server on indefinitely till closing of sender
     {
     	if(recvfrom(sockfd, &rcv_pack, sizeof(rcv_pack),0,(struct sockaddr*) &cli_addr,
     		(socklen_t*) &clilen) < 0)
     	{
        	error("ERROR, could not attain packet\n");
     	}
     	if(rcv_pack.head.sig == REQ)//will ignore all packets, until initial request
     	{
     		//Trying to open the file requested
     		req_file = fopen(rcv_pack.data, "rb");
     		if(req_file==NULL)
     			error("ERROR, could not open the file requested\n");
     		//Method to grab total file size
     		fseek(req_file, 0, SEEK_END);
     		f_size = ftell(req_file);
     		fseek(req_file, 0, SEEK_SET);
     		//Figuring out total number of packets.
     		numPacks = ( (f_size%MAX_DATA_SIZE > 0) ? ((f_size/MAX_DATA_SIZE)+1)
     				   :(f_size/MAX_DATA_SIZE) )
     		sentPacks=0;
     		trkSeqNo=0;
     		/*
				The actual meat of the project.
     		*/
     		int i;
     		//initially, safe to send all 5 packets at once
     		time(&setTime);//grabbing initial time
     		for(i=0;i<CWIN_SIZE;i++)
     		{
     			//Preping the response packet
     			rsp_pack.head.sig = PACK;
     			rsp_pack.head.sPortNo = portno; //The server's port number
     			rsp_pack.head.dPortNo = rcv_pack.head.sPortNo; //The client's port number
     			rsp_pack.head.totalSize = f_size; //Total size of data to transmit
     			trkSeqNo+=i;
     			rsp_pack.head.seqNo = trkSeqNo;
     			fread(rsp_pack.data, 1, MAX_DATA_SIZE,req_file);//sequentially read the file
     			pWin[i]=rsp_pack;
     			sendto(sockfd, &rsp_pack, sizeof(rsp_pack), 0,
     				  (struct sockaddr*) &cli_addr, clilen);
     		}
     		while(sentPacks<=numPacks)
     		{
     			//Now only "slide the window" iff...
     			//  1. ack received
     			//  2. timeout has not occured
     			if(time(NULL)>setTime+RTT) //if this is true then timeout has occurred
     			{
     				//basically have to resubmit everything in window
     			}
     			else if
     			//this part when received proper ack, shift the window and read in new
     		}
     	}
     }
}