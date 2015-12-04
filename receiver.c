/*
Project 2: Simple Window-based Reliable Data Transfer
COM SCI 118
Eric Chiang
June Lee
The project comprises of building a reliable transport layer protocol
built over the UDP transport layer. We will build receiver/client and
sender/server.
Primary file for client.
./receiver <hostname> <portnumber> <filename>
*/
#include "udpGBN.h"
double LOSS_PROB = 0.00;
double CORRUPT_PROB = 0.0;

int main(int argc, char * * argv) {
    int socketfd;
    int mSeqNo = 0;
    struct pack rcv_pack;
    struct pack rsp_pack;
    FILE * req_file; //file to ask for
    struct sockaddr_in serverAddr;
    double loss;
    double corr;
    
    //zero out structs
    bzero((char * ) & rsp_pack, sizeof(rsp_pack));
    bzero((char * ) & rcv_pack, sizeof(rcv_pack));
    bzero((char * ) & serverAddr, sizeof(serverAddr));
    //check if correct arguments
    if (argc < 4 || argc > 6) {
        fprintf(stderr, "usage: %s <hostname> <port> <filename> <loss prob> <corr prob>\n", argv[0]);
        exit(1);
    }
    if(argc == 4){
        loss = 0;
        corr = 0;
    }
    else if(argc == 5){
        loss = argv[4];
        corr = 0;
    }
    else if(argc == 6){
        loss = argv[4];
        corr = argv[5];
    }
    
    //make socket
    socketfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (socketfd < 0)
        error("ERROR opening socket");
    //find host
    char * hName = argv[1];
    struct hostent * hostName;
    hostName = gethostbyname(hName);
    if (hostName == NULL) {
        fprintf(stderr, "ERROR, no such host");
        exit(1);
    }
    serverAddr.sin_family = AF_INET;
    bcopy((char * ) hostName->h_addr, (char * ) & serverAddr.sin_addr.s_addr, hostName-> h_length);
    serverAddr.sin_port = htons(atoi(argv[2]));

    //Set up response packet
    rsp_pack.head.sig = REQ;
    rsp_pack.head.sPortNo = atoi(argv[2]); //The client's port number
    rsp_pack.head.dPortNo = 0; //Unknown server port number
    rsp_pack.head.totalSize = sizeof(rsp_pack); //Total size of data to transmit
    rsp_pack.head.seqNo = mSeqNo;
    char* fileName = argv[3];
    strcpy(rsp_pack.data, fileName); //ask for file
    rsp_pack.head.packSize = strlen(fileName);
    int sAddrLen = sizeof(serverAddr);
    //send request
    sendto(socketfd, & rsp_pack, sizeof(rsp_pack), 0, (struct sockaddr * ) & serverAddr, sAddrLen);
    bzero((char * ) & rsp_pack, sizeof(rsp_pack));
    printf("Sent request for file %s\n", rsp_pack.data);
    srand(time(NULL));
    FILE * file = fopen(strcat(argv[3], "2"), "w");
    while (1) {
        loss = rand()/(double) RAND_MAX;
        corr = rand()/(double) RAND_MAX;
        if (recvfrom(socketfd, & rcv_pack, sizeof(rcv_pack), 0, (struct sockaddr * ) & serverAddr, (socklen_t * ) & sAddrLen) < 0 || loss < LOSS_PROB) {
            printf("Lost packet\n");
        } else if (corr < CORRUPT_PROB) {
            printf("Sending corrupted packet\n");
            rsp_pack.head.sig = COR;
            if (sendto(socketfd, &rsp_pack, sizeof(rcv_pack), 0, (struct sockaddr*) &serverAddr, sAddrLen) < 0)
              error("ERROR responding to corrupt packet");
        } else {
            if (rcv_pack.head.sig == COR)
                printf("Received corrupted packet");
            else if (rcv_pack.head.sig == CLO)
                break;
            else if (rcv_pack.head.sig == PACK && rcv_pack.head.seqNo == mSeqNo) {
                printf("Received packet number %d\n", rcv_pack.head.seqNo);
                fwrite(rcv_pack.data, 1, rcv_pack.head.packSize, file);
                rsp_pack.head.seqNo = rcv_pack.head.seqNo;
                rsp_pack.head.sig = ACK;
                rsp_pack.head.sPortNo = atoi(argv[2]);
                rsp_pack.head.dPortNo = rcv_pack.head.sPortNo;
                rsp_pack.head.totalSize = sizeof(rsp_pack);
                rsp_pack.head.packSize = 0;
                if (sendto(socketfd, & rsp_pack, sizeof(rsp_pack), 0, (struct sockaddr * ) & serverAddr, sAddrLen) < 0)
                    error("ERROR acking");
                printf("ACK'd packet %d\n", rsp_pack.head.seqNo);
                mSeqNo++;
            }
            else
                printf("IGNORE %d: expected %d\n", rcv_pack.head.seqNo, mSeqNo);
        }
    }
    rsp_pack.head.seqNo = rcv_pack.head.seqNo;
    rsp_pack.head.sig = CLO;
    rsp_pack.head.sPortNo = atoi(argv[2]);
    rsp_pack.head.dPortNo = rcv_pack.head.sPortNo;
    rsp_pack.head.totalSize = sizeof(rsp_pack);
    rsp_pack.head.packSize = 0;
    if (sendto(socketfd, & rsp_pack, sizeof(rsp_pack), 0, (struct sockaddr * ) & serverAddr, sAddrLen) < 0)
        error("ERROR acking");
    printf("Sent FIN ACK\n");
    fclose(file);
    close(socketfd);
    return 0;
}
