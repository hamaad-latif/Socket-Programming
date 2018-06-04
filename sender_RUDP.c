#include <stdio.h>
#include <stdlib.h>

// Time function, sockets, htons... file stat
#include <sys/time.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <sys/stat.h>
#include <time.h>
#include <poll.h>

// File function and bzero
#include <fcntl.h>
#include <unistd.h>
#include <strings.h>
#include <string.h>


/* Size of the buffer used to send the file
* in several blocks
*/
#define bufferSize 500

/* Command to generate a test file
* dd if = / dev / urandom of = count = 8 file
*/

/*struct of payLoad that we have to send to the receiving side along with
sequence number followed by the packet and its size*/
struct packetPayload{
    char Buffer[bufferSize];
    int seqNumber;
    int dataSize;
};
/* Declaration of functions*/
int duratn(struct timeval *start, struct timeval *stop, struct timeval *delta);
int clt_sock_create(int prt, char* i_paddr);

/*Declaration of variables */
int total_packets_count;
void delay(int number_of_seconds);
int sizeOfLastPacket;
struct sockaddr_in serverSocket;
int seqAcks[6] = {-1,-1,-1,-1,-1,-1};
int NullseqAcks[6];
int Ack = -1;
int counter = 1;
int count = 1;
int index1;
int re = 0;
int flag = 0;
char cmd[50];
int dataSize1;
int dataSize2;
int dataSize3;
int dataSize4;
int dataSize5;

int null_counter=0;
int null_flag=0;


/*buffers for maintaining copy of data which will
 be used in case of retransmission of window's lost packets*/
char dataBuffer1[500] ;
char dataBuffer2[500] ;
char dataBuffer3[500] ;
char dataBuffer4[500] ;
char dataBuffer5[500] ;


