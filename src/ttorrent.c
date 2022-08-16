// Trivial Torrent

// TODO: some includes here

#include "file_io.h"
#include "logger.h"
#include <inttypes.h>

#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <stdio.h>
#include <inttypes.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <byteswap.h>
#include <sys/types.h>
 #include <sys/wait.h>

#define PORT 8080

// TODO: hey!? what is this?

/**
 * This is the magic number (already stored in network byte order).
 * See https://en.wikipedia.org/wiki/Magic_number_(programming)#In_protocols
 */
static const uint32_t MAGIC_NUMBER = 0xde1c3232; // = htonl(0x32321cde);

static const uint8_t MSG_REQUEST = 0;
static const uint8_t MSG_RESPONSE_OK = 1;
static const uint8_t MSG_RESPONSE_NA = 2;

enum { RAW_MESSAGE_SIZE = 13 };

struct send {

	uint8_t message;
	uint64_t blockNum;
	uint32_t num_magic;
};

/**
 * Main function.
 */
int main(int argc, char **argv) {
//printf("PAPAPAAPAPAPAAPPAAPAPAAPAPAPAPAPAPAPAPAPAPAPAPAPAPAPAPAPAAPAPAPAPAPAPAPAPAPAPAPAPAPAPAPAPPAPAPAPAPAPAPA\n");
	set_log_level(LOG_DEBUG);

	log_printf(LOG_INFO, "Trivial Torrent (build %s %s) by %s", __DATE__, __TIME__, "Guillem Alerany and Jordi Pedrero");

	// ==========================================================================
	// Parse command line
	// ==========================================================================

	// TODO: some magical lines of code here that call other functions and do various stuff.

	// The following statements most certainly will need to be deleted at some point...
	(void) argc;
	(void) argv;
	(void) MAGIC_NUMBER;
	(void) MSG_REQUEST;
	(void) MSG_RESPONSE_NA;
	(void) MSG_RESPONSE_OK;


	if(argc == 4) {
  //int status=0;
  //pid_t wpid;
		/***********************************************************************
	 	*************************PARTE DEL SERVIDOR*****************************
	 	***********************************************************************/
//printf("PAPAPAAPAPAPAAPPAAPAPAAPAPAPAPAPAPAPAPAPAPAPAPAPAPAPAPAPAAPAPAPAPAPAPAPAPAPAPAPAPAPAPAPAPPAPAPAPAPAPAPA\n");
		struct torrent_t torrent;

		// metainfo_file()
		int filelen = (int)(strlen(argv[3]) - 9);

		char downloaded_file_name[filelen + 1];

		for (int i = 0; i < filelen; i++) {

			downloaded_file_name[i] = argv[3][i];
			downloaded_file_name[filelen] = '\0';
		}

		// sock()
		int sock = socket(AF_INET, SOCK_STREAM, 0);

		if(sock < 0) {

			log_message(LOG_INFO, "Error en la creació del socket");
			exit(0);
		}

		struct sockaddr_in clientaddr, serveraddr;

		// Creem el torrent on aniran les dades que descarregarem
		int metainfoFile = create_torrent_from_metainfo_file(argv[3], &torrent, downloaded_file_name);


		// Comprovació que el torrent s'hagi creat correctament
		if (metainfoFile < 0) {

			log_message(LOG_INFO, "Error en la creació del torrent");
			exit(0);

		}

		int port = (int)strtoul(argv[2], NULL, 0);

		memset(&serveraddr, '\0', sizeof(serveraddr));
		serveraddr.sin_family = AF_INET;
		serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
		serveraddr.sin_port = htons((unsigned short)port);

			                  	      
		int bnd = bind(sock, (struct sockaddr *) &serveraddr, sizeof(serveraddr));

		if(bnd != 0) {

			log_message(LOG_INFO, "Error en el bind");
		}

		int lisn = listen(sock, 10);

		if(lisn != 0) {

			log_message(LOG_INFO, "Error en el listen");
		}


		int accept_sock;
		
		socklen_t size_serv = sizeof(serveraddr);

		//Bucle infinit. Quan s'acavin d'enviar els paquets es surt amb un exit().
		while(1) {
				
			accept_sock = accept(sock, (struct sockaddr *) &clientaddr, &size_serv);


			if(accept_sock < 0) {

				log_message(LOG_INFO, "Error en el accept");
			}

			//Creem un proces
			int id;
			id = fork();

			//Si es tracta del proces fill
			if(id == 0) {

				close(sock);

				for(unsigned int x = 0; x < torrent.block_count; x++) {
					struct send s;

					s.num_magic = htonl(MAGIC_NUMBER);
					s.message = MSG_REQUEST;
					s.blockNum = x;
					
					
					uint8_t bufferrecive[RAW_MESSAGE_SIZE] = {0};

					recv(accept_sock, bufferrecive, sizeof(bufferrecive), 0);

					memcpy(&s.num_magic, bufferrecive, 4);
					memcpy(&s.message, (bufferrecive + 4), 1);
					memcpy(&s.blockNum, (bufferrecive + 5), 8);

					s.blockNum = __builtin_bswap64(s.blockNum);
	
					if(s.message == 0) {
					
						if (torrent.block_map[s.blockNum]){
							struct block_t block;
						
							if(load_block(&torrent, s.blockNum, &block)){

							}else {

								s.message = MSG_RESPONSE_OK;

								uint8_t buffersend[RAW_MESSAGE_SIZE ] = {0};

			
								s.blockNum = __builtin_bswap64(s.blockNum);

				
								memcpy(buffersend, &s.num_magic, 4);
								memcpy((buffersend + 4), &s.message, 1);
								memcpy((buffersend + 5), &s.blockNum, 8);
								
								send(accept_sock, buffersend, RAW_MESSAGE_SIZE, 0);
								long int prova_send = 0;
								prova_send = send(accept_sock, block.data, MAX_BLOCK_SIZE, 0);
								printf("SEGON SEND %ld\n", prova_send);
							}

						
						}else{

							s.message = MSG_RESPONSE_NA;

							uint8_t buffersend[RAW_MESSAGE_SIZE] = {0};

							memcpy(buffersend, &s.num_magic, 4);
							memcpy((buffersend + 4), &s.message, 1);
							memcpy((buffersend + 5), &s.blockNum, 8);

							send(accept_sock, buffersend, RAW_MESSAGE_SIZE, 0);
								
						}
					}
				}	
				exit(0);

			}else{

			kill(0,getpid());
			wait(NULL);

			} 
	
		close(accept_sock);		
				
		}

       destroy_torrent(&torrent);



	} else if(argc == 2) {

		/***********************************************************************
	 	*************************PARTE DEL CLIENTE*****************************
	 	***********************************************************************/

		// Tractem el fitxer de metadata per tenir el fitxer on descarregar les dades
		int filelen = (int)(strlen(argv[1]) - 9);

		char downloaded_file_name[filelen + 1];

		for (int i = 0; i < filelen; i++) {

			downloaded_file_name[i] = argv[1][i];
			downloaded_file_name[filelen] = '\0';
		}

		struct torrent_t torrent;


		// Creem el torrent on aniran les dades que descarregarem
		int metainfoFile = create_torrent_from_metainfo_file(argv[1], &torrent, downloaded_file_name);

		//printf("%d\n", metainfoFile);

		// Comprovació que el torrent s'hagi creat correctament
		if (metainfoFile < 0) {

			log_message(LOG_INFO, "Error en la creació del torrent");
			//exit(0);

		} 

		// Creem l'estructura sockaddr
		struct sockaddr_in servaddr;

		memset(&servaddr, '\0', sizeof(servaddr));

		//printf("Numero de Peers: %d\n", torrent.peer_count);

		int sock;
		// Bucle por cada peer (sabem que en son 3)
		for (unsigned int i = 0; i < torrent.peer_count; i++) {

			// Creacio del socket.
			sock = socket(AF_INET, SOCK_STREAM, 0);

			if(sock < 0) {
				log_message(LOG_INFO, "Error en la creació del socket");
				exit(0);
			}

			uint32_t address = (uint32_t)(torrent.peers[i].peer_address[0] << 24 | torrent.peers[i].peer_address[1] << 16 | torrent.peers[i].peer_address[2] << 8 | torrent.peers[i].peer_address[3]); 

			servaddr.sin_family = AF_INET;
			servaddr.sin_addr.s_addr = htonl(address);
			servaddr.sin_port = torrent.peers[i].peer_port;

			// Connexió amb el peer.
			int conn = connect(sock, (struct sockaddr *) &servaddr, sizeof(servaddr));

			// Si la connexió és correcta
			if(conn == 0) {


				for(unsigned int j = 0; j < torrent.block_count; j++) {

					printf("Iteracion bucle de blockcount  : %d\n", j);
					struct send s;

					s.num_magic = htonl(MAGIC_NUMBER);
					s.message = MSG_REQUEST;

					uint8_t buffersend[RAW_MESSAGE_SIZE] = {0};
					uint8_t bufferreciv[RAW_MESSAGE_SIZE] = {0};

					s.blockNum = j;

					memcpy(buffersend, &s.num_magic, 4);
					memcpy((buffersend + 4), &s.message, 1);
					memcpy((buffersend + RAW_MESSAGE_SIZE - 1), &s.blockNum, 8);

					send(sock, buffersend, RAW_MESSAGE_SIZE, 0);
					
					recv(sock, bufferreciv, sizeof(bufferreciv), 0);
					memcpy(&s.num_magic, bufferreciv, 4);
					memcpy(&s.message, (bufferreciv + 4), 1);
					memcpy(&s.blockNum, (bufferreciv + 5), 8);

					s.blockNum = __builtin_bswap64(s.blockNum);

					if(MSG_RESPONSE_OK == s.message) {

					
						struct block_t block;

						block.size = get_block_size(&torrent, (long unsigned int)j);
						block.size = (long unsigned int)recv(sock, &block, block.size,MSG_WAITALL);
						
						store_block(&torrent, j, &block);
						
					}
				

				}

			}

		}

		close(sock);
    	destroy_torrent (&torrent);
	}

	return 0;

}

