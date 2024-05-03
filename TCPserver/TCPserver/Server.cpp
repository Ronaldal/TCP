#include "Header.h"


void main()
{
	struct SocketState sockets[MAX_SOCKETS] = { 0 };
	int socketsCount = 0;
	// Initialize Winsock (Windows Sockets).

	// Create a WSADATA object called wsaData.
	// The WSADATA structure contains information about the Windows 
	// Sockets implementation.
	WSAData wsaData;

	// Call WSAStartup and return its value as an integer and check for errors.
	// The WSAStartup function initiates the use of WS2_32.DLL by a process.
	// First parameter is the version number 2.2.
	// The WSACleanup function destructs the use of WS2_32.DLL by a process.
	if (NO_ERROR != WSAStartup(MAKEWORD(2, 2), &wsaData))
	{
		cout << "Time Server: Error at WSAStartup()\n";
		return;
	}

	// Server side:
	// Create and bind a socket to an internet address.
	// Listen through the socket for incoming connections.

	// After initialization, a SOCKET object is ready to be instantiated.

	// Create a SOCKET object called listenSocket. 
	// For this application:	use the Internet address family (AF_INET), 
	//							streaming sockets (SOCK_STREAM), 
	//							and the TCP/IP protocol (IPPROTO_TCP).
	SOCKET listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	// Check for errors to ensure that the socket is a valid socket.
	// Error detection is a key part of successful networking code. 
	// If the socket call fails, it returns INVALID_SOCKET. 
	// The if statement in the previous code is used to catch any errors that
	// may have occurred while creating the socket. WSAGetLastError returns an 
	// error number associated with the last error that occurred.
	if (INVALID_SOCKET == listenSocket)
	{
		cout << "Time Server: Error at socket(): " << WSAGetLastError() << endl;
		WSACleanup();
		return;
	}

	// For a server to communicate on a network, it must bind the socket to 
	// a network address.

	// Need to assemble the required data for connection in sockaddr structure.

	// Create a sockaddr_in object called serverService. 
	sockaddr_in serverService;
	// Address family (must be AF_INET - Internet address family).
	serverService.sin_family = AF_INET;
	// IP address. The sin_addr is a union (s_addr is a unsigned long 
	// (4 bytes) data type).
	// inet_addr (Iternet address) is used to convert a string (char *) 
	// into unsigned long.
	// The IP address is INADDR_ANY to accept connections on all interfaces.
	serverService.sin_addr.s_addr = INADDR_ANY;
	// IP Port. The htons (host to network - short) function converts an
	// unsigned short from host to TCP/IP network byte order 
	// (which is big-endian).
	serverService.sin_port = htons(TIME_PORT);

	// Bind the socket for client's requests.

	// The bind function establishes a connection to a specified socket.
	// The function uses the socket handler, the sockaddr structure (which
	// defines properties of the desired connection) and the length of the
	// sockaddr structure (in bytes).
	if (SOCKET_ERROR == bind(listenSocket, (SOCKADDR*)&serverService, sizeof(serverService)))
	{
		cout << "Time Server: Error at bind(): " << WSAGetLastError() << endl;
		closesocket(listenSocket);
		WSACleanup();
		return;
	}

	// Listen on the Socket for incoming connections.
	// This socket accepts only one connection (no pending connections 
	// from other clients). This sets the backlog parameter.
	if (SOCKET_ERROR == listen(listenSocket, 5))
	{
		cout << "Time Server: Error at listen(): " << WSAGetLastError() << endl;
		closesocket(listenSocket);
		WSACleanup();
		return;
	}
	addSocket(listenSocket, LISTEN ,sockets,socketsCount);

	// Accept connections and handles them one by one.
	cout << "Wating for clients..." << endl;
	while (true)
	{
		// The select function determines the status of one or more sockets,
		// waiting if necessary, to perform asynchronous I/O. Use fd_sets for
		// sets of handles for reading, writing and exceptions. select gets "timeout" for waiting
		// and still performing other operations (Use NULL for blocking). Finally,
		// select returns the number of descriptors which are ready for use (use FD_ISSET
		// macro to check which descriptor in each set is ready to be used).
		
		updateSocketsAccordingToTimeout(sockets, socketsCount);
		fd_set waitRecv;
		FD_ZERO(&waitRecv);
		for (int i = 0; i < MAX_SOCKETS; i++)
		{
			if ((sockets[i].recv == LISTEN) || (sockets[i].recv == RECEIVE))
				FD_SET(sockets[i].id, &waitRecv);
		}

		fd_set waitSend;
		FD_ZERO(&waitSend);
		for (int i = 0; i < MAX_SOCKETS; i++)
		{
			if (sockets[i].send == SEND)
				FD_SET(sockets[i].id, &waitSend);
		}

		//
		// Wait for interesting event.
		// Note: First argument is ignored. The fourth is for exceptions.
		// And as written above the last is a timeout, hence we are blocked if nothing happens.
		//
		int nfd;
		nfd = select(0, &waitRecv, &waitSend, NULL, NULL);
		if (nfd == SOCKET_ERROR)
		{
			cout << "Time Server: Error at select(): " << WSAGetLastError() << endl;
			WSACleanup();
			return;
		}

		for (int i = 0; i < MAX_SOCKETS && nfd > 0; i++)
		{
			if (FD_ISSET(sockets[i].id, &waitRecv))
			{
				nfd--;
				switch (sockets[i].recv)
				{
				case LISTEN:
					acceptConnection(i,sockets,socketsCount);
					break;

				case RECEIVE:
					receiveMessage(i, sockets, socketsCount);
					break;
				}
			}
		}

		for (int i = 0; i < MAX_SOCKETS && nfd > 0; i++)
		{
			if (FD_ISSET(sockets[i].id, &waitSend))
			{
				nfd--;
				switch (sockets[i].send)
				{
				case SEND:
					sendMessage(i,sockets,socketsCount);
					break;
				}
			}
		}
	}

	// Closing connections and Winsock.
	cout << "Time Server: Closing Connection.\n";
	closesocket(listenSocket);
	WSACleanup();
}

