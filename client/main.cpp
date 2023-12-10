#include "DataBaseClient.h"

const int PORT = 8080;

int main(int argc, char const* argv[]){
	if(argc < 2){
		std::cout << "Config path is not provided";
		return 0;
	}

	DB::DataBaseClient db;
	try{
		db.init(argv[1]);
	}
	catch(std::exception &msg){
		std::cout << msg.what();
		return 0;
	}
	
	std::string command;
	bool exit = false;

	std::map<std::string, DB::REQUEST_TYPE> str2cmd{
		{std::string("add"), DB::REQUEST_TYPE::add},
		{std::string("remove"), DB::REQUEST_TYPE::remove},
		{std::string("edit"), DB::REQUEST_TYPE::edit},
		{std::string("get"), DB::REQUEST_TYPE::get},
		{std::string("dump"), DB::REQUEST_TYPE::dump},
		{std::string("filter"), DB::REQUEST_TYPE::filter},
		{std::string("size"), DB::REQUEST_TYPE::size},
		{std::string("stop"), DB::REQUEST_TYPE::stop},
		{std::string("save"), DB::REQUEST_TYPE::save},
		{std::string("exit"), DB::REQUEST_TYPE::end}
	};
	while(!exit){
	 	std::cin >> command;
		if(str2cmd.find(command) == str2cmd.end()){
			if(command == "exit"){
				exit = true;
			}
			else{
				std::cout << "Unknown command: " << command << std::endl;
				std::cin.ignore(1024, '\n');
			}
			continue;
		}
		DB::REQUEST_TYPE request = str2cmd[command];
		getline(std::cin, command);
		if(command.size() > 0){
			command = command.substr(command.find_first_not_of(' '));
		}

		size_t size;
		size_t idx;
		size_t jdx;
		std::string result;
		DB::Entry filter;
		DB::Entry current;
		bool match;
		switch (request){
		case DB::REQUEST_TYPE::add:
			if(db.is_valid_entry(command)){
				std::cout << db.send_command(std::string{request} + command) << std::endl;
			}
			else{
				std::cout << "Entry is invalid" << std::endl;
			}
			break;
		
		case DB::REQUEST_TYPE::get:
		case DB::REQUEST_TYPE::remove:
			result = db.send_command(std::string{request} + command);
			if(result.size() == 0){
				std::cout << "Field does not exist" << std::endl;
				break;
			}
			std::cout << result << std::endl;
			break;
		
		case DB::REQUEST_TYPE::edit:
			for(idx = 0; idx < command.size() && '0' <= command[idx] && command[idx] <= '9'; ++idx);
			if(db.is_valid_entry(command.substr(idx))){
				result = db.send_command(std::string{request} + command);
				std::cout << command << std::endl;

				if(result.size() == 0){
					std::cout << "Entry does not exist" << std::endl;
					break;
				}
				std::cout << result << std::endl;
			}
			else{
				std::cout << "Entry is invalid" << std::endl;
			}
			break;
		
		case DB::REQUEST_TYPE::dump:
			result = db.send_command(std::string{DB::REQUEST_TYPE::size});
			size = stoi(result);
			for(idx = 0; idx < size; ++idx){
				result = db.send_command(std::string{DB::REQUEST_TYPE::get} + std::to_string(idx));
				usleep(10);
				if(result.size() > 0){
					std::cout << idx << ": " << result << std::endl;
				}
			}
			break;

		case DB::REQUEST_TYPE::filter:
			result = db.send_command(std::string{DB::REQUEST_TYPE::size});
			size = stoi(result);
			filter = db.StringToEntry(command.c_str(), false);
			for(idx = 0; idx < size; ++idx){
				result = db.send_command(std::string{DB::REQUEST_TYPE::get} + std::to_string(idx));
				usleep(10);
				current = db.StringToEntry(result.c_str());

				match = true;
				for(jdx = 0; jdx < current.size() && match; ++jdx){
					if(filter[jdx].get() != nullptr){
						match = filter[jdx]->getValue() == current[jdx]->getValue();
					}
				}

				if(match && result.size() > 0){
					std::cout << idx << ": " << result << std::endl;
				}
			}
			break;
		
		case DB::REQUEST_TYPE::stop:
			exit = true;
		case DB::REQUEST_TYPE::size:
		case DB::REQUEST_TYPE::save:
			std::cout << db.send_command(std::string{request}) << std::endl;
			break;

		default:
			exit = true;
			break;
		}
	}
	
	return 0;
}