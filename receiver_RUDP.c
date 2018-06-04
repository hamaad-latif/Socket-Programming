#include <stdio.h>
#include <stdlib.h>

// Time function, sockets, htons... file stat
#include <sys/time.h>
#include <time.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <sys/stat.h>
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

/* Declaration of functions*/
int duratn(struct timeval *start, struct timeval *stop, struct timeval *delta);
int serverSocket(int prt);
void delay(int number_of_seconds);
int total_packets_count = 1;
int time_out(int file_des, unsigned int secnds);
int m;

/* buffers for maintaining incoming 
*  data for 5 different sequence numbers
*/
char dataBuffer1[500] ;
char dataBuffer2[500] ;
char dataBuffer3[500] ;
char dataBuffer4[500] ;
char dataBuffer5[500] ;
int flag = 0;
int a = 0;
int buff = 0;
int count = 1;

int seqNumberAckArray[6] =  { -1, -1, -1, -1, -1, -1 };
int counter = 1;

/*payload declared as same struct as we have
*done in case of senders
*/
struct packetPayload{
    char Buffer[bufferSize];
    int seqNumber;
    int dataSize;
};

/* variables that will store the size of incoming 
*  buffers of each incoming window's packcets
*/
int dataSize1;
int dataSize2;
int dataSize3;
int dataSize4;
int dataSize5;


struct sockaddr_in serverSock, client;


// New Declared
int packet_counter=1;
int missing_counter=0;
int missing_packets[5];