bool addSocket(SOCKET id, Status what, SocketState* sockets, int& socketsCount)
{
	for (int i = 0; i < MAX_SOCKETS; i++)
	{
		if (sockets[i].recv == EMPTY)
		{
			sockets[i].id = id;
			sockets[i].recv = what;
			sockets[i].send = IDLE;
			sockets[i].len = 0;
			socketsCount++;
			sockets[i].timeOut = time(0);
			return (true);
		}
	}
	return (false);
}

void removeSocket(int index,SocketState* sockets, int& socketsCount)
{
	sockets[index].recv = EMPTY;
	sockets[index].send = EMPTY;
	sockets[index].timeOut = 0;
	socketsCount--;
}

void acceptConnection(int index, SocketState* sockets, int& socketsCount)
{
	SOCKET id = sockets[index].id;
	struct sockaddr_in from;		// Address of sending partner
	int fromLen = sizeof(from);

	SOCKET msgSocket = accept(id, (struct sockaddr*)&from, &fromLen);
	if (INVALID_SOCKET == msgSocket)
	{
		cout << "TCP Server: Error at accept(): " << WSAGetLastError() << endl;
		return;
	}
	cout << "TCP Server: Client " << inet_ntoa(from.sin_addr) << ":" << ntohs(from.sin_port) << " is connected." << endl;

	//
	// Set the socket to be in non-blocking mode.
	//
	unsigned long flag = 1;
	if (ioctlsocket(msgSocket, FIONBIO, &flag) != 0)
	{
		cout << "TCP Server: Error at ioctlsocket(): " << WSAGetLastError() << endl;
	}

	if (addSocket(msgSocket, RECEIVE,sockets,socketsCount) == false)
	{
		cout << "\t\tToo many connections, dropped!\n";
		closesocket(id);
	}
	return;
}

void receiveMessage(int index, SocketState* sockets, int& socketsCount)
{
	SOCKET msgSocket = sockets[index].id;

	int len = sockets[index].len;
	int bytesRecv = recv(msgSocket, &sockets[index].buffer[len], sizeof(sockets[index].buffer) - len, 0);

	if (SOCKET_ERROR == bytesRecv)
	{
		cout << "TCP Server: Error at recv(): " << WSAGetLastError() << endl;
		closesocket(msgSocket);
		removeSocket(index,sockets,socketsCount);
		return;
	}
	else if (bytesRecv == 0)
	{
		closesocket(msgSocket);
		removeSocket(index,sockets,socketsCount);
		return;
	}
	else
	{
		sockets[index].buffer[len + bytesRecv] = '\0'; //add the null-terminating to make it a string
		cout << "TCP Server: Recieved: " << bytesRecv << " bytes of \"" << &sockets[index].buffer[len] << "\" message.\n";

		sockets[index].len += bytesRecv;

		if (sockets[index].len > 0)
			fillSocket(sockets[index]);
		
	}

}
void fillSocket(SocketState& socket)
{
	string buffer = (string)socket.buffer;
	string reqType = buffer.substr(0, buffer.find(" "));
	socket.send = SEND;
	socket.requestType = stringToReqType[reqType];
	socket.len -= (reqType.length() + 2);
	memcpy(socket.buffer, &socket.buffer[reqType.length() + 2], socket.len);
	socket.buffer[socket.len] = '\0';
	return;
}


