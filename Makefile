CC=g++
CFLAGS=-c -std=c++17 -pedantic-errors -g -I ./include

SERVER_SOURCES=include/DataBase.cpp include/Fields.cpp server/DataBaseServer.cpp server/main.cpp
SERVER_OBJECTS=$(SERVER_SOURCES:.cpp=.o)
SERVER_EXECUTABLE=DataBase_Server

CLIENT_SOURCES=include/DataBase.cpp include/Fields.cpp client/DataBaseClient.cpp client/main.cpp
CLIENT_OBJECTS=$(CLIENT_SOURCES:.cpp=.o)
CLIENT_EXECUTABLE=DataBase_Client


server: $(SERVER_SOURCES) $(SERVER_EXECUTABLE)
    
$(SERVER_EXECUTABLE): $(SERVER_OBJECTS) $(COMMON_OBJECTS)
	$(CC) $(LDFLAGS) $(SERVER_OBJECTS) $(COMMON_OBJECTS) -o $@
client: $(CLIENT_SOURCES) $(CLIENT_EXECUTABLE)

$(CLIENT_EXECUTABLE): $(CLIENT_OBJECTS) $(COMMON_OBJECTS)
	$(CC) $(LDFLAGS) $(CLIENT_OBJECTS) $(COMMON_OBJECTS) -o $@


.cpp.o:
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -rf server/*.o $(SERVER_EXECUTABLE)
	rm -rf client/*.o $(CLIENT_EXECUTABLE)
	rm -rf include/*.o