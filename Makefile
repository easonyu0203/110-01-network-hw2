client2: client.cpp clientSocket.h clientSocket.cpp serverSocket.h serverSocket.cpp ulti.h ulti.cpp
	g++ client.cpp clientSocket.cpp ulti.cpp serverSocket.cpp -o client2 -std=c++17 -pthread

rm:
	rm ./*.o client