void sendMessage(int index, SocketState* sockets, int& socketsCount) {
	SOCKET msgSocket = sockets[index].id;

	string reqStatus, body, headers, response;
	buildResponse(sockets[index], reqStatus, body, headers, response);

	if (reqStatus == NOT_FOUND) {
		body = insertResourceToBuffer(ERROR_DIR);
	}

	sendResponse(msgSocket, response, index, sockets, socketsCount);
}
void sendResponse(SOCKET msgSocket, string& response, int index, SocketState* sockets, int& socketsCount) 
{
	char sendBuff[BUFFER_LEN];
	sprintf(sendBuff, response.c_str());
	int bytesSent = send(msgSocket, sendBuff, (int)strlen(sendBuff), 0);
	
	if (SOCKET_ERROR == bytesSent) {
		cout << "TCP Server: Error at send(): " << WSAGetLastError() << endl;
		return;
	}

	cout << "TCP Server: Sent: " << bytesSent << " bytes of \"" << response << "\" message.\n";

	// Clear buffers and update socket state
	memset(sockets[index].buffer, 0, BUFFER_LEN);
	sockets[index].len = 0;
	sockets[index].send = IDLE;
}


void buildResponse(SocketState& socket,string& reqStatus, string& body,string& headers ,string& response) {
	switch (socket.requestType) {
	case GET:
		getRequest(socket, reqStatus, body, headers);
		break;
	case POST:
		postRequest(socket, reqStatus, body, headers);
		break;
	case PUT:
		putRequest(socket, reqStatus, body, headers);
		break;
	case DEL:
		deleteRequest(socket, reqStatus);
		break;
	case TRACE:
		traceRequest(socket, reqStatus, body, headers);
		break;
	case HEAD:
		getRequest(socket, reqStatus, body, headers);
		body = "";
		break;
	case OPTIONS:
		optionsRequest(socket, reqStatus, headers);
		break;
	default:
		reqStatus = NOT_FOUND;
		break;
	}

	response.append(reqStatus);
	response.append(headers);
	response.append(END_HEADER);
	response.append(body);
}
void getRequest(SocketState& socket, string& reqStatus, string& answer, string& header)
{
	string buffer = (string)socket.buffer;
	string lang = getLanguage(buffer);
	time_t currentTime;
	time(&currentTime);

	if (lang != "")
	{
		if (lang != "en" && lang != "fr" && lang != "he")
		{
			reqStatus = NOT_FOUND;
			return;
		}

		if (buffer.substr(0, buffer.find(".html?lang")) != RESOURCE_NAME)
		{
			reqStatus = NOT_FOUND;
			return;
		}
	}
	else if (lang == "not_found")
	{
		reqStatus = NOT_FOUND;
		return;
	}
	else
	{
		if (buffer.substr(0, buffer.find(".html")) != RESOURCE_NAME)
		{
			reqStatus = NOT_FOUND;
			return;
		}

		lang = "en";
	}

	string htmlResource;
	htmlResource.append(ROOT_DIR);
	htmlResource.append(RESOURCE_NAME);
	htmlResource.append(lang);
	htmlResource.append(".html");

	answer = insertResourceToBuffer(htmlResource);

	if (answer == "") {
		reqStatus = INTERNAL_SERVER_ERROR;
	}
	else {
		reqStatus = STATUS_OK;
	}

	header.append("Date: ");
	header.append(ctime(&currentTime));
	header.append(CONTENT_TYPE_TEXT_HTML);
	header.append(CONTENT_LEN);
	header.append(to_string(answer.length()));
}
void putRequest(SocketState& socket, string& reqStatus, string& body, string& header)
{
	time_t currentTime;
	time(&currentTime);
	size_t x;
	string reqBody,resource = ROOT_DIR,buffer = socket.buffer;

	x = buffer.find("\r\n\r\n");
	reqBody = &buffer[x + 4];
	resource.append(strtok(socket.buffer, " "));
	reqStatus = NOT_IMPLEMENTED;
	const char* fileName = resource.c_str();
	if (strlen(fileName) != 0) {
		FILE* file = fopen(fileName, "r");
		if (reqBody.length() != 0)
		{
			if (file)
			{
				fclose(file);
				file = fopen(fileName, "w");
				fwrite(reqBody.c_str(), strlen(reqBody.c_str()), 1, file);
				fclose(file);
				reqStatus = STATUS_OK;
			}
			else
			{
				file = fopen(fileName, "w");
				fwrite(reqBody.c_str(), strlen(reqBody.c_str()), 1, file);
				fclose(file);
				reqStatus = FILE_CREATED;
			}
		}
		else
		{
			file = fopen(fileName, "w");
			fwrite(reqBody.c_str(), strlen(reqBody.c_str()), 1, file);
			fclose(file);
			reqStatus = STATUS_OK_NO_CONTENT;
		}
		body = "New file created in path :" + resource;
	}
	else 
	{
		reqStatus = BAD_REQUEST;
		body = "Bad request";
	}
	
	header.append("Date: ");
	header.append(ctime(&currentTime));
	header.append(CONTENT_LEN);
	header.append(to_string(body.length()));
}
void postRequest(SocketState& socket, string& reqStatus, string& body, string& header)
{
	time_t currentTime;
	time(&currentTime);
	size_t x;
	string reqBody;
	string buffer = socket.buffer;
	x = buffer.find("\r\n\r\n");
	reqBody = &buffer[x + 4];

	FILE* outputFile = fopen(OUTPUT_FILE_PATH.c_str(), "a+");
	if (outputFile != NULL) {
		fprintf(outputFile, "%s\n", reqBody.c_str());
		fclose(outputFile);
		reqStatus = STATUS_OK;
		body = "\""+ reqBody+"\"" + " , is written to :" + OUTPUT_FILE_PATH;
	}
	else {
		reqStatus = INTERNAL_SERVER_ERROR;
	}

	header.append("Date: ");
	header.append(ctime(&currentTime));
	header.append(CONTENT_LEN);
	header.append(to_string(body.length()));
	cout << socket.buffer << endl << endl;
}
void deleteRequest(SocketState& socket, string& reqStatus)
{
	string resource = ROOT_DIR;
	resource.append(strtok(socket.buffer, " "));

	FILE* file = fopen(resource.c_str(), "r");
	if (file)
	{
		fclose(file);
		reqStatus = remove(resource.c_str()) == 0 ? STATUS_OK_NO_CONTENT : INTERNAL_SERVER_ERROR;
	}
	else
	{
		reqStatus = NOT_FOUND;
	}

}

