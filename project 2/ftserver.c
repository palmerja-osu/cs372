#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <netdb.h>
#include <dirent.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>


struct addrinfo * create_address_info(char * port){
	int status;
	struct addrinfo hints;
	struct addrinfo * res;
	
	//return info of address
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	//error output
	if((status = getaddrinfo(NULL, port, &hints, &res)) != 0){
		fprintf(stderr,
				"getaddrinfo error: %s\nDid you enter the correct IP/Port?\n",
				gai_strerror(status));
		exit(1);
	}
	
	return res;
}


struct addrinfo * create_address_info_with_ip(char * ip_address, char * port){
	int status;
	struct addrinfo hints;
	struct addrinfo * res;
	
	//return address of linked list
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;

	//error output
	if((status = getaddrinfo(ip_address, port, &hints, &res)) != 0){
		fprintf(stderr, "ERROR! %s\nDid you enter the correct IP/Port?\n", gai_strerror(status));
		exit(1);
	}
	
	return res;
}


int create_socket(struct addrinfo * res){
	int sockfd;
	
	//returning socket file
	if ((sockfd = socket((struct addrinfo *)(res)->ai_family, res->ai_socktype, res->ai_protocol)) == -1){
		fprintf(stderr, "ERROR! Cannot create socket\n");
		exit(1);
	}
	return sockfd;
}


void connect_socket(int sockfd, struct addrinfo * res){
	int status;
	
	//connects the address infrom from the linked list
	if ((status = connect(sockfd, res->ai_addr, res->ai_addrlen)) == -1){
		fprintf(stderr, "ERROR! Cannot connect socket\n"); //error message
		exit(1);
	}
}


void bind_socket(int sockfd, struct addrinfo * res){
	//add socket to file descriptor
	if (bind(sockfd, res->ai_addr, res->ai_addrlen) == -1) {
		close(sockfd);
		
		//error message
		fprintf(stderr, "ERROR! Issue binding socket\n");
		exit(1);
	}
}


void listen_socket(int sockfd){
	//call listen command 
	//attempting to bound port
	if(listen(sockfd, 5) == -1){
		close(sockfd);
		
		//error message
		fprintf(stderr, "ERROR! Issue listening to socket\n");
		exit(1);
	}
}

//heap interactions
char ** create_string_array(int size){
	//create heap string array
	char ** array = malloc(size*sizeof(char *));
	int i = 0;
	//int our number files
	for(; i < size; i++){
		array[i] = malloc(100*sizeof(char));
		memset(array[i],0,sizeof(array[i]));
	}
	return array;
}

//heap interactions
void delete_string_array(char ** array, int size){
	int i = 0;
	//delete string array on the heap
	for (; i < size; i++){
		free(array[i]);
	}
	free(array);
}


int get_available_files(char ** files){
	DIR * d;
	struct dirent * dir;
	d = opendir(".");
	int i = 0;
	
	//counts number of files
	if (d){
		while ((dir = readdir(d)) != NULL){
			//place string files into our array
			if (dir->d_type == DT_REG){
				strcpy(files[i], dir->d_name);
				i++;
			}
		}
		closedir(d);
	}
	return i;
}

//Does the file name exist?
int does_file_exist(char ** files, int num_files, char * filename){
	int file_exists = 0;
	int i = 0;
	
	//loop through files, if the filename we want is in there, set the file_exists
	for (; i < num_files; i++){
		if(strcmp(files[i], filename) == 0){
			file_exists = 1;
		}
	}
	return file_exists;
}

//connecting data sockets to file
void send_file(char * ip_address, char * port, char * filename){
	
	//sleep command to make sure that python has enough time to setup its data socket
	sleep(2);
	struct addrinfo * res = create_address_info_with_ip(ip_address, port);
	int data_socket = create_socket(res);
	connect_socket(data_socket, res);
	
	//create and clean buffer for file contents
	char buffer[1000];
	memset(buffer, 0, sizeof(buffer));
	//open the file
	int fd = open(filename, O_RDONLY);
	while (1) {
		//Read data into buffer
		int bytes_read = read(fd, buffer, sizeof(buffer)-1);
		if (bytes_read == 0) 
			break;
		//error message check
		if (bytes_read < 0) {
			fprintf(stderr, "Error reading file\n");
			return;
		}

		
		//p keeps track of where in the buffer we are
		void *p = buffer;
		while (bytes_read > 0) {
			//write returns how many bytes were written.
			int bytes_written = send(data_socket, p, sizeof(buffer),0);
			if (bytes_written < 0) {
				fprintf(stderr, "ERROR! With writing to socket\n");
				return;
			}
			//bytes_read keeps track of how many bytes are left to write.
			bytes_read -= bytes_written;
			p += bytes_written;
		}
		//Added buffer clear for erroring
		memset(buffer, 0, sizeof(buffer));
	}
	//clear out buffer 
	memset(buffer, 0, sizeof(buffer));
	//added output message to double check
	strcpy(buffer, "** File Buffering Finished ***");
	send(data_socket, buffer, sizeof(buffer),0);
	//close socket and free address info
	close(data_socket);
	freeaddrinfo(res);
}


