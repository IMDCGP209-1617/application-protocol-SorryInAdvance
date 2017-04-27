#include <iostream>
#include <WinSock2.h>
#include <vector>
#include <thread>
#include <string>
#include <stdlib.h>

#pragma comment(lib, "Ws2_32.lib")

struct client_type
{
	int id;
	SOCKET socket;
	std::string username;
	std::string currentRoom;
};

struct room_type
{
	std::string roomname;
	std::string tag;
	bool isPrivate;
	int roomSize;
};
const int MAX_CLIENTS = 5;
const int MAX_ROOMS = 25;
const int DEFAULT_BUFLEN = 512;

std::vector<room_type> rooms;
std::vector<client_type> clients(MAX_CLIENTS);
std::thread threads[MAX_CLIENTS];

room_type room;

const std::vector<std::string> explode(const std::string& s, const char& c) {
	std::string buff{ "" };
	std::vector<std::string> v;
	for (auto n : s) {
		if (n != c)
		{
			buff += n;
		}
		else if (n == c && buff != "")
		{
			v.push_back(buff); 
			buff = "";
		}
	}

	if (buff != "") 
		v.push_back(buff);

	return v;

}
int processClient(client_type &new_client, std::vector<client_type> &client_array,
	std::thread &thread, std::vector<room_type> &current_room, room_type &room)
{
	std::string msg = "";
	char tempmsg[DEFAULT_BUFLEN];
	while (1)
	{
		memset(tempmsg, 0, DEFAULT_BUFLEN);
		int result = recv(new_client.socket, tempmsg, DEFAULT_BUFLEN, 0);
		if (result != SOCKET_ERROR)
		{
			if (strcmp("\r\n", tempmsg) == 0)
				continue;

			if (strcmp("", tempmsg)) {
				msg = new_client.username + " #" + std::to_string(new_client.id) + ": " + tempmsg + "\r\n";
			}
			std::vector<std::string> sMessage{ explode(tempmsg, ' ') };

			std::string command;

			if (sMessage.size() == 0)
				command = tempmsg;
			else
				command = sMessage[0];

			if (command[0] == '/') {
				if (strcmp("/command", command.c_str()) == 0)
				{
					if (sMessage.size() > 1)
					{
						if (strcmp("1", sMessage[1].c_str()) == 0) {
							msg = "<Page 1 of x> \r\n/user <username> \r\n/create <room name> <privacy> \r\n/close <room name> \r\n/join <room name>";
						}
					}
					else {
						msg = "ERR try /command <page number>";
					}
					result = send(client_array[new_client.id].socket,
						msg.c_str(),
						strlen(msg.c_str()), 0);
				}

				else if (strcmp("/user", command.c_str()) == 0)
				{
					if (sMessage.size() > 1)
					{
						new_client.username = sMessage[1];
						std::cout << "changing username to: " << sMessage[1] << std::endl;
					}
					else
					{
						std::cout << "no username specified" << std::endl;
					}
				}
				/*
				else if (strcmp("/setpass", command.c_str()) == 0)
				{
					std::cout << "setpass";
				}

				else if (strcmp("/login", command.c_str()) == 0)
				{
					std::cout << "login";
				}

				else if (strcmp("/logout", command.c_str()) == 0)
				{
					std::cout << "logout";
				}
				*/
				else if (strcmp("/openrooms", command.c_str()) == 0)
				{
					if (sMessage.size() < 2) {
						std::cout << "Open Rooms";
						std::string returnMsg = "Open Rooms: ";
						for (int i = 0; i < rooms.size(); i++)
						{
							if (rooms[i].isPrivate == false) {
								
								returnMsg = returnMsg + rooms[i].roomname + ", ";
							}
						}
						returnMsg = returnMsg + "\r\n";

						result = send(client_array[new_client.id].socket,
							returnMsg.c_str(),
							strlen(returnMsg.c_str()), 0);
						
						}
					else {
						std::cout << "no username specified" << std::endl;
					}
					
					}

					else if (strcmp("/search", command.c_str()) == 0)
					{
						if (sMessage.size() > 1) {
							std::string returnMsg = "Matching Rooms: ";
							for (int i = 0; i < rooms.size(); i++)
							{
								if (strcmp(sMessage[1].c_str(), rooms[i].roomname.c_str) == 0) {
									returnMsg = returnMsg + rooms[i].roomname + ", ";
								}			
							
							}
							returnMsg = returnMsg + "\r\n";
							result = send(client_array[new_client.id].socket,
								returnMsg.c_str(),
								strlen(returnMsg.c_str()), 0);
						}
							
					}

					else if (strcmp("/create", command.c_str()) == 0)
					{
						if (sMessage.size() > 1) {


							room.roomname = sMessage[1];
							if (strcmp("true", sMessage[2].c_str()) == 0 || strcmp("True", sMessage[2].c_str()) == 0) {
								room.isPrivate = true;

							}
							else if (strcmp("false", sMessage[2].c_str()) == 0 || strcmp("False", sMessage[2].c_str()) == 0) {
								room.isPrivate = false;
							}

							rooms.push_back(room);
						}
						else
						{
							std::cout << "ERR Incorrect Syntax" << std::endl;
						}
					}

					else if (strcmp("/close", command.c_str()) == 0)
					{
						std::cout << "close";
					}

					else if (strcmp("/join", command.c_str()) == 0)
					{
						std::cout << "join";
					}

					else if (strcmp("/tag", command.c_str()) == 0)
					{
						std::cout << "tag";
					}

					else if (strcmp("/untag", command.c_str()) == 0)
					{
						std::cout << "untag";
					}

					else if (strcmp("/leave", command.c_str()) == 0)
					{
						std::cout << "leave";
					}

					else if (strcmp("/promote", command.c_str()) == 0)
					{
						std::cout << "promote";
					}

					else if (strcmp("/demote", command.c_str()) == 0)
					{
						std::cout << "demote";
					}

					else if (strcmp("/kick", command.c_str()) == 0)
					{
						std::cout << "kick";
					}

					else if (strcmp("/ban", command.c_str()) == 0)
					{
						std::cout << "ban";
					}

					else if (strcmp("/set", command.c_str()) == 0)
					{
						std::cout << "set";
					}

					else if (strcmp("/invite", command.c_str()) == 0)
					{
						std::cout << "invite";
					}

					else if (strcmp("/accept", command.c_str()) == 0)
					{
						std::cout << "accept";
					}

					else if (strcmp("/pm", command.c_str()) == 0)
					{
						std::cout << "pm";
					}

					else if (strcmp("/whosin", command.c_str()) == 0)
					{
						std::cout << "whosin";
					}

					else if (strcmp("/bold/", command.c_str()) == 0)
					{
						std::cout << "bold";
					}

					else if (strcmp("/italic/", command.c_str()) == 0)
					{
						std::cout << "italic";
					}

					else if (strcmp("/underline/", command.c_str()) == 0)
					{
						std::cout << "underline";
					}

					else if (strcmp("/c", command.c_str()) == 0)
					{
						std::cout << "c";
					}
					else
					{
						std::cout << "Unknown Command";
					}

				}

				//this is where you intersept the messages!!!!


				for (int i = 0; i < MAX_CLIENTS; i++)
				{
					if (client_array[i].socket != INVALID_SOCKET)
					{
						if (new_client.id != i)
						{
							result = send(client_array[i].socket,
								msg.c_str(),
								strlen(msg.c_str()), 0);
						}

						//result = send(client_array[new_client].socket,
						//	msg.c_str(),
						//	strlen(msg.c_str()), 0);
					}
				}
			}
			else
			{
				msg = new_client.username + "# " + std::to_string(new_client.id) + " disconnected";
				std::cout << msg << std::endl;

				closesocket(new_client.socket);
				closesocket(client_array[new_client.id].socket);
				client_array[new_client.id].socket = INVALID_SOCKET;

				for (int i = 0; i < MAX_CLIENTS; i++)
				{
					if (client_array[i].socket != INVALID_SOCKET)
					{
						if (new_client.id != i)
						{
							result = send(client_array[i].socket,
								msg.c_str(),
								strlen(msg.c_str()), 0);
						}
					}
				}
				break;
			}

		}
		thread.detach();
		return 0;
	}

