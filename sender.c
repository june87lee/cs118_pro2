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
     bzero((char *) &rcv_pack, sizeof(rcv_pack));
     int probLoss=0;
     int probCor=0;
     int pl=0;
     int cwnd_size=CWIN_SIZE;
     int pc=0;
     //First we check if it has port number
     if(argc<2)
     {
         fprintf(stderr,"ERROR, no port provided\n");
         exit(1);
     }
     if(argc==3)
     {
     	cwnd_size=atoi(argv[2]);
     	//pl = (int)(argv[2]*100);
     	//probLoss = ((rand()%100+1)<=pl);
     }
     struct pack pWin[cwnd_size];
     if(argc==4)
     {
     	pl = (int)(atof(argv[3])*100);
     }
     if(argc==5)
     {
     	pc = (int)(atof(argv[4])*100);
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
     	if(rcv_pack.head.sig == REQ && cv_pack.head.sig != COR)//will ignore all packets, until initial request
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
     				   :(f_size/MAX_DATA_SIZE) );
               fprintf(stdout,"  -THE NUMBER OF PACKETS TO SEND: %d \n", numPacks);
     		sentPacks=0;
     		trkSeqNo=0;
     		/*
				The actual meat of the project.
     		*/
     		int i;
     		//initially, safe to send all 5 packets at once
     		//time(&setTime);//grabbing initial time
     		for(i=0;i<cwnd_size;i++)
     		{
     			bzero((char *) &rsp_pack, sizeof(rsp_pack));
     			//Preping the response packet
     			rsp_pack.head.sig = PACK;
     			rsp_pack.head.sPortNo = portno; //The server's port number
     			rsp_pack.head.dPortNo = rcv_pack.head.sPortNo; //The client's port number
     			rsp_pack.head.totalSize = f_size; //Total size of data to transmit
     			//trkSeqNo++;
     			rsp_pack.head.seqNo = trkSeqNo;
                    trkSeqNo++;
     			rsp_pack.head.packSize = fread(rsp_pack.data, 1, MAX_DATA_SIZE,req_file);//sequentially read the file
     			pWin[i]=rsp_pack;
     			if(pl>0)
					probLoss = ((rand()%100+1)<=pl);
				if(pc>0)
					probCor = ((rand()%100+1)<=pc);
				if(probCor!=0)
					rsp_pack.head.sig = COR;
				fprintf(stdout,"  Sending packet: %d \n", trkSeqNo-1);
				if(probLoss==0) //simulating loss
				{     				
     				sendto(sockfd, &rsp_pack, sizeof(rsp_pack), 0,
     				 	  (struct sockaddr*) &cli_addr, clilen);
     			}
     			probLoss = 0;
     			probCor = 0;
     		}
               fprintf(stdout,"  BEFORE TIME \n");
     		time(&setTime);//grabbing initial time
               fprintf(stdout,"  AFTER TIME \n");
     		while(sentPacks<numPacks)
     		{
                    fprintf(stdout,"  IN SECOND LOOP \n");
     			//Now only "slide the window" iff...
     			//  1. ack received
     			//  2. timeout has not occured
     			//*making sure the received packet is zeroed out
     			bzero((char *) &rcv_pack, sizeof(rcv_pack));
     			if(time(NULL)>setTime+RTT) //if this is true then timeout has occurred
     			{
     				//basically have to resubmit everything in window
     				int k;
     				for(k=0;k<cwnd_size;k++)
     				{
     					fprintf(stdout,"  Resending packet: %d \n", pWin[k].head.seqNo);
     					sendto(sockfd, &pWin[k], sizeof(rsp_pack), 0,
     						  (struct sockaddr*) &cli_addr, clilen);
     				}
     				//also reset timer

     				time(&setTime);//grabbing initial time
     			}
     			//if we received something from client, check the ack and slide window
     			//*making sure the received packet is zeroed out
     			//bzero((char *) &rcv_pack, sizeof(rcv_pack));
     			else if(recvfrom(sockfd, &rcv_pack, sizeof(rcv_pack),0,(struct sockaddr*) &serv_addr,
     					(socklen_t*) &clilen) > 0)
     			{
                         //fprintf(stdout,"  RECEIVER SOMETHING!");
     				if(rcv_pack.head.sig == ACK && cv_pack.head.sig != COR) //only evaluate ACK packets
     				{
                              fprintf(stdout,"  Received ACK: %d \n", rcv_pack.head.seqNo);
     					//this will remove first element of window iff
     					//ACK seq no matches it, if not ignore
     					//OR if we some how manage to receive a seqNo greater
     					//than first, we can assume the client has previous
     					if(pWin[0].head.seqNo <= rcv_pack.head.seqNo)
     					{
     						//we can confirm a packet has been sent now
     						sentPacks++;
     						//check to see if it was last packet to be sent
     						if(sentPacks<numPacks)
     						{
     							//now shift the window to the left
     							int j;
     							for(j=0;j<(cwnd_size-1);j++)
     							{
     								pWin[j]=pWin[j+1];
     							}
     							//now adding in new packet to window
     							bzero((char *) &rsp_pack, sizeof(rsp_pack));
     							//Preping the response packet
     							rsp_pack.head.sig = PACK;
     							rsp_pack.head.sPortNo = portno; //The server's port number
     							rsp_pack.head.dPortNo = rcv_pack.head.sPortNo; //The client's port number
     							rsp_pack.head.totalSize = f_size; //Total size of data to transmit
     							//trkSeqNo+=1;//bump up the sequence number
     							rsp_pack.head.seqNo = trkSeqNo;
                                        trkSeqNo+=1;
     							rsp_pack.head.packSize = fread(rsp_pack.data, 1, MAX_DATA_SIZE,req_file);//sequentially read the file
     							pWin[cwnd_size-1]=rsp_pack;
     							if(pl>0) //simulating loss
									probLoss = ((rand()%100+1)<=pl);
								if(pc>0)
									probCor = ((rand()%100+1)<=pc);
								if(probCor!=0)
									rsp_pack.head.sig = COR;
								fprintf(stdout,"  Sending packet: %d \n", trkSeqNo);
								if(probLoss==0)
								{
     								//try sending packet
     								sendto(sockfd, &rsp_pack, sizeof(rsp_pack), 0,
     								 	  (struct sockaddr*) &cli_addr, clilen);
     							}
     							probLoss = 0;
     							probCor = 0;
     							//reset timer
     							time(&setTime);//grabbing initial time
     						}
     					}
     				}
     			}
     		}
     		//if out of second loop, can assume all packets have been sent
     		//send out packet signaling to close
     		bzero((char *) &rsp_pack, sizeof(rsp_pack));
     		rsp_pack.head.sig = CLO;
     		sendto(sockfd, &rsp_pack, sizeof(rsp_pack), 0,
     			  (struct sockaddr*) &cli_addr, clilen);
     		fclose(req_file);
     	}
    }
    return 0;
}