//connect data sockets to directory
void send_directory(char * ip_address, char * port, char ** files, int num_files){

	//sleep command to make sure that python has enough time to setup its data socket
	sleep(2);
	struct addrinfo * res = create_address_info_with_ip(ip_address, port);
	int data_socket = create_socket(res);
	connect_socket(data_socket, res);
	
	//loop through filenames and send to client
	int i = 0;
	for (; i < num_files; i++){
		send(data_socket, files[i], 100,0);
	}
	//added output message to double check
	char * done_message = "** Directory Buffering Finished ***";
	send(data_socket, done_message, strlen(done_message),0);
	// close socket and free address information
	close(data_socket);
	freeaddrinfo(res);
}

//Handle client request
void handle_request(int new_fd){
	
	//get client port number
	//confirm with outputs
	char * ok_message = "ok message";
	char * bad_message = "bad message";
	char port[100];
	memset(port, 0, sizeof(port));
	recv(new_fd, port, sizeof(port)-1, 0);
	
	//send message to fix bug when python oversends information *personal check*
	send(new_fd, ok_message, strlen(ok_message),0);
	
	char command[100];
	memset(command,0,sizeof(command));
	recv(new_fd, command, sizeof(command)-1, 0);
	//re-send message to fix bug when python oversends information *personal check*
	send(new_fd, ok_message, strlen(ok_message),0);
	
	//receive client ip
	char ip_address[100];
	memset(ip_address,0,sizeof(ip_address));
	recv(new_fd, ip_address, sizeof(ip_address)-1,0);
	
	
	//output connection with address 
	printf("Connection from address: %s\n", ip_address);

	if(strcmp(command,"l") == 0){
		//send confirmation message to client stating command functioned properly
		send(new_fd, ok_message, strlen(ok_message),0);
		printf("File requested from port: %s\n", port);
		printf("Sending file list to: %s from port %s\n", ip_address, port);
		//create string array for files in our dir
		char ** files = create_string_array(100);
		int num_files = get_available_files(files);
		
		//send contents of the file array to the client
		send_directory(ip_address, port, files, num_files);
		//free memory
		delete_string_array(files,100);
	}
	
	else if(strcmp(command, "g") == 0){
		//send confirmation message to client stating command functioned properly
		send(new_fd, ok_message, strlen(ok_message),0);
		//get file name from client
		char filename[100];
		memset(filename, 0, sizeof(filename));
		recv(new_fd, filename, sizeof(filename)-1,0);
		printf("File: %s requested from port: %s\n", filename, port);
		
		//check for existing file
		char ** files = create_string_array(100);
		int num_files = get_available_files(files);
		int file_exists = does_file_exist(files, num_files, filename);
		
		//If file exist, confirm with client
		if(file_exists){
			printf("Client file found! Sending %s to client\n", filename);
			char * file_found = "File found";
			send(new_fd, file_found, strlen(file_found),0);
			
			//create new filename based on current location
			//send new file to client
			char new_filename[100];
			memset(new_filename,0,sizeof(new_filename));
			strcpy(new_filename, "./");
			char * end = new_filename + strlen(new_filename);
			end += sprintf(end, "%s", filename);
			send_file(ip_address, port, new_filename);
		}
		//else/error if file isn't found
		else{
			
			printf("File not found! Client error message\n");
			char * file_not_found = "File not found";
			send(new_fd, file_not_found, 100, 0);
		}
		delete_string_array(files, 100);
	}
	//second error massge when initial command is missing. 
	else{
		send(new_fd, bad_message, strlen(bad_message), 0);
		printf("Invalid command sent to server. Try again! \n");
	}
	//Finishing output text to see where I am during output.
	printf("Finished processing user request.........what now????\n");
}

//Waiting on user
void wait_for_connection(int sockfd){

	//create container connection
	struct sockaddr_storage their_addr;
	//create connection size
    socklen_t addr_size;
	//create new file descriptor for connection
	int new_fd;

	while(1){
		//address size
		addr_size = sizeof(their_addr);
		//new client
		new_fd = accept(sockfd, (struct addrinfo *)&their_addr, &addr_size);
		//if no new client...... keep waiting
		if(new_fd == -1){
			//continue connection
			//continue to run call
			continue;
		}
		//handle client request
		handle_request(new_fd);
		close(new_fd);
	}
}

//main function
int main(int argc, char *argv[]){
	if(argc != 2){
		fprintf(stderr, "ERROR! Invalid number of arguments\n");
		exit(1);
	}
	
	//function calls
	printf("Current server open on port: %s\n", argv[1]);
	struct addrinfo * res = create_address_info(argv[1]);
	int sockfd = create_socket(res);
	bind_socket(sockfd, res);
	listen_socket(sockfd);
	wait_for_connection(sockfd);
	freeaddrinfo(res);
}