//main function starts
int main(int argmnt_c, char**argmnt_v){
	int f_d, s_fd;//file distributors
	struct pollfd fd;
	int ret;
	struct timeval tv;

	struct packetPayload p1;//payload variable
	char buf[12];
	char buffer[bufferSize];
	off_t sum = 0, n; // long type
	char file_name[256];
	unsigned int l = sizeof(struct sockaddr_in);
	time_t intps;
	struct tm* tmi;
int i;
tv.tv_sec = 5;
tv.tv_usec = 0;

	p1.seqNumber = -1;

	if (argmnt_c != 2){
		printf("Error usage : %s <port_serv>\n", argmnt_v[0]);
		return EXIT_FAILURE;
	}
	s_fd = serverSocket(atoi(argmnt_v[1]));
	intps = time(NULL);
	tmi = localtime(&intps);
	bzero(file_name, 256);
	sprintf(file_name, "client.%d.%d.%d.%d.%d.%d", tmi->tm_mday, tmi->tm_mon + 1, 1900 + tmi->tm_year, tmi->tm_hour, tmi->tm_min, tmi->tm_sec);
	printf("Creating the output file : %s\n", file_name);
	if ((f_d = open(file_name, O_CREAT | O_WRONLY | O_TRUNC, 0600)) == -1){
		perror("open fail");
		return EXIT_FAILURE;
	}
	bzero(&p1, sizeof(p1));
	
	// setsockopt(s_fd, SOL_SOCKET, SO_RCVTIMEO,(char *)&tv,sizeof(struct timeval));

//receiving starts from here
	n = recvfrom(s_fd, &p1, sizeof(p1), 0, (struct sockaddr *)&client, &l);
	//delay(1000);
//receining loop
	while (n){
		if (n == -1){
			perror("read fails");
			return EXIT_FAILURE;
		}//if ends
		
	//loop for dealing with window size of 5 pakcets
		while (packet_counter < 6){

			printf("\nSeq received by sender: %d\n", p1.seqNumber);

			if (p1.seqNumber == 1){
				for(int k=0;k<sizeof(p1.Buffer);k++){
	        		dataBuffer1[k]=p1.Buffer[k];
	        	}
				dataSize1 = p1.dataSize;
				seqNumberAckArray[1] = p1.seqNumber;
				// delay(100);
				//m = sendto(s_fd, &p1.seq, sizeof(p1.seq), 0, (struct sockaddr*)&client, l);
				}
			else if (p1.seqNumber == 2){
				for(int k=0;k<sizeof(p1.Buffer);k++){
	        		dataBuffer2[k]=p1.Buffer[k];
	        	}
				dataSize2 = p1.dataSize;
				seqNumberAckArray[2] = p1.seqNumber;
			}//else-if ends

			else if (p1.seqNumber == 3){
				for(int k=0;k<sizeof(p1.Buffer);k++){
	        		dataBuffer3[k]=p1.Buffer[k];
	        	}
				dataSize3 = p1.dataSize;
				seqNumberAckArray[3] = p1.seqNumber;
			}//else-if ends

			else if (p1.seqNumber == 4){
				for(int k=0;k<sizeof(p1.Buffer);k++){
	        		dataBuffer4[k]=p1.Buffer[k];
	        	}
				dataSize4 = p1.dataSize;
				seqNumberAckArray[4] = p1.seqNumber;
			}//else-if ends

			else if (p1.seqNumber == 5){
				for(int k=0;k<sizeof(p1.Buffer);k++){
	        		dataBuffer5[k]=p1.Buffer[k];
	        	}
				dataSize5 = p1.dataSize;
				seqNumberAckArray[5] = p1.seqNumber;
			}//else-if ends

			packet_counter++;

		/*initilizing payLoad buffer after every receiving untill counter
		* reaches to 6
		*/ 
			if(packet_counter != 6){
				for (int i = 0; i < sizeof(p1.Buffer); i++) {
					p1.Buffer[i]= '\0';
				}//for ends
				n = recvfrom(s_fd, &p1, sizeof(p1), 0, (struct sockaddr *)&client, &l);
				total_packets_count++;
				if (p1.seqNumber == -2){
					flag = 1;
					break;
				}//inner if
			}//paket counter if
		}//while(packet_counter) is ending here

//dealing with missing packets
		while (missing_counter >= 0){

			// delay(1000);
			// Sending Ack to sender
			for(count =1 ; count < packet_counter ;count++){
				printf("______seq number sending___:%d\n",p1.seqNumber);
				if(seqNumberAckArray[count]!=-1){
					// packet_counter
					p1.seqNumber = count;
					printf("\n\n\nSequence is %d\n\n", count);
					m = sendto(s_fd, &p1.seqNumber, sizeof(p1.seqNumber), 0, (struct sockaddr*)&client, l);				
				}
				else{
					missing_counter++;
					missing_packets[missing_counter] = count;
				}
			}

			printf("\nMissing counter%d\n", missing_counter);

			p1.seqNumber=-1;
			m = sendto(s_fd, &p1.seqNumber, sizeof(p1.seqNumber), 0, (struct sockaddr*)&client, l);

			// Reseting sequence array for getting missing sequence
			for (int i = 0; i < 6; i++)
			{
				seqNumberAckArray[count]=-1;
			}

			int remaining_missing=missing_counter;

		// Recieving missing packets
			for (int i = 0; i < missing_counter; i++)
			{
				n = recvfrom(s_fd, &p1, sizeof(p1), 0, (struct sockaddr *)&client, &l);
				if(missing_packets[i]==1){
					// strncpy(buffer1,p1.Buffer,sizeof(p1.Buffer));
					for(int k=0;k<sizeof(p1.Buffer);k++){
	        			dataBuffer1[k]=p1.Buffer[k];
	        		}
					dataSize1 = p1.dataSize;
					seqNumberAckArray[1] = p1.seqNumber;
					remaining_missing--;

				}

				else if(missing_packets[i]==2){
					// strncpy(buffer2,p1.Buffer,sizeof(p1.Buffer));
					for(int k=0;k<sizeof(p1.Buffer);k++){
	        			dataBuffer2[k]=p1.Buffer[k];
	        		}
					dataSize2 = p1.dataSize;
					seqNumberAckArray[2] = p1.seqNumber;
					remaining_missing--;
				}//else-if ends

				else if(missing_packets[i]==3){

					// strncpy(buffer3,p1.Buffer,sizeof(p1.Buffer));
					for(int k=0;k<sizeof(p1.Buffer);k++){
	        			dataBuffer3[k]=p1.Buffer[k];
	        		}
					dataSize3 = p1.dataSize;
					seqNumberAckArray[3] = p1.seqNumber;
					remaining_missing--;
				}//else-if ends

				else if(missing_packets[i]==4){
					// strncpy(buffer4,p1.Buffer,sizeof(p1.Buffer));
					for(int k=0;k<sizeof(p1.Buffer);k++){
	        			dataBuffer4[k]=p1.Buffer[k];
	        		}
					dataSize4 = p1.dataSize;
					seqNumberAckArray[4] = p1.seqNumber;
					remaining_missing--;
				}//else-if ends

				else if(missing_packets[i]==5){
					// strncpy(buffer5,p1.Buffer,sizeof(p1.Buffer));
					for(int k=0;k<sizeof(p1.Buffer);k++){
	        			dataBuffer5[k]=p1.Buffer[k];
	        		}
					dataSize5 = p1.dataSize;
					seqNumberAckArray[5] = p1.seqNumber;
					remaining_missing--;

				}//else-if ends

			}//for(missing_counter) ends here

			missing_counter=remaining_missing;

			printf("\nNext Missing counter%d\n", missing_counter);

			if(missing_counter == 0){
				missing_counter=-1;
				printf("\nzbrdsti BREAK lag gai raja ji\n");
				break;
			}

		}

	// Reseting the missing packets array
		printf("\nBreaking----------Bad chalao raja ji!!!\n");

		packet_counter=1;
		missing_counter=0;
		for (int i = 0; i < 5; i++)
		{
			missing_packets[i]='\0';
		}

//writtin kro g window fit hogai poori
		for (counter = 1; counter < 6; counter ++){
			if (counter == 1){
				write(f_d, &dataBuffer1, dataSize1);
				dataSize1 = 0;

				// Reseting the received buffer after writing
				for (int i = 0; i < sizeof(dataBuffer1); i++)
				{
					dataBuffer1[i]='\0';
				}
				
			}
			else if (counter == 2){
				write(f_d, &dataBuffer2, dataSize2);
				dataSize2 = 0;

				// Reseting the received buffer after writing
				for (int i = 0; i < sizeof(dataBuffer2); i++)
				{
					dataBuffer2[i]='\0';
				}
				// //getchar();
				// printf("writing buffer 2\n");
					}
			else if (counter == 3){
				write(f_d, &dataBuffer3, dataSize3);
				dataSize3 = 0;		

				// Reseting the received buffer after writing
				for (int i = 0; i < sizeof(dataBuffer3); i++)
				{
					dataBuffer3[i]='\0';
				}			
		
			}
			else if (counter ==4){
				write(f_d, &dataBuffer4, dataSize4);
				dataSize4 = 0;

				// Reseting the received buffer after writing
				for (int i = 0; i < sizeof(dataBuffer4); i++)
				{
					dataBuffer4[i]='\0';
				}

			}
			else if (counter == 5){
				write(f_d, &dataBuffer5, dataSize5);
				dataSize5 = 0;

				// Reseting the received buffer after writing
				for (int i = 0; i < sizeof(dataBuffer5); i++)
				{
					dataBuffer5[i]='\0';
				}

			}
		}
		sum += n;
		total_packets_count += 1;
		//strncpy(buffer1,Nullbuffer,sizeof(Nullbuffer));
		for(counter = 1; counter < 6; counter ++){
			seqNumberAckArray[counter] = -1;
		}

		bzero(&p1, sizeof(p1));

//resetting payload buffer after receiving
		for (int i = 0; i < sizeof(p1.Buffer); i++)
		{
			p1.Buffer[i]= '\0';
		}


		n = recvfrom(s_fd, &p1, sizeof(p1), 0, (struct sockaddr *)&client, &l);
		if (p1.seqNumber == -2){
			flag = 1;
			break;
		}
	}//outer while(n) ends here
	printf("Number of bytes transferred: %ld \n", sum);
	printf("Number of Packets received: %d \n", total_packets_count);

	close(s_fd);
	close(f_d);
	printf("File Received!! \n");
	return EXIT_SUCCESS;
}//main function ends here


		/* Function allowing the calculation of the duration of the sending*/
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
	}//duration calculator function 

/* Function allowing the creation of a socket and its attachment to the system
* Returns a file descriptor in the process descriptor table
* bind allows its definition in the system */
int serverSocket(int prt){
	int l;
	int s_fd;

	s_fd = socket(AF_INET, SOCK_DGRAM, 0);
	if (s_fd == -1){
		perror("socket fail");
		return EXIT_FAILURE;
	}

	//preparation of the address of the destination socket
	l = sizeof(struct sockaddr_in);
	bzero(&serverSock, l);

	serverSock.sin_family = AF_INET;
	serverSock.sin_port = htons(prt);
	serverSock.sin_addr.s_addr = htonl(INADDR_ANY);

	//Assign an identity to the socket
	if (bind(s_fd, (struct sockaddr*)&serverSock, l) == -1){
		perror("bind fail");
		return EXIT_FAILURE;
	}


	return s_fd;
}//server socket creation

 void delay(int number_of_seconds){
    // Converting time into milli_seconds
    int milli_seconds = 1000 * number_of_seconds;

    // Stroing start time
    clock_t start_time = clock();

    // looping till required time is not acheived
    while (clock() < start_time + milli_seconds);
}//delay function