//start of main function
int main(int argmnt_c, char**argmnt_v){

	struct timeval start, stop, delta;
	int s_fd, f_d;
	// char buffr[BUFFERT];
	off_t sum = 0, m, size;//long
	long int n;
	int l = sizeof(struct sockaddr_in);
	struct stat buffer;
	int r;

	//payLoad struct's variable for transmission of packets along with 
	// their sequence numbers.
	struct packetPayload p1;
	


	//time out function related variables
	struct timeval tv;
	tv.tv_sec = 1;
	tv.tv_usec = 0;

	setsockopt(s_fd, SOL_SOCKET, SO_RCVTIMEO,(char *)&tv,sizeof(struct timeval));

//initializing all our local buffers to null.
	for (int i = 0; i < bufferSize; i++){
		dataBuffer1[i]='\0';
		dataBuffer2[i]='\0';
		dataBuffer3[i]='\0';
		dataBuffer4[i]='\0';
		dataBuffer5[i]='\0';
	}

//connection initiation/check staements
	if (argmnt_c != 4){
		printf("Error usage : %s <ip_serv> <port_serv> <filename>\n", argmnt_v[0]);
		return EXIT_FAILURE;
	}

	s_fd = clt_sock_create(atoi(argmnt_v[2]), argmnt_v[1]);

	if ((f_d = open(argmnt_v[3], O_RDONLY)) == -1){
		perror("open fail");
		return EXIT_FAILURE;
	}

	//file size
	if (stat(argmnt_v[3], &buffer) == -1){
		perror("stat fail");
		return EXIT_FAILURE;
	}
	else{
		size = buffer.st_size;
	}

//sending process preparation....
	bzero(&p1, sizeof(p1));

	gettimeofday(&start, NULL);
	n = read(f_d, p1.Buffer, bufferSize);
	printf("\nSending Packet Counter: 1\n");
	// fputs(p1.Buffer,stdout);

	while (n){
		if (n == -1){
			perror("read fails");
			return EXIT_FAILURE;
		}
		printf("inside while(n)->loop for sending packets(transmission controlled loop)\n");

		/*this while loop is used for sending 5-packets
		to meet the requirement of window size=5 */
		while (counter < 6 ){
    		printf("\n n is %ld\n",n);
	        if (n == 0){
				p1.seqNumber = -2;
		        p1.dataSize = n;
				m = sendto(s_fd, &p1, sizeof(p1), 0, (struct sockaddr*)&serverSocket, l);
				flag = 1;
				break;
			}

			//here actual process of sending start(seqNumber+Buffer[500])
	        p1.seqNumber = counter;
	        p1.dataSize = n;
	        m = sendto(s_fd, &p1, sizeof(p1), 0, (struct sockaddr*)&serverSocket, l);
	        
	        //delay(1000);

	//copying data to the local buffers
	        if (p1.seqNumber == 1){
	        	dataSize1 = n;
	      		for(int k=0;k<sizeof(p1.Buffer);k++){
	        		dataBuffer1[k]=p1.Buffer[k];
	        	}
	        }//if ends

	        else if (p1.seqNumber == 2){
	        	dataSize2 = n;
	        	for(int k=0;k<sizeof(p1.Buffer);k++){
	        		dataBuffer2[k]=p1.Buffer[k];
	        	}
			}//else-if ends

			else if (p1.seqNumber == 3){
	        	dataSize3 = n;
	        	for(int k=0;k<sizeof(p1.Buffer);k++){
	        		dataBuffer3[k]=p1.Buffer[k];
	        	}
        	}//else-if ends

        	else if (p1.seqNumber == 4){
	        	dataSize4 = n;
	        	// strncpy(buffer4,p1.Buffer,sizeof(p1.Buffer));
	        	for(int k=0;k<sizeof(p1.Buffer);k++){
	        		dataBuffer4[k]=p1.Buffer[k];
	        	}
        	}//else-if ends

        	else if (p1.seqNumber == 5){
	        	dataSize5 = n;
	        	// strncpy(buffer5,p1.Buffer,sizeof(p1.Buffer));
	        	for(int k=0;k<sizeof(p1.Buffer);k++){
	        		dataBuffer5[k]=p1.Buffer[k];
	        	}
        	}//else-if ends

	        total_packets_count++;
	        
			counter++;
		/*initilizing payLoad buffer after every receiving untill counter
		* reaches to 6
		*/ 			
			if (counter != 6){
				for (int i = 0; i < sizeof(p1.Buffer); i++)
				{
					p1.Buffer[i]= '\0';
				}
        		n = read(f_d, p1.Buffer, bufferSize);
        		printf("\nSending Packet Counter: %d\n", counter);
        		// fputs(p1.Buffer,stdout);
			}//if ends
		}//packet sender while loop ends


		if (flag!=1){
			// Receiving ACK for Received Packets
			// Checking and Sending all missing packets
			while (1){
	        	r = recvfrom(s_fd, &p1.seqNumber, sizeof(p1.seqNumber), 0, (struct sockaddr *)&serverSocket, &l);
	        	printf("______seq number receiving___:%d\n",p1.seqNumber);
	        	if(p1.seqNumber==-1){

	    			if(seqAcks[1]==-1){

				        p1.seqNumber = 1;
				        p1.dataSize = dataSize1;
			        	// strncpy(p1.Buffer,buffer1,sizeof(buffer1));
			        	for(int k=0;k<sizeof(dataBuffer1);k++){
	        				p1.Buffer[k]=dataBuffer1[k];
	        			}
				        m = sendto(s_fd, &p1, sizeof(p1), 0, (struct sockaddr*)&serverSocket, l);

	    			}
	    			if(seqAcks[2]==-1){
				        p1.seqNumber = 2;
				        p1.dataSize = dataSize2;
			        	for(int k=0;k<sizeof(dataBuffer2);k++){
	        				p1.Buffer[k]=dataBuffer2[k];
	        			}
				        m = sendto(s_fd, &p1, sizeof(p1), 0, (struct sockaddr*)&serverSocket, l);
	    			
	    			}//if-ends
	    			if(seqAcks[3]==-1){
				        p1.seqNumber = 3;
				        p1.dataSize = dataSize3;
			        	for(int k=0;k<sizeof(dataBuffer3);k++){
	        				p1.Buffer[k]=dataBuffer3[k];
	        			}
				        m = sendto(s_fd, &p1, sizeof(p1), 0, (struct sockaddr*)&serverSocket, l);
	    			
	    			}//if ends

	    			if(seqAcks[4]==-1){
				        p1.seqNumber = 4;
				        p1.dataSize = dataSize4;
			        	for(int k=0;k<sizeof(dataBuffer4);k++){
	        				p1.Buffer[k]=dataBuffer4[k];
	        			}
				        m = sendto(s_fd, &p1, sizeof(p1), 0, (struct sockaddr*)&serverSocket, l);
	    			
	    			}//if ends

	    			if(seqAcks[5]==-1){
				        p1.seqNumber = 5;
				        p1.dataSize = dataSize5;
			        	for(int k=0;k<sizeof(dataBuffer5);k++){
	        				p1.Buffer[k]=dataBuffer5[k];
	        			}
				        m = sendto(s_fd, &p1, sizeof(p1), 0, (struct sockaddr*)&serverSocket, l);
	    			
	    			}//if ends

	    		/*	making check if there's array full with expected sequence numbers then break this and 
					make program ready for transmission of next window
				*/
	    			if(seqAcks[1]!=-1 && seqAcks[2]!=-1 && seqAcks[3]!=-1 && seqAcks[4]!=-1 && seqAcks[5]!=-1){
	    				break;
	    			}

	        	}//outer if statement is ending here
	        	else{
		        	seqAcks[p1.seqNumber] = p1.seqNumber;
	        	}//else followed by outer if...
	    	}//while(1) is ending here

		}//main if ends
		else{
			break;
		}

		// Reseting seqAcks
		for (int i = 0; i < 6; i++)
		{
        	seqAcks[i] = -1;
		}
    	counter = 1;
		count = 1;
		
		if (m == -1){
			perror("send error");
			return EXIT_FAILURE;
		}
		// printf("Sending...\n");
		sum += m;
		bzero(&p1, sizeof(p1));

		for (int i = 0; i < sizeof(p1.Buffer); i++)
		{
			p1.Buffer[i]= '\0';
		}

//reading data from file untill EOF reaches
		n = read(f_d, p1.Buffer, bufferSize);
		if (n == 0){
			p1.seqNumber = -2;
			p1.dataSize = n;
			// while (re < 1){
				m = sendto(s_fd, &p1 ,sizeof(p1), 0, (struct sockaddr*)&serverSocket, l);
				// re++;
			// }
			// re = 0;
			break;
		}
	}
	printf("File Sent\n");
	sizeOfLastPacket = size % 500;
	gettimeofday(&stop, NULL);
	duratn(&start, &stop, &delta);
	printf("Number of Packets: %d \n", total_packets_count);
	printf("Size of Last Packet: %d \n", sizeOfLastPacket);
	printf("Number of bytes transferred : %ld\n", sum);
	printf("On a total size of: %ld \n", size);
	printf("For a total duration of : %ld.%ld \n", delta.tv_sec, delta.tv_usec);

	close(s_fd);
	close(f_d);
	return EXIT_SUCCESS;
}
/*Function allowing the calculation of the duration of the sending*/
int duratn(struct timeval *start, struct timeval *stop, struct timeval *delta)
{
	suseconds_t microstart, microstop, microdelta;

	microstart = (suseconds_t)(5000 * (start->tv_sec)) + start->tv_usec;
	microstop = (suseconds_t)(5000 * (stop->tv_sec)) + stop->tv_usec;
	microdelta = microstop - microstart;

	delta->tv_usec = microdelta % 5000;
	delta->tv_sec = (time_t)(microdelta / 5000);

	if ((*delta).tv_sec < 0 || (*delta).tv_usec < 0)
		return -1;
	else
		return 0;
}

/* Function allowing the creation of a socket
* Returns a file descriptor
*/
int clt_sock_create(int prt, char* i_paddr){
	int l;
	int s_fd;

	s_fd = socket(AF_INET, SOCK_DGRAM, 0);
	if (s_fd == -1){
		perror("socket fail");
		return EXIT_FAILURE;
	}

	//preparation of the address of the destination socket
	l = sizeof(struct sockaddr_in);
	bzero(&serverSocket, l);

	serverSocket.sin_family = AF_INET;
	serverSocket.sin_port = htons(prt);
	if (inet_pton(AF_INET, i_paddr, &serverSocket.sin_addr) == 0){
		printf("Invalid IP adress\n");
		return EXIT_FAILURE;
	}

	return s_fd;
}
void delay(int number_of_seconds)
{
    // Converting time into milli_seconds
    int milli_seconds = 1000 * number_of_seconds;

    // Stroing start time
    clock_t start_time = clock();

    // looping till required time is not acheived
    while (clock() < start_time + milli_seconds);
}
