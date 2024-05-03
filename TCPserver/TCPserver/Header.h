#ifndef HEADER_H
#define HEADER_H

#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <iostream>
#include <fstream>
#include <sstream>
#include <filesystem>
using namespace std;
#pragma comment(lib, "Ws2_32.lib")
#include <winsock2.h>
#include <string.h>
#include <time.h>
#include <map>

//#define BUFFER_LEN 2048
//#define MAX_SOCKETS 60

enum Status { EMPTY, LISTEN, RECEIVE, IDLE, SEND };

enum RequestType {
    GET,      // HTTP GET method
    POST,     // HTTP POST method
    PUT,      // HTTP PUT method
    DEL,      // HTTP DELETE method
    TRACE,    // HTTP TRACE method
    HEAD,     // HTTP HEAD method
    OPTIONS   // HTTP OPTIONS method
};

const int TIME_PORT = 27015;
const int SEND_TIME = 1;
const int SEND_SECONDS = 2;
const int MAX_SOCKETS = 60;
const int BUFFER_LEN = 2048;

struct SocketState
{
	SOCKET id;			
	Status	recv;
	Status	send;
	RequestType requestType;
	char buffer[BUFFER_LEN];
	int len;
    time_t timeOut;
};


map<string, RequestType> stringToReqType =
{ {"GET",GET},
	{"POST",POST},
	{"PUT",PUT},
	{"DELETE",DEL},
	{"TRACE",TRACE},
	{"HEAD",HEAD},
	{"OPTIONS",OPTIONS} };

const string ROOT_DIR = "C:\\temp\\";
const string ERROR_DIR ="C:\\temp\\error.html";
const string OUTPUT_FILE_PATH="C:\\temp\\serverOutput.txt";
const string RESOURCE_NAME = "htmlServerFile_";
const string END_LINE = "\r\n";
const string END_HEADER = "\r\n\r\n";
const string NOT_FOUND = "HTTP/1.1 404 Not Found" + END_LINE;
const string STATUS_OK = "HTTP/1.1 200 OK" + END_LINE;
const string STATUS_OK_NO_CONTENT = "HTTP/1.1 204" + END_LINE;
const string INTERNAL_SERVER_ERROR = "HTTP/1.1 500 Internal Server Error" + END_LINE;
const string NOT_IMPLEMENTED = "HTTP/1.1 501 Not Implemented" + END_LINE;
const string FILE_CREATED = "HTTP/1.1 201 Created" + END_LINE;
const string BAD_REQUEST = "HTTP/1.1 400 Bad Request" + END_LINE;
const string CONTENT_TYPE_TEXT_PLAIN = "Content-Type: text/plain" + END_LINE;
const string CONTENT_LEN = "Content-length: ";
const string CONTENT_TYPE_TEXT_HTML = "Content-Type: text/html" + END_LINE;

bool addSocket(SOCKET id, Status what, SocketState* sockets, int& socketsCount);
void removeSocket(int index, SocketState* sockets, int& socketsCount);
void acceptConnection(int index, SocketState* sockets, int& socketsCount);
void receiveMessage(int index, SocketState* sockets, int& socketsCount);
void sendMessage(int index, SocketState* sockets, int& socketsCount);
void buildResponse(SocketState& socket, string& reqStatus, string& body, string& headers, string& response);
void sendResponse(SOCKET msgSocket, string& response, int index, SocketState* sockets, int& socketsCount);
void fillSocket(SocketState& socket);
void getRequest(SocketState& socket, string& reqStatus, string& answer, string& header);
void optionsRequest(SocketState& socket, string& reqStatus, string& header);
void traceRequest(SocketState& socket, string& reqStatus, string& body, string& header);
void postRequest(SocketState& socket, string& reqStatus, string& body, string& header);
void deleteRequest(SocketState& socket, string& reqStatus);
void putRequest(SocketState& socket, string& reqStatus, string& body, string& header);
void updateSocketsAccordingToTimeout(SocketState* sockets, int& socketsCount);
string getLanguage(string url);
string insertResourceToBuffer(string resourceName);
#endif // HEADER_H
