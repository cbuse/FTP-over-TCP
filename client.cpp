#include "header.h"

using namespace std;

void error(string msg)
{
    perror(msg.c_str());
    exit(0);
}

int main(int argc, char *argv[])
{
	int sockfd, n, fdmax;
	struct sockaddr_in serv_addr;
	char buffer[BUFLEN];
	
	fd_set read_fds, temp_fds;
	FD_ZERO(&read_fds);

	
	if (argc < 6) {
       fprintf(stderr,"Usage %s nume_client nume_director port_client ip_server server_port\n", argv[0]);
       exit(0);
    }
	
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
        error("ERROR opening socket");

	/* Parametri conexiunii client-server */
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(atoi(argv[5]));
    inet_aton(argv[4], &serv_addr.sin_addr);


    if (connect(sockfd,(struct sockaddr*) &serv_addr,sizeof(serv_addr)) < 0)
        error("ERROR connecting");
	
		
	//se trimite serverului numele clientului si portul pe care acesta asculta
	sprintf(buffer,"0 %s %i ", argv[1], atoi(argv[3]));
	n = send(sockfd, buffer, strlen(buffer)+1, 0);
	if (n < 0)
		error("ERROR writing to socket");

    FD_SET(sockfd, &read_fds);
    
    FD_SET(0, &read_fds);
    FD_SET(0, &temp_fds);
    fdmax = sockfd;
	
	//fisierul de log
	FILE * pFile;
	char aux_argv[100];
	strcpy(aux_argv, argv[1]);
	char* log_file = strcat(aux_argv,".log");	
	pFile = fopen (log_file,"a+");
	
	
    while(1) {
		
		temp_fds = read_fds;
		
			
    	if (select(fdmax+1, &temp_fds, NULL, NULL, NULL) == -1)
    		error("Error in select");
    		
		if (FD_ISSET(0, &temp_fds)) {
			//Citesc de la tastatura
			memset(buffer, 0 , BUFLEN);			
			fgets(buffer, BUFLEN-1, stdin);
			
			//scriu comanda in fisierul de log
			fprintf(pFile, ">%s\n", buffer);
			
			
			if (strcmp(buffer, "\n") != 0) {
				char* params = NULL;
				switch (get_command(buffer, &params)) { 
					case 1: { // Comanda listclients
						if (params == NULL){ 
							n = send(sockfd,buffer,strlen(buffer)+1, 0);
							if (n < 0)
							error("ERROR writing to socket");					
							
						}
						else {
							fprintf(pFile, "Comanda listclients nu are parametri\n");
							printf("Comanda listclients nu are parametri\n");							
						}				
						
						break;
					}
					case 2:{// Comanda infoclient
						if (params == NULL){
							fprintf(pFile, "Adaugati numele clientului!\n"); 
							printf("Adaugati numele clientului!\n");
							break;
						}
						n = send(sockfd,buffer,strlen(buffer)+1, 0);
						if (n < 0)
    	 					error("ERROR writing to socket");
						break;
					}
					
					case 3: {
						// Comanda getshare
						if (params == NULL){ 
							fprintf(pFile, "Adaugati numele clientului!\n"); 
							printf("Adaugati numele clientului!\n");
							break;
						}
						n = send(sockfd, buffer, strlen(buffer)+1, 0);
						
						if (n < 0)
							error("ERROR writing to socket");
						break;
					}
					
					case 4:{
						//comanda infofile
						if (params == NULL) {
							fprintf(pFile, "Adaugati numele fisierului!\n"); 
							printf("Adaugati numele fisierului!\n");
							break;
						}
						n = send(sockfd, buffer, strlen(buffer)+1, 0);
						if (n < 0)
							error("ERROR writing to socket");
						break;
					} 
					case 5: {
						// Comanda sharefile
						
						if (params == NULL) {
							fprintf(pFile, "Adaugati numele fisierului!\n");
							printf("Adaugati numele fisierului!\n");
							break;
						}						
						
						//construiesc calea din directorul clientului spre fisierul care va fi share-uit
						// ./nume_director/nume_fisier						
						char path[BUFLEN] = "./";
						char aux3[BUFLEN];
						strcpy(aux3, argv[2]);						
						char* aux1 = strcat(aux3, "/");						
						strcat(path, aux1);	
						//concatenez calea la numele fisierului					
						strcat(path, params);
						//verific daca fisierul care urmeaza sa fie share-uit se afla in directorul clientului
						int fd;
						fd = open(path, O_RDONLY);
						if (fd < 0) {
							fprintf(pFile, "-2: Fisier inexistent!\n");
							printf("-2: Fisier inexistent!\n");
						}
						else {
							struct stat file_size;
							n = stat(path, &file_size);
							if (n < 0)
								error("ERROR file stat");
								
							char size[BUFLEN];							
							sprintf(size, "%ld", file_size.st_size);
							strcat(buffer, " ");
							strcat(buffer, size);
							n = send(sockfd, buffer, strlen(buffer)+1, 0);
							if (n < 0)
    	 						error("ERROR writing to socket");
    	 					else {
    	 						fprintf(pFile, "Succes\n");
								printf("Succes\n");
							}
						}
						close(fd);
						break;
					}
					case 6: {
						// Comanda unsharefile
						if (params == NULL){ 
							fprintf(pFile, "Adaugati numele fisierului!\n");
							printf("Adaugati numele fisierului!\n");
							break;
						}
						n = send(sockfd, buffer, strlen(buffer)+1, 0);
						if (n < 0)
    	 						error("ERROR writing to socket");
						break;
					
					}
					
					case 8: {
						n = send(sockfd,buffer, strlen(buffer)+1, 0);
						if (n < 0)
							error("ERROR writing to socket");
						else 
							close(sockfd); 
							
						fprintf(pFile, "Clientul inchide conexiunea cu serverul.\n");
						printf("Clientul inchide conexiunea cu serverul.\n");
						fclose(pFile);
						//sterg fisirul de log
						//Acesta linie poate fi comentata daca se doreste vizualizarea continutului fisierului de log
						remove(log_file);
						return 0;
						break;
					}
					default: {
						fprintf(pFile, "Comanda nu exista\n");
						printf("Comanda nu exista\n");
						break;
					}
				}
			
			}
			
		}

		else if (FD_ISSET(sockfd, &temp_fds)) {
			
			n = recv(sockfd, buffer, BUFLEN, 0);
			if (n <= 0) { 
				
				if (n == 0) {
					fprintf(pFile, "Serverul se inchide.\n");
					printf("Serverul se inchide.\n");
					break;
				}
				else
					printf("ERROR in recv");
			}
			
			else {
								
				if (strcmp(buffer, "reject") == 0) {
					printf("Exista deja un client cu acelasi nume\n");
					return -1;
				}
				else if (strcmp(buffer, "accept") == 0) {
					printf("Clientul s-a conectat la server\n");
				}
				else 
					//raspunsul cererilor de la server
					fprintf(pFile, "%s\n", buffer);
					printf("%s\n", buffer);
			}			
		}		
    }
    
    close(sockfd);    
    return 0;
}

