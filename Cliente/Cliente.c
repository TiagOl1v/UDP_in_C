/*============================== =
Este cliente destina - se a enviar mensagens passadas na linha de comando, sob
a forma de um argumento, para um servidor especifico cuja locacao e' dada
pelas seguintes constantes : SERV_HOST_ADDR(endereco IP) e SERV_UDP_PORT(porto)

O protocolo usado e' o UDP.
==============================================================================*/

#include <winsock.h>
#include <stdio.h>

#pragma comment (lib, "Ws2_32.lib")

//#define SERV_HOST_ADDR "127.0.0.1"
//#define SERV_UDP_PORT  6000
#define BUFFERSIZE 4096

void Abort(char* msg);

/*________________________________ main _______________________________________
*/

int main(int argc, char* argv[])
{

	SOCKET sockfd;
	int msg_len, iResult, nbytes, length_addr,opt;
	struct sockaddr_in serv_addr,cli_addr,serv2_addr;
	char buffer[BUFFERSIZE], serv2_ip, serv_ip;
	DWORD timeout = 10000;
	WSADATA wsaData;

	/*========================= TESTA A SINTAXE =========================*/
	//No 5 alterado de arcv 2 para 4 para permitir os novos 2 parametros
	if (argc != 4) {
		fprintf(stderr, "Sintaxe: %s frase_a_enviar ip porto\n", argv[0]);
		getchar(); //system("pause");
		exit(EXIT_FAILURE);
	}

	/*=============== INICIA OS WINSOCKS ==============*/

	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		printf("WSAStartup failed: %d\n", iResult);
		getchar(); //system("pause");
		exit(1);
	}

	/*=============== CRIA SOCKET PARA ENVIO/RECEPCAO DE DATAGRAMAS ==============*/

	sockfd = socket(PF_INET, SOCK_DGRAM, 0);
	if (sockfd == INVALID_SOCKET)
		Abort("Impossibilidade de criar socket");

	/*=============== 7 - DECLARAR A OPÇÃO DO SOCKET PARA PERMITIR BROADCAST ==============*/
	opt = 1;
	setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, (char*)&opt, sizeof(opt));


	/*===============8- ATENDER COM TIMEOUT TIMEOUT DO SOCKET ==============*/
	setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout));


	/*================= PREENCHE ENDERECO DO SERVIDOR ====================*/

	memset((char*)&serv_addr, 0, sizeof(serv_addr)); /*Coloca a zero todos os bytes*/
	serv_addr.sin_family = AF_INET; /*Address Family: Internet*/
	serv_addr.sin_addr.s_addr = inet_addr(argv[2]); /*IP no formato "dotted decimal" => 32 bits*/
	serv_addr.sin_port = htons(atoi(argv[3])); /*Host TO Netowork Short*/

	/*====================== ENVIA MENSAGEM AO SERVIDOR ==================*/

	msg_len = strlen(argv[1]);

	if (sendto(sockfd, argv[1], msg_len + 1, 0, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == SOCKET_ERROR)
		Abort("O subsistema de comunicacao nao conseguiu aceitar o datagrama");

	printf("<CLI1>Mensagem enviada ... a entrega nao e' confirmada.\n");

	/*================ MOSTRA PORTO AUTOMATICAMENTE ATRIBUIDO ==============*/

		length_addr = sizeof(cli_addr);
		if (getsockname(sockfd, (struct sockaddr*)&cli_addr, &length_addr) == SOCKET_ERROR) {
			Abort("Erro ao obter informações sobre o socket");
		}
		else {
			printf("<CLI1>Porto local automatico: %d\n", ntohs(cli_addr.sin_port));
		}

	/*====================== RECEBER RESPOSTA DO SERVIDOR ==================*/
		length_addr = sizeof(serv2_addr);
		nbytes = recvfrom(sockfd, buffer, sizeof(buffer), 0, (struct sockaddr*)&serv2_addr, &length_addr);

		if (nbytes == SOCKET_ERROR) {
			/* ============== = 8 - Parte 2 - Verificar o timeout ================ */
			if (WSAGetLastError() == WSAETIMEDOUT) {
				Abort("Timeout do servidor");
			}
			else {
				Abort("Erro na recepcao de datagramas");
			}
		}

		buffer[nbytes]='\0'; /*Termina a cadeia de caracteres recebidos com '\0'*/

		/*========= 6 - Validar se a resposta foi de facto recebida do servidor ==================*/

		printf("\n<SER1>Mensagem recebida {%s} de {IP: %s; porto: %d}\n",
			buffer, inet_ntoa(serv2_addr.sin_addr), ntohs(serv2_addr.sin_port));

		if (strcmp(inet_ntoa(serv2_addr.sin_addr),argv[2]) == 0 && ntohs(serv2_addr.sin_port) == atoi(argv[3]))
		{
			printf("\n<SER1>Recebi esta mensagem do servidor\n");
		}
		else
		{
			printf("\n<SER1>Nao recebi esta mensagem do servidor\n");
		}
	/*========================= FECHA O SOCKET ===========================*/

	closesocket(sockfd);
	WSACleanup();

	printf("\n");
	getchar();
	exit(EXIT_SUCCESS);
}

/*________________________________ Abort________________________________________
  Mostra uma mensagem de erro e o código associado ao ultimo erro com Winsocks.
  Termina a aplicacao com "exit status" a 1 (constante EXIT_FAILURE)
________________________________________________________________________________*/

void Abort(char* msg)
{

	fprintf(stderr, "<CLI1>Erro fatal: <%s> (%d)\n", msg, WSAGetLastError());
	exit(EXIT_FAILURE);

}