void traceRequest(SocketState& socket, string& reqStatus, string& body, string& header)
{
	time_t currentTime;
	time(&currentTime);
	body = socket.buffer;
	reqStatus = STATUS_OK;
	header.append("Content-Type: message/http\r\n");
	header.append("Date: ");
	header.append(ctime(&currentTime));
	header.append(CONTENT_LEN);
	header.append(to_string(body.length()));
}

void optionsRequest(SocketState& socket, string& reqStatus, string& header)
{
	reqStatus = STATUS_OK_NO_CONTENT;
	header.append("Allow: GET, HEAD, POST, PUT, DELETE, TRACE, OPTIONS");
}
string getLanguage(string url)
{
	string lang;
	int pos = url.find("lang=");
	if (pos != -1)
	{
		string lang = url.substr(strlen("lang=") + pos, 3);
		if (lang.at(2) == '&' || lang.at(2) == ' ')
		{
			return url.substr(strlen("lang=") + pos, 2);
		}
		return "not_found";
	}
	return "";
}



string insertResourceToBuffer(string resourceName)
{
	ifstream file;
	file.open(resourceName);
	string content;
	char buffer[BUFFER_LEN];
	if (file)
	{
		while (file.getline(buffer, BUFFER_LEN))
		{
			content += buffer;
		}
	}
	file.close();
	return content;
}

void updateSocketsAccordingToTimeout(SocketState* sockets, int& socketsCount)
{
	time_t currentTime;
	for (int i = 1; i < MAX_SOCKETS; i++)
	{
		currentTime = time(0);
		if ((currentTime - sockets[i].timeOut > 120) && (sockets[i].timeOut != 0))
		{
			removeSocket(i, sockets, socketsCount);
		}
	}
}