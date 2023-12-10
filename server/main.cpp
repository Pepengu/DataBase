#include "DataBaseServer.h"
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <cstring>
#include <iostream>


int main(int argc, char const* argv[]){
	if(argc < 2){
		std::cout << "Config path is not provided";
		return 0;
	}

	DB::DataBaseServer db;
	try{
		db.init(argv[1]);
	}
	catch(std::exception &msg){
		std::cout << msg.what();
		return 0;
	}
	
	bool not_ended = true;
	while(not_ended){
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

		read(new_socket, buffer, 1024);

		size_t idx = 0;
		try{
			idx = db.validateRequest(buffer);
		}
		catch(DB::STATUS &error){
			send(new_socket, reinterpret_cast<char*>(&error), 1, 0);
			close(new_socket);
			shutdown(server_fd, SHUT_RDWR);
			close(server_fd);
			continue;
		}

		DB::REQUEST_TYPE request = DB::REQUEST_TYPE(buffer[idx++]);
		db.log << "request with id " + std::to_string(request) + " recieved from user " + std::to_string(new_socket);
		std::string result;
		char msg[1024]{};
		try{
			switch (request){
			case DB::REQUEST_TYPE::connection:
				msg[0] = DB::STATUS::connection_success;
				break; 

			case DB::REQUEST_TYPE::add:
				if(!db.is_valid_entry(buffer+idx)){
					throw DB::STATUS::structure_differ;
				}
				db.addRecord(buffer+idx);
				msg[0] = DB::STATUS::addition_success;
				break;
			
			case DB::REQUEST_TYPE::remove:
				db.remove(atoi(buffer+idx));
				msg[0] = DB::STATUS::removal_success;
				break;
			
			case DB::REQUEST_TYPE::edit:
				db.editRecord(buffer+idx);
				msg[0] = DB::STATUS::edition_success;
				break;

			case DB::REQUEST_TYPE::save:
				db.save();
				msg[0] = DB::STATUS::save_success;
				break; 

			case DB::REQUEST_TYPE::get:
				msg[0] = DB::STATUS::get_success;
				result = db.get(atoi(buffer+idx));
				memcpy(msg+1, result.c_str(), result.size()+1);
				break; 
			
			case DB::REQUEST_TYPE::size:
				msg[0] = DB::STATUS::size_success;
				result = std::to_string(db.size());
				memcpy(msg+1, result.c_str(), result.size()+1);
				break; 

			case DB::REQUEST_TYPE::stop:
				not_ended = false;
				db.close();
				msg[0] = DB::STATUS::stop_success;
				break;

			
			default:
				break;
			}
		}
		catch(DB::STATUS error){
			msg[0] = error;
		}
		send(new_socket, msg, 1024, 0);

		db.log << "Response " + std::to_string(msg[0]) + " was sent to user " + std::to_string(new_socket);
		// closing the connected socket
		close(new_socket);
		// closing the listening socket
		shutdown(server_fd, SHUT_RDWR);
		close(server_fd);
	}
	return 0;
}