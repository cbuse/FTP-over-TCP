#include "header.h"

int get_command(char* command, char** params) {
 
	char aux_command[BUFLEN];
	strcpy(aux_command, command);
	strcat(aux_command, " \n");
	strtok(aux_command, " \n");
	*params = strtok(NULL, " \n");	
	
		
	if(strncmp(command,"listclients", strlen("listclients")) == 0) { 
		return 1;
	}
	
	if(strncmp(command,"infoclient", strlen("infoclient")) == 0) {
		return 2;
	}
	
	if(strncmp(command,"getshare", strlen("getshare")) == 0) {
		return 3;
	}
	
	if(strncmp(command,"infofile", strlen("infofile")) == 0) {
		return 4;
	}
	
	if(strncmp(command,"sharefile", strlen("sharefile")) == 0) { 
		return 5;
	}
	
	if(strncmp(command,"unsharefile", strlen("unsharefile")) == 0) { 
		return 6;
	}
	
	if(strncmp(command,"quit", strlen("quit")) == 0) {
		return 8;
	}
		
	return -1;
}


int get_file_unit(long int size) {
	if(size > 1024*1024*1024)	
		//dimensiunea fisierului e in GB			
		return 1;
	if(size > 1024*1024 && size < 1024*1024*1024)
		//dimensiunea fisierului e in MB
		return 2;
	if(size > 1024 && size < 1024*1024)
		//dimensiunea fisierului e in KB
		return 3;
	
	//dimensiunea fisierului e in B
	return 4;
}

int file_size(long int size) {

	if(get_file_unit(size) == 1)
		return size / (1024*1024*1024);
	if(get_file_unit(size) == 2)
		return size / (1024*1024);
	if(get_file_unit(size) == 3)
		return size / 1024;
	
	return size;
}

string file_unit(long int size) {
	if(get_file_unit(size) == 1)
		return "GB";
	if(get_file_unit(size) == 2)
		return "MB";
	if(get_file_unit(size) == 3)
		return "KB";	
	return "B";
}
