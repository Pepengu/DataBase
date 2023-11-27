#include "DataBaseServer.h"
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <cstring>
#include <iostream>


int main(int argc, char const* argv[]){
	DB::DataBaseServer db("server/config.cfg");
	
	while(true){
		int valread;
		int opt = 1;
		char buffer[1024] = { 0 };
		int server_fd = socket(AF_INET, SOCK_STREAM, 0);
		// Creating socket file descriptor
		if (server_fd < 0) {
			perror("socket failed");
			exit(EXIT_FAILURE);
		}
		// Forcefully attaching socket to the port 8080
		int res = setsockopt(
			server_fd, 
			SOL_SOCKET,
			SO_REUSEADDR | SO_REUSEPORT, 
			&opt,
			sizeof(opt)
		);
		if (res < 0) {
			perror("setsockopt");
			exit(EXIT_FAILURE);
		}
		struct sockaddr_in address;
		int addrlen = sizeof(address);
		address.sin_family = AF_INET;
		address.sin_addr.s_addr = INADDR_ANY;
		address.sin_port = htons(DB::PORT);

		// Forcefully attaching socket to the port 8080
		res = bind(
			server_fd, 
			(struct sockaddr*)&address,
			sizeof(address)
		);
		if (res	< 0) {
			perror("bind failed");
			exit(EXIT_FAILURE);
		}

		res = listen(server_fd, 3);
		if (res < 0) {
			perror("listen");
			exit(EXIT_FAILURE);
		}

		int new_socket = accept(
			server_fd, 
			(struct sockaddr*)&address,
			(socklen_t*)&addrlen
		);
		if (new_socket < 0) {
			perror("accept");
			exit(EXIT_FAILURE);
		}

		valread = read(new_socket, buffer, 1024);

		size_t idx;
		try{
			idx = db.validateRequest(buffer);
		}
		catch(std::exception &error){
			send(new_socket, error.what(), strlen(error.what()), 0);
			close(new_socket);
			shutdown(server_fd, SHUT_RDWR);
			close(server_fd);
			continue;
		}
		
		DB::REQUEST_TYPE request = DB::REQUEST_TYPE(buffer[idx]);
		char msg[1];
		switch (request)
		{
		case DB::REQUEST_TYPE::connection:
			msg[0] = DB::STATUS::connection_success;
			break; 
		
		default:
			break;
		}
		send(new_socket, msg, 1, 0);
		// closing the connected socket
		close(new_socket);
		// closing the listening socket
		shutdown(server_fd, SHUT_RDWR);
		close(server_fd);
	}
	return 0;
}