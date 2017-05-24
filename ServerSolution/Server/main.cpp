#include <iostream>
#include <WinSock2.h>
#include <vector>
#include <thread>
#include <string>
#include <stdlib.h>
#include <algorithm>
#include <utility>
#pragma comment(lib, "Ws2_32.lib")

struct client_type
{
	int id;
	SOCKET socket;
	std::string username;
	std::string currentRoom;

	bool usedInvite;
	std::string roomInvite;
};

struct room_type
{
	std::string roomname;
	std::string tag;
	bool isPrivate;
	int roomSize;
	std::vector<std::string> promotedUsers;
	std::vector<std::string> bannedUsers;
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
	//splits a string into individual parts by spaces 
}
int processClient(client_type &new_client, std::vector<client_type> &client_array,
	std::thread &thread, std::vector<room_type> &current_room, room_type &room)
{
	bool privMsg = false;
	std::string msg = "";
	char tempmsg[DEFAULT_BUFLEN];
	while (1)
	{
		memset(tempmsg, 0, DEFAULT_BUFLEN);
		int result = recv(new_client.socket, tempmsg, DEFAULT_BUFLEN, 0);
		if (result != SOCKET_ERROR)
		{
			if (strcmp("\r\n", tempmsg) == 0)//if the message is empty, ignore it
				continue;

			if (strcmp("", tempmsg)) {
				msg = new_client.username + " #" + std::to_string(new_client.id) + ": " + tempmsg + "\r\n";
			}
			std::vector<std::string> sMessage{ explode(tempmsg, ' ') };

			std::string command;

			std::cout << msg << std::endl;

			if (sMessage.size() == 0)
				command = tempmsg;
			else
				command = sMessage[0];

			if (command[0] == '/') {//checks for a / so that the program knows the entered string is a command
				if (strcmp("/command", command.c_str()) == 0)// /command <page number>, shows different commands and the formatting of them 
				{
					if (sMessage.size() > 1)
					{
						if (strcmp("1", sMessage[1].c_str()) == 0) {//shows page 1
							msg = "<Page 1 of 4> \r\n/user <username> \r\n/create <room name> <privacy> \r\n/close <room name> \r\n/join <room name>";

						}
						else if (strcmp("2", sMessage[1].c_str()) == 0) {//shows page 2
							msg = "<Page 2 of 4> \r\n/openrooms \r\n/search <room name> || /search <tag> \r\n/tag <room name> <tag> \r\n/untag <room name> <tag>";

						}
						else if (strcmp("3", sMessage[1].c_str()) == 0) {//shows page 3
							msg = "<Page 3 of 4> \r\n/leave \r\n/promote <username> <room name> \r\n/demote <username> <room name> \r\n/kick <username>";

						}
						else if (strcmp("4", sMessage[1].c_str()) == 0) {//shows page 4
							msg = "<Page 4 of 4> \r\n/ban <username> <room name>\r\n/invite <username> <room name> \r\n/accept \r\n/pm <username> <message> \r\n/whosin <roomname>";

						}
						result = send(client_array[new_client.id].socket,
							msg.c_str(),
							strlen(msg.c_str()), 0);
						continue;
					}
					else {//error if command entered wrong
						msg = "ERR try /command <page number(1-4)>";
					}

				}

				else if (strcmp("/user", command.c_str()) == 0)// /user <username>, assigns a username to the client
				{
					if (sMessage.size() > 1)
					{
						new_client.username = sMessage[1];
						std::cout << "changing username to: " << sMessage[1] << std::endl;//changes name
					}
					else
					{
						std::cout << "no username specified" << std::endl;//error if no username is entered
					}
					continue;
				}
				else if (strcmp("/openrooms", command.c_str()) == 0)// /openrooms, shows user all the rooms that are not private
				{
					if (sMessage.size() < 2) {
						std::cout << "Open Rooms";
						std::string returnMsg = "Open Rooms: ";
						for (int i = 0; i < rooms.size(); i++)
						{
							if (rooms[i].isPrivate == false) {

								returnMsg = returnMsg + rooms[i].roomname + ", ";//adds matching rooms to the output of the command 
							}
						}
						returnMsg = returnMsg + "\r\n";

						result = send(client_array[new_client.id].socket,
							returnMsg.c_str(),
							strlen(returnMsg.c_str()), 0);

					}
					else {//errors if the user mis-types command
						std::cout << "ERR try /openrooms" << std::endl;
					}
					continue;
				}

				else if (strcmp("/search", command.c_str()) == 0)// /search <room name> || /search <tag>, used to search for a room by name or by the tag given to it
				{
					if (sMessage.size() > 1) {
						std::string returnMsg = "Matching Rooms: ";
						for (int i = 0; i < rooms.size(); i++)
						{
							if (strcmp(sMessage[1].c_str(), rooms[i].roomname.c_str()) == 0 || strcmp(sMessage[1].c_str(), rooms[i].tag.c_str()) == 0 && rooms[i].isPrivate == false) {//makes checks to see if room or tag exists and is not private
								returnMsg = returnMsg + rooms[i].roomname + ", ";//adds matching rooms to the output of the command 
							}

						}
						returnMsg = returnMsg + "\r\n";
						result = send(client_array[new_client.id].socket,
							returnMsg.c_str(),
							strlen(returnMsg.c_str()), 0);

					}
					continue;
				}

				else if (strcmp("/create", command.c_str()) == 0) // / create <room name> <privacy>, creates a room that is either 
				{
					if (sMessage.size() > 2) {


						room.roomname = sMessage[1];
						new_client.currentRoom = room.roomname;//puts user in the room they created

						if (strcmp("true", sMessage[2].c_str()) == 0 || strcmp("True", sMessage[2].c_str()) == 0) {//sets privacy to true
							room.isPrivate = true;

						}
						else if (strcmp("false", sMessage[2].c_str()) == 0 || strcmp("False", sMessage[2].c_str()) == 0) {//sets privacy to false
							room.isPrivate = false;
						}
						msg = "Created Room: " + room.roomname;
						result = send(client_array[new_client.id].socket,
							msg.c_str(),
							strlen(msg.c_str()), 0);
						rooms.push_back(room);
						for (int i = 0; i < rooms.size(); i++)
						{
							if (strcmp(sMessage[1].c_str(), rooms[i].roomname.c_str()) == 0) {
								rooms[i].promotedUsers.push_back(new_client.username);//instantly promotes user who made the room
							}
						}
					}
					else
					{
						std::cout << "ERR Incorrect Syntax" << std::endl;
					}
					continue;
				}

				else if (strcmp("/close", command.c_str()) == 0)// /close <room name>, takes everyone in the room out of it.
				{
					for (int i = 0; i < MAX_CLIENTS; i++)
					{
						if (client_array[i].socket != INVALID_SOCKET)
						{
							if (client_array[i].currentRoom == sMessage[1]) {//searches through all users to find ones in the room
					
								client_array[i].currentRoom = "Global";//sets the room back to the global room

								msg = "Room " + room.roomname + " has been closed";
								result = send(client_array[new_client.id].socket,
									msg.c_str(),
									strlen(msg.c_str()), 0);

								continue;
							}
						}
					}
				}

				else if (strcmp("/join", command.c_str()) == 0)// / join <room name>, used to join a room
				{
					std::string returnMsg;
					if (sMessage.size() > 1) {
						bool canjoin = true;
						for (int i = 0; i < rooms.size(); i++)
						{
							if (strcmp(sMessage[1].c_str(), rooms[i].roomname.c_str()) == 0 && rooms[i].isPrivate == false) {//makes checks to see if room exists and is not private
								for (int x = 0; x < rooms[i].bannedUsers.size(); x++)
								{
									if (rooms[i].bannedUsers[x] == new_client.username) {//checks if user that is trying to join is not banned from the room
										canjoin = false;
										continue;
									}
								}
								if (canjoin) {//user is able to join 
									new_client.currentRoom = rooms[i].roomname;
									msg = new_client.username + " has joined " + room.roomname;
									result = send(client_array[new_client.id].socket,
										msg.c_str(),
										strlen(msg.c_str()), 0);
								}
								else {//user is banned
									msg = new_client.username + " is banned in this room and can not join";
									result = send(client_array[new_client.id].socket,
										msg.c_str(),
										strlen(msg.c_str()), 0);
									continue;
								}
								
							}
						}
					}
				}

				else if (strcmp("/tag", command.c_str()) == 0)// /tag <room name> <tag>, tags a room making it easier to serach for
				{
					if (sMessage.size() > 2) {
						std::string returnMsg;
						for (int i = 0; i < rooms.size(); i++)
						{
							if (strcmp(sMessage[1].c_str(), rooms[i].roomname.c_str()) == 0 && rooms[i].isPrivate == false) {//makes checks to see if room exists and is not private
								rooms[i].tag = sMessage[2];
								msg = "Room Tagged as: " + sMessage[2] + "\r\n";//tags room

								result = send(client_array[new_client.id].socket,
									msg.c_str(),
									strlen(msg.c_str()), 0);
							}

						}
					}
					continue;
				}

				else if (strcmp("/untag", command.c_str()) == 0) // /untag <room name> <tag>, untags a room
				{
					if (sMessage.size() > 1) {
						for (int i = 0; i < rooms.size(); i++)
						{
							if (strcmp(sMessage[1].c_str(), rooms[i].roomname.c_str()) == 0 && rooms[i].isPrivate == false) {//makes checks to see if room exists and is not private
								rooms[i].tag = "";//removes tag
							}
						}
					}
				}

				else if (strcmp("/leave", command.c_str()) == 0) // /leave, leaves current room back to the global room
				{
					if (new_client.currentRoom != "Global") {//makes sure user is not in Global
						new_client.currentRoom = "Global";
						continue;
					}
					else {
						msg = "ERR Can't leave Global";

					}
					result = send(client_array[new_client.id].socket,
						msg.c_str(),
						strlen(msg.c_str()), 0);
				}

				else if (strcmp("/promote", command.c_str()) == 0) // /promote <username> <room name>, promotes a user so they can use promoted user commands
				{
					if (sMessage.size() > 2) {
						for (int h = 0; h < MAX_CLIENTS; h++)
						{
							bool foundClient = false;
							if (client_array[h].socket != INVALID_SOCKET)
							{
								if (client_array[h].username == sMessage[1]) {//checks username against list of users
									for (int i = 0; i < rooms.size(); i++)
									{
										if (strcmp(sMessage[2].c_str(), rooms[i].roomname.c_str()) == 0) {
											bool alreadyPromoted = false;
											for (int j = 0; j < rooms[i].promotedUsers.size(); j++)
											{
												if (new_client.username == rooms[i].promotedUsers[j]) {//checks that the user using the command has is promoted
													for (int k = 0; k < rooms[i].promotedUsers.size(); k++)
													{
														if (strcmp(rooms[i].promotedUsers[k].c_str(), client_array[h].username.c_str()) == 0) {
															alreadyPromoted = true;
															break;
														}
													}
														if (new_client.currentRoom == client_array[h].currentRoom && new_client.currentRoom == rooms[i].roomname) {//makes sure that both users are in the room
															if (!alreadyPromoted) {
																msg = client_array[h].username + " Has been promoted in room " + rooms[i].roomname;
																rooms[i].promotedUsers.push_back(client_array[h].username);//promotes user
															}
															else {
																msg = client_array[h].username + " Has already been promoted in room " + rooms[i].roomname;
															}
															foundClient = true;
															break;
														}
														else {//error shown when user is not in the same room
															msg = client_array[h].username + " is not in this room";
															break;
														}
													
												}
												else {//error that is displayed when a non-promoted user uses the command
													msg = new_client.username + " is not a promoted user, so cannot use /promote";
												}
												break;
											}
										}
									}
								}
							}

							if (foundClient) break;
						}
						result = send(client_array[new_client.id].socket,
							msg.c_str(),
							strlen(msg.c_str()), 0);
					}
				}

				else if (strcmp("/demote", command.c_str()) == 0) // /promote <username> <room name>, demotes a user so they cannot use promoted user commands
				{
					if (sMessage.size() > 2) {
						for (int h = 0; h < MAX_CLIENTS; h++)
						{
							bool foundClient = false;
							if (client_array[h].socket != INVALID_SOCKET)
							{
								if (client_array[h].username == sMessage[1]) {//checks username against list of users
									for (int i = 0; i < rooms.size(); i++)
									{
										if (strcmp(sMessage[2].c_str(), rooms[i].roomname.c_str()) == 0) {
											bool alreadyPromoted = false;
											for (int j = 0; j < rooms[i].promotedUsers.size(); j++)
											{
												if (new_client.username == rooms[i].promotedUsers[j]) {//checks that the user using the command has is promoted
													for (int k = 0; k < rooms[i].promotedUsers.size(); k++)
													{
														if (strcmp(rooms[i].promotedUsers[k].c_str(), client_array[h].username.c_str()) == 0) {
															alreadyPromoted = true;
															break;
														}
													}
													if (new_client.currentRoom == client_array[h].currentRoom && new_client.currentRoom == rooms[i].roomname) {//makes sure that both users are in the room
														if (alreadyPromoted) {
															msg = client_array[h].username + " Has been demoted in room " + rooms[i].roomname;//demotes user
															auto itr = std::find(rooms[i].promotedUsers.begin(), rooms[i].promotedUsers.end(), client_array[h].username);

															if (itr != rooms[i].promotedUsers.end()) {
																std::swap(*itr, rooms[i].promotedUsers.back());//removes them from the list of promoted users
																rooms[i].promotedUsers.pop_back();
															}
														}
														else {
															msg = client_array[h].username + " Has already been demoted in room " + rooms[i].roomname;//if they have already been demoted
														}
														foundClient = true;
														break;
													}
													else {
														msg = client_array[h].username + " is not in this room";//error shown when user is not in the same room
														break;
													}

												}
												else {
													msg = new_client.username + " is not a promoted user, so cannot use /demote";//error that is displayed when a non-promoted user uses the command
												}
											}
										}
									}
								}
							}

							if (foundClient) break;
						}
						result = send(client_array[new_client.id].socket,
							msg.c_str(),
							strlen(msg.c_str()), 0);
					}
					
				}

				else if (strcmp("/kick", command.c_str()) == 0) // /kick <username>, kicks user from a room
				{
					if (sMessage.size() > 1) {
						for (int h = 0; h < MAX_CLIENTS; h++)
						{
							if (client_array[h].socket != INVALID_SOCKET)
							{
								if (client_array[h].username == sMessage[1]) {//checks username against list of users
									for (int i = 0; i < rooms.size(); i++)
									{
										bool kicked = false;
										if (new_client.currentRoom == client_array[h].currentRoom && strcmp(new_client.currentRoom.c_str(), rooms[i].roomname.c_str()) == 0) {
											kicked = true;
											for (int j = 0; j < rooms[i].promotedUsers.size(); j++)
											{
												
												if (new_client.username == rooms[i].promotedUsers[j]) {//checks that the user using the command has is promoted
													
													client_array[h].currentRoom = "Global";//sets kicked users room to Global
													msg = client_array[h].username + " has been kicked from the room: " + rooms[i].roomname;

												}
												else {
													msg = new_client.username + " is not a promoted user, so cannot use /kick";//error that is displayed when a non-promoted user uses the command

												}
												result = send(client_array[new_client.id].socket,
													msg.c_str(),
													strlen(msg.c_str()), 0);
												break;
											}
											if (!kicked) {//error shown when user is not in the same room
												msg = client_array[h].username + " is not in this room";
												result = send(client_array[new_client.id].socket,
													msg.c_str(),
													strlen(msg.c_str()), 0);
											}
											break;
										}
									}
								}
							}
						}
					}
				}

				else if (strcmp("/ban", command.c_str()) == 0) // /ban <username> <room name>, bans users so they cannot rejoin
				{
					if (sMessage.size() > 2) {
						for (int h = 0; h < MAX_CLIENTS; h++)
						{
							if (client_array[h].socket != INVALID_SOCKET)
							{//checks username against list of users
								if (client_array[h].username == sMessage[1]) {
									for (int i = 0; i < rooms.size(); i++)
									{
										bool banned = false;
										for (int j = 0; j < rooms[i].promotedUsers.size(); j++)
										{
											//checks that the user using the command has is promoted
											if (new_client.username == rooms[i].promotedUsers[j]) {
												if (strcmp(new_client.currentRoom.c_str(), rooms[i].roomname.c_str()) == 0) {
													banned = false;
													for (int k = 0; k < rooms[i].bannedUsers.size(); k++)
													{
														if (rooms[i].bannedUsers[k] == client_array[h].username) {//checks if the user is already banned
															msg = client_array[h].username + " has already been banned from the room: " + rooms[i].roomname;
															banned = true;
															break;
														}		
													}

													if (!banned)
													{//sets banned users room to Global
														client_array[h].currentRoom = "Global";
														msg = client_array[h].username + " has been banned from the room: " + rooms[i].roomname;
														rooms[i].bannedUsers.push_back(client_array[h].username);//puts them on the banned user list for the room
													}
												

													result = send(client_array[new_client.id].socket,
														msg.c_str(),
														strlen(msg.c_str()), 0);
												}
											}
										}
									}
								}
							}
						}
					}
				}
				else if (strcmp("/invite", command.c_str()) == 0) // /invite <username> <room name>, invites a user to the current room the commands user is in
				{
					if (sMessage.size() > 2) {
						for (int h = 0; h < MAX_CLIENTS; h++)
						{
							if (client_array[h].socket != INVALID_SOCKET)
							{
								if (client_array[h].username == sMessage[1]) {//checks username against list of users
									for (int i = 0; i < rooms.size(); i++)
									{
										if (strcmp(sMessage[2].c_str(), rooms[i].roomname.c_str()) == 0 && new_client.currentRoom == rooms[i].roomname) {//makes sure room exist and the user of the command is in the room
											privMsg = true;//make the message private
											msg = new_client.username + " wants you to join the room they are in. use /accept to join them";//sends invite to user
											client_array[h].roomInvite = sMessage[2];
											client_array[h].usedInvite = false;
											result = send(client_array[client_array[h].id].socket,
												msg.c_str(),
												strlen(msg.c_str()), 0);
											break;
										}
									}
								}
							}
						}
					}
				}

				else if (strcmp("/accept", command.c_str()) == 0)// /accept, accepts an invite request
				{
					if (!new_client.usedInvite) {
						new_client.currentRoom = new_client.roomInvite;
						msg = new_client.username + " has joined " + room.roomname;
						new_client.usedInvite = true;//sets usedInvite to true so it can only be used once
					}
					else {
						msg = "You have already accepted an invite, or one wasn't sent";
						//error sent when the command was used when no acceptable invites were sent
					}
					result = send(client_array[new_client.id].socket,
						msg.c_str(),
						strlen(msg.c_str()), 0);
				}

				else if (strcmp("/pm", command.c_str()) == 0) // /pm <username> <message>, sends a private message to a user
				{
					if (sMessage.size() > 2) {
						for (int h = 0; h < MAX_CLIENTS; h++)
						{//checks username against list of users
							if (client_array[h].socket != INVALID_SOCKET)
							{
								if (client_array[h].username == sMessage[1]) {
									privMsg = true;//make the message private
									std::string hstring = new_client.username + " says to you: ";
									for (int i = 2; i < sMessage.size(); i++)
									{
										hstring = hstring + sMessage[i] + ' ';//adds message to current string
									}
									
									msg = hstring;
									result = send(client_array[client_array[h].id].socket,
										msg.c_str(),
										strlen(msg.c_str()), 0);
									break;
								}
							}
						}
					}
				}

				else if (strcmp("/whosin", command.c_str()) == 0)// /whosin <roomname>, shows who is in a room
				{
					if (sMessage.size() > 1) {
						msg = "People in this room: ";
						for (int h = 0; h < MAX_CLIENTS; h++)
						{//checks username against list of users
							if (client_array[h].socket != INVALID_SOCKET)
							{
								if (client_array[h].currentRoom == sMessage[1]) {
									msg = msg + client_array[h].username + ", ";//creates a list of users in the room
								}
							}
						}
						result = send(client_array[new_client.id].socket,
							msg.c_str(),
							strlen(msg.c_str()), 0);
					}
				}
				else
				{
					std::cout << "Unknown Command";//command not recognised
				}

			}


			if (!privMsg) {//checks for a private message being sent
				for (int i = 0; i < MAX_CLIENTS; i++)
				{
					if (client_array[i].socket != INVALID_SOCKET)
					{
						if (new_client.id != i && new_client.currentRoom == client_array[i].currentRoom)
						{
							result = send(client_array[i].socket,
								msg.c_str(),
								strlen(msg.c_str()), 0);
						}
					}
				}
			}
			else {
				privMsg = false;
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
			clients[temp_id].currentRoom = "Global";
			threads[temp_id] = std::thread(processClient,
				std::ref(clients[temp_id]),
				std::ref(clients),
				std::ref(threads[temp_id]),
				rooms, room);
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
