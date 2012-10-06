/*
 ** server.c -- a stream socket server demo
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define MYPORT 12016    // the port users will be connecting to
#define BACKLOG 10     // how many pending connections queue will hold

int main(void)
{

	int sockfd, new_fd;  // listen on sock_fd, new connection on new_fd
	struct sockaddr_in my_addr;    // my address information
	struct sockaddr_in their_addr; // connector's address information
	int sin_size;
	int yes=1;

	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		perror("socket");
		exit(1);
	}

	if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
		perror("setsockopt");
		exit(1);
	}

	my_addr.sin_family = AF_INET;         // host byte order
	my_addr.sin_port = htons(MYPORT);     // short, network byte order
	my_addr.sin_addr.s_addr = INADDR_ANY; // automatically fill with my IP
	memset(&(my_addr.sin_zero), '\0', 8); // zero the rest of the struct

	if (bind(sockfd, (struct sockaddr *)&my_addr, sizeof(struct sockaddr)) == -1) {
		perror("bind");
		exit(1);
	}

	if (listen(sockfd, BACKLOG) == -1) {
		perror("listen");
		exit(1);
	}

#define BUFFER_SIZE 256

	while(1) {  // main accept() loop

		sin_size = sizeof(struct sockaddr_in);
		if ((new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size)) == -1) {
			perror("accept");
			continue;
		}
		printf("server: got connection from %s\n",inet_ntoa(their_addr.sin_addr));

		char bufferStr[BUFFER_SIZE]={0};

		// recv
		if(recv(new_fd, bufferStr, BUFFER_SIZE-1, 0)<0) perror("read");
		else{
			
			//printf("%s",bufferStr);
			char *pch1 = strtok(bufferStr, " ");
			char *pch2 = strtok(NULL, " ");

			if(strcmp(pch1, "GET")!=0){

				const char *response="HTTP/1.0 400 Bad Request\r\n"
				"Connection: close\r\n"
				"Content-type: text/html\r\n"
				"\r\n"
				"<html><head><title>400 Bad Request</title></head><body>Bad Request</body></html>\r\n";

				int sendResult = send(new_fd, response, strlen(response), 0);
				if(sendResult == -1) perror("send");
				continue;

			}

			char file_position[128]={0};
			FILE *requested_file;
			int file_found = 0;
			if(pch2[0]=='/'){
			
				file_position[0]='.';
				strcat(file_position, pch2);
				requested_file=fopen(file_position, "r");
				if(requested_file) file_found=1;

			}

			if(file_found){

				char tmpstring[64];
				int end;

				fseek(requested_file, 0, SEEK_END); 
				end=ftell(requested_file);
				sprintf(tmpstring, "file size: %d<br/>\n", end);

				char response[256];
				char *http_response="HTTP/1.0 200 OK\r\n"
				"Connection: close\r\n"
				"Content-type: text/html\r\n"
				"\r\n"
				"<html><head><title>Found</title></head><body>File found.<br/>\r\n";
				strcat(response, http_response);
				strcat(response, tmpstring);
				strcat(response, "</body></html>\r\n");

				int sendResult = send(new_fd, response, strlen(response), 0);
				if(sendResult == -1) perror("send");

			}
			else{

				const char *response="HTTP/1.0 404 Not Found\r\n"
				"Connection: close\r\n"
				"Content-type: text/html\r\n"
				"\r\n"
				"<html><head><title>404 Not Found</title></head><body>Not found</body></html>\r\n";

				int sendResult = send(new_fd, response, strlen(response), 0);
				if(sendResult == -1) perror("send");

			}
		}	

		close(new_fd);
	}


	return 0;
}

