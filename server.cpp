#include "header.h"

#define MAX_CLIENTS	20

using namespace std;

void error(string msg) {
    perror(msg.c_str());
    exit(1);
}


int main(int argc, char *argv[])
{
	int sockfd, newsockfd, portno;
	unsigned int clilen;
	char buffer[BUFLEN];
	struct sockaddr_in serv_addr, cli_addr;
	int n, i;
	
	fd_set read_fds; //multimea de citire folosita in select()
	fd_set tmp_fds;  //multime folosita temporar
	int fdmax;		 //valoare maxima file descriptor din multimea read_fds
	
	
	char time_buffer[30]; //momentul de timp la care s-a conectat un client nou
	string ip; //adresa clientului nou conectat
	int client_sockfd; //identificatorul clientului nou conectat
	
	
	//serverul tine evidenta clientilor conectati
	vector<client> server_clients;
		
	
	if (argc < 2) {
		fprintf(stderr,"Usage : %s port\n", argv[0]);
		exit(1);
	}

	//golim multimea de descriptori de citire (read_fds) si multimea tmp_fds
	FD_ZERO(&read_fds);
	FD_ZERO(&tmp_fds);

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0)
    	error("ERROR opening socket");

	portno = atoi(argv[1]);

	memset((char *) &serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	// foloseste adresa IP a masinii
	serv_addr.sin_addr.s_addr = INADDR_ANY;	
	serv_addr.sin_port = htons(portno);

	if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(struct sockaddr)) < 0)
		error("ERROR on binding");

	listen(sockfd, MAX_CLIENTS);

	// adaugam noul file descriptor (socketul pe care se 
	// asculta conexiuni) in multimea read_fds
	FD_SET(sockfd, &read_fds);
	FD_SET(0, &read_fds); // stdin se adauga la file descriptori
	fdmax = sockfd;

	// main loop
	while (1) {
		tmp_fds = read_fds;
		if (select(fdmax + 1, &tmp_fds, NULL, NULL, NULL) == -1)
			error("ERROR in select");

		for(i = 0; i <= fdmax; i++) {
			if (FD_ISSET(i, &tmp_fds)) {

				if (i == sockfd) {
					// a venit ceva pe socketul inactiv(cel cu listen) = o noua conexiune
					// actiunea serverului: accept()

					clilen = sizeof(cli_addr);
					if ((newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen)) == -1) {
						error("ERROR in accept");
					}
					else {
						//adaug noul socket intors de accept() la multimea descriptorilor de citire
						FD_SET(newsockfd, &read_fds);
						if (newsockfd > fdmax)
							fdmax = newsockfd;
					}
					printf("Noua conexiune de la %s, port %d, socket_client %d\n",
							inet_ntoa(cli_addr.sin_addr), ntohs(cli_addr.sin_port), newsockfd);
					
					//retin adresa ip, newsockfd si timpul la care s-a conectat un nou client
					ip = inet_ntoa(cli_addr.sin_addr);					
					
					time_t rawtime;
					struct tm *information;										
					time(&rawtime);
					information = localtime( &rawtime );      
					strftime(time_buffer,30,"%r", information);
					
					client_sockfd = newsockfd;
				}

				else if (i == 0) {
					//Se citeste de la stdin
					memset(buffer, 0 , BUFLEN);					
					fgets(buffer, BUFLEN-1, stdin);
					
					//daca serverul primeste comanda status
					if (strcmp(buffer, "status\n") == 0) {					
						
						unsigned int i;
						//pentru fiecare client afisez numele, adresa IP si portul pe care 
						//asteapta eventuale conexiuni
						for (i = 0; i < server_clients.size(); i++){
							printf("nume: %s  adresa IP: %s  port: %d\n",
									server_clients[i].name.c_str(), server_clients[i].ip.c_str(), 
									server_clients[i].port);

						}
					}
					//daca serverul primeste comanda quit
					else if (strcmp(buffer, "quit\n") == 0) {
						//se inchid toti clientii conectati in acel moment
						int i;
						for(i = 0; i <= fdmax; i++) {
							FD_CLR(i, &read_fds);
							close(i);
						}
						//`se inchide si serverul
						close(sockfd); 
						return 0;
					}

					else { 
						printf("Comanda nu exista!\n");
					}
				}
				
				
				else {
					// am primit date pe unul din socketii cu care vorbesc cu clientii
					//actiunea serverului: recv()
					memset(buffer, 0, BUFLEN);
					if ((n = recv(i, buffer, sizeof(buffer), 0)) <= 0) {
						if (n == 0) {
							//conexiunea s-a inchis
							printf("Clientul cu socketfd %d a inchis conexiunea.\n", i);
														
						} else {
							error("ERROR in recv");
						}
						close(i);
						FD_CLR(i, &read_fds); // scoatem din multimea de citire socketul pe care
					}

					else { //recv intoarce >0
												
						//am primit informatii  despre un client nou conectat
						if(buffer[0] == '0') {
							int port;
							char* name;							
							char* port_string;
							
							char aux_buffer[BUFLEN]; 
							strcpy(aux_buffer, buffer);	
							
							strtok(aux_buffer," ");							
							name = strtok(NULL," ");
							port_string = strtok(NULL, " ");
							
							port = atoi(port_string);							
							
							string name_string(name);								
						
							//pentru cazul in care nu s-a conectat inca niciun client
							if(server_clients.size() == 0) {
								client client;
								
								client.name = name_string;
								client.port = port;
								client.ip = ip;
								strcpy(client.start_time, time_buffer);
								client.client_sockfd = client_sockfd;
								
								server_clients.push_back(client);
								
								strcpy(buffer, "accept");
								send(i, buffer, strlen(buffer)+1, 0);
								
								
							} 
							else {
							//nu exista client cu acelasi nume
							int ok = 0;
							for (unsigned int ii = 0; ii < server_clients.size(); ii++) {
								//daca nu mai am niciun client cu acleasi nume il adaug
								if(server_clients[ii].name == name_string) {
																		
									ok = 1;
									break;												
								}
							}
							//daca nu este in sistem un client cu acelasi nume
							if(ok == 0) {
							
								client client;
								
								client.name = name_string;
								client.port = port;
								client.ip = ip;
								strcpy(client.start_time, time_buffer);
								client.client_sockfd = client_sockfd;
								
								//adaug noul client in baza de date a serverului
								server_clients.push_back(client);
								
								strcpy(buffer, "accept");
								send(i, buffer, strlen(buffer)+1, 0);
								
							}
							else{
								//daca exista un client cu acelasi nume il resping
								strcpy(buffer, "reject");
								send(i, buffer, strlen(buffer)+1, 0);
							}
							
						}
								
							break;
						}
						
						else {  
							//Am primit o comanda de la un client
							char* params = NULL;
																				
							switch (get_command(buffer, &params)){ 
								case 1: {
									// Am primit comanda listclients
									char destbuffer[BUFLEN] = "";
																			
									unsigned int ii;
									//trimit clientului care mi-a cerut lista de nume ale clientilor conectati
									for (ii = 0; ii < server_clients.size(); ii++) {											
										strcat(destbuffer, server_clients[ii].name.c_str());
										strcat(destbuffer, "\n");
									}
									
									send(i, destbuffer, strlen(destbuffer)+1, 0);
									break;
								}
								
								case 2: { 
									//Am primit comanda infoclient
									char destbuffer[BUFLEN];
									if (params != NULL) {  
										string name(params);
										unsigned int ii, pos, ok = 0/*nu am gasit clientul despre care se cer info*/;
										
										//caut clientul despre care se cer informatii
										for (ii = 0; ii < server_clients.size(); ii++) {
											if(server_clients[ii].name == name) {
												ok = 1;
												pos = ii;
												break;
											}									
										}
										//am gasit clientul despre care se cer informatii
										if(ok == 1) {
											//returnez informatiile cerute despre clientul de pe pozitia pos
											sprintf(destbuffer,"nume: %s, port: %i, timp conectare: %s",
													server_clients[pos].name.c_str(), server_clients[pos].port, 
													server_clients[pos].start_time);
											
										}
										else
										//nu am gasit clientul despre care se cer informatii
										sprintf(destbuffer,"-1: Client inexistent");											
											
									send(i, destbuffer, strlen(destbuffer)+1, 0);
									}		
									break;					
								}
								
								case 3: { 
								// Am primit comanda getshare
									char destbuffer[BUFLEN] = "";

									if (params != NULL) { 
									
										string name(params);
										long int size; 
										sscanf(buffer, "%*s %*s %ld", &size);
										
										unsigned int ii, pos, ok = 0/*nu am gasit clientul despre care se cer info*/;
									
										for (ii = 0; ii < server_clients.size(); ii++) {
											if(server_clients[ii].name == name) {
												ok = 1;
												//retin pozitia clientului
												pos = ii;
												break;
											}											
										}
										
										//am gasit clientul cerut si ii trimit clientului care a cerut info 											
										if(ok == 1) {
											
											unsigned int iii;
											
											for (iii = 0; iii < server_clients[pos].shared_files.size(); iii++) {
											
												strcat(destbuffer, server_clients[pos].shared_files[iii].file_name.c_str());
												strcat(destbuffer, " ");
												
												int user_friendly_size = file_size(server_clients[pos].shared_files[iii].size);
												string unit = file_unit(server_clients[pos].shared_files[iii].size);
												
												string result;
												ostringstream convert;
												convert << user_friendly_size;
												result = convert.str();													
												strcat(destbuffer, result.c_str());
												
												strcat(destbuffer, unit.c_str());
												strcat(destbuffer, "\n");											
													
											}
											
											//daca clientul nu are nimic la share
											if (server_clients[pos].shared_files.size() == 0) {
												sprintf(destbuffer, "Clientul nu a dat la share nicun fisier");
												send(i, destbuffer, strlen(destbuffer)+1, 0);	
											}
											else								
												send(i, destbuffer, strlen(destbuffer)+1, 0);			
											
										}
										else {
											sprintf(destbuffer, "-1: Client inexistent");
											send (i, destbuffer, strlen(destbuffer)+1, 0);
										}								
									

									}										
									break;
								}
								
								case 4: {
									// Am primit comanda infofile
									char destbuffer[BUFLEN] = "";

									if (params != NULL) { 
									
										string file_name(params);											
										
										unsigned int iii, ii;
										int ok = 0/* nimeni nu partajeaza fisierul*/;
										
										//parcurg fiecare client										
										for(ii = 0; ii < server_clients.size(); ii++) {
											
											//parcurg fisierele fiecarui client
											for (iii = 0; iii < server_clients[ii].shared_files.size(); iii++) {
												
												//daca am gasit un fisier cu numele cautat
												if(server_clients[ii].shared_files[iii].file_name == file_name) {
													
													ok = 1;
													strcat(destbuffer, server_clients[ii].name.c_str());
													strcat(destbuffer, " ");
													strcat(destbuffer, server_clients[ii].shared_files[iii].file_name.c_str());
													strcat(destbuffer, " ");
													
													int user_friendly_size = file_size(server_clients[ii].shared_files[iii].size);
													string unit = file_unit(server_clients[ii].shared_files[iii].size);
																						
													string result;
													ostringstream convert;
													convert << user_friendly_size;
													result = convert.str();													
													strcat(destbuffer, result.c_str());
												
													strcat(destbuffer, unit.c_str());
													strcat(destbuffer, "\n");
																			
													
												}
											}
										}								
																			
														
											
										
										if(ok == 0) {
											sprintf(destbuffer, "-2: Fisier inexistent");
											send (i, destbuffer, strlen(destbuffer)+1, 0);
										}
										else{
											printf("destbuffer %s\n", destbuffer );
											send(i, destbuffer, strlen(destbuffer)+1, 0);
										}								
									

									}		
									break;
								}
								
								case 5: { 
									//Am primit comanda sharefile
									if (params != NULL) { 
										string file_name(params);
										long int size; 
										sscanf(buffer, "%*s %*s %ld", &size);
										
										unsigned int ii, pos, ok = 0/*nu am gasit clientul despre care se cer info*/;
									
										for (ii = 0; ii < server_clients.size(); ii++) {
											if(server_clients[ii].client_sockfd == i) {
												ok = 1;
												pos = ii;
												break;
											}											
										}
										
										//daca  am gasit clientul ii adaugam la share fisierul
										
										if(ok == 1) {
											
											//daca s-a share-uit deja fisierul respectiv
											unsigned int jj, ok0 = 0/*nu am gasit fisierul*/;
									
											for (jj = 0; jj < server_clients[pos].shared_files.size(); jj++) {
												if(server_clients[pos].shared_files[jj].file_name == file_name) {
													//fisierul respectiv a fost deja share-uit
													ok0 = 1;														
													break;
												}											
											}
											
											//daca fisierul nu a fost share-uit il adaug 
											if(ok0 == 0) {
												file f;
												f.file_name = file_name;
												f.size = size;
											
												server_clients[pos].shared_files.push_back(f);
												
											}
											
											
										}								
										
									}
									
									break;
								}
								
								case 6: { 
									//Am primit comanda unsharefile
									char destbuffer[BUFLEN];

									if (params != NULL) { 
										string file_name(params);
										long int size; 
										sscanf(buffer, "%*s %*s %ld", &size);
										
										unsigned int ii, pos, ok = 0/*nu am gasit clientul despre care se cer info*/;
									
										for (ii = 0; ii < server_clients.size(); ii++) {
											if(server_clients[ii].client_sockfd == i) {
												ok = 1;
												pos = ii;
												break;
											}											
										}
										
										//am gasit clientul caut fisierul care va fi sters de la share
										
										if(ok == 1) {
											
											//daca exista fisierul respectiv la share
											unsigned int jj, pos0, ok0 = 0/*nu am gasit fisierul*/;
									
											for (jj = 0; jj < server_clients[pos].shared_files.size(); jj++) {
												if(server_clients[pos].shared_files[jj].file_name == file_name) {
													//fisierul respectiv a fost deja share-uite la share
													pos0 = jj;
													ok0 = 1;														
													break;
												}											
											}
											
											//daca fisierul e la share il sterg
											if(ok0 == 1) {
												
												server_clients[pos].shared_files.erase(server_clients[pos].shared_files.begin() + pos0);
												sprintf(destbuffer, "Succes");
											}
											else
												sprintf(destbuffer, "-2: Fisier inexistent!");

											
											send(i, destbuffer, strlen(destbuffer)+1, 0);
										}								
									

									}
									
									break;
								}							
								
								
								case 8: {
									//Am primit comanda quit 
									
									//caut clientul care a inchis conexiunea si il sterg
									unsigned int ii;
									int pos;
									for (ii = 0; ii < server_clients.size(); ii++) {
										
										if(server_clients[i].client_sockfd == i) {
											//pastrez pozitia pentru a o sti ce sa sterg
											pos = ii;							
											break;												
										}
									}
									server_clients.erase(server_clients.begin() + pos);
									
									FD_CLR(i, &read_fds);
									close(i);
									break;
								}
								default: {
									printf("Eroare\n");
									break;
								}
				
							}
							break;
						}						
					}
				}
			}
		}
	}

	close(sockfd);

	return 0;

}