int main() {

	WORD wVersionRequested = MAKEWORD(2, 2);
	WSADATA wsaData;
	printf("Initializing WinSock...");
	int error = WSAStartup(wVersionRequested, &wsaData);




	if (error != 0)
	{
		printf("Failed initializing winsock. Error code: %d\n", WSAGetLastError());
		return 0;
	}



	SOCKET sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sock == INVALID_SOCKET)
	{
		printf("Could not create socket : %d\n", WSAGetLastError());
		return 0;
	}

	const unsigned short PORT = 1234;

	struct sockaddr_in sa, client;
	sa.sin_family = AF_INET;
	sa.sin_addr.s_addr = ADDR_ANY;
	sa.sin_port = htons(PORT);

	error = bind(sock, (struct sockaddr*)&sa, sizeof(sa));
	if (error == SOCKET_ERROR)
	{
		printf("Bind failed with error code: %d", WSAGetLastError());
		return 0;
	}
	
	if (listen(sock, 256) == SOCKET_ERROR)
	{
		printf("Failed to set socket to listen with error code: %d", WSAGetLastError());
		return 0;
	}

	int num_clients = 0;
	int temp_id = -1;

	for (int i = 0; i < MAX_CLIENTS; i++)
	{
		clients[i] = { -1, INVALID_SOCKET };
	}

	int size = sizeof(client);
	std::string msg;
	while (1)
	{
		SOCKET clientSock = accept(sock, (sockaddr *)&client, &size);
		if (clientSock == INVALID_SOCKET) continue;

		num_clients = -1;
		temp_id = -1;
		for (int i = 0; i < MAX_CLIENTS; i++)
		{
			if (clients[i].socket == INVALID_SOCKET && temp_id == -1)
			{
				clients[i].socket = clientSock;
				clients[i].id = i;
				temp_id = i;
			}
			if (clients[i].socket != INVALID_SOCKET)
				num_clients++;
		}
		if (temp_id != -1)
		{
			std::cout << "Client #" << clients[temp_id].id << " accepted." << std::endl;
			msg = std::to_string(clients[temp_id].id);
			send(clients[temp_id].socket, msg.c_str(), strlen(msg.c_str()), 0);
			threads[temp_id] = std::thread(processClient,
				std::ref(clients[temp_id]),
				std::ref(clients),
				std::ref(threads[temp_id]),
				rooms,room);
		}
		else
		{
			msg = "Server is full";
			send(clientSock, msg.c_str(), strlen(msg.c_str()), 0);
			std::cout << msg << std::endl;
		}
	}
	closesocket(sock);
	for (int i = 0; i < MAX_CLIENTS; i++)
	{
		threads[i].detach();
		closesocket(clients[i].socket);
	}
	WSACleanup();
	return 0;

}
