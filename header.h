#include <iostream>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>
#include <sys/time.h>
#include <sstream>
#include <vector>
#include <iomanip>
#include <locale>

using namespace std;

#define BUFLEN 1024

//structura care retine informatii despre fisierele share-uite
typedef struct {
	string file_name; //nume fisier
	long int size; //dimensiune in bytes
} file;

//structura care retine informatii despre clientii care se conecteaza la server
typedef struct {
	int port; //portul pe care un client astepta conexiuni
	int client_sockfd; //identificatorul primit cand se conecteaza la server	
	char start_time[30]; //ora exacta la care s-a conectat la server
	string ip; //adresa IP
	string name; //nume client	
	vector<file> shared_files; //fisere share-uite(partajate)
	
} client;

//returneaza un numar de identificare in functie de numele comenzii, precum si parametrii
//comenzii in params
int get_command(char* command, char** params);

//functie care identifica unitatea de masura a dimensiunii fisierului
int get_file_unit(long int size); 

//returneaza dimensiunea fisierului in concordanta cu unitatea de masura
int file_size(long int size); 

//returneaza unitatea de masura a dimensiunii fisierului B,KB,MB sau GB
string file_unit(long int size); 
