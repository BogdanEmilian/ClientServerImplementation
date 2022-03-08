
//	**************************************************
//	*  cn_server.cpp - simple server implementation  *
//	**************************************************
//	*  version - 2.0                                 *
//	*  author  - stelian mihalas                     *
//	*  date    - 2019/02/15                          *
//	**************************************************

#include <winsock2.h>
#include <iostream>
#include <io.h>

using namespace std;

//	--------------------------------------------------------------------

#define FD_SET_SIZE 16
#define MAX_LINE 1024
#define SERV_PORT 8080

//	====================================================================

//	main

//	--------------------------------------------------------------------

int
main(int argc, char** argv)
{
	int i, maxfd, listen_fd, conn_fd;
	int num_ready, client[FD_SET_SIZE];
	int n, err, cli_len;
	fd_set	rset;
	char line[MAX_LINE];
	char mess_to_client[MAX_LINE];
	char cha;
	struct sockaddr_in cli_addr, serv_addr;

	//	some windows specific code is necessary; check the error below
    WORD wVersionRequested;
    WSADATA wsaData;

    wVersionRequested = MAKEWORD(2, 2);
    err = WSAStartup(wVersionRequested, &wsaData);
	//	**

	//	create the listening (server) socket

	//	**
    listen_fd = socket(AF_INET, SOCK_STREAM, 0);

	//	this testing block was used to detect the socket startup error
	if (listen_fd <0) {
		err = WSAGetLastError();
		cout << " socket() failed, error = " << err << endl;

		if (err == WSAENETDOWN)	cout << "NETDOWN" << endl;
		else if (err == WSANOTINITIALISED) cout << "NOINIT" << endl;
		else if (err == WSAEAFNOSUPPORT) cout << "NOSUPPORT" << endl;
		else if (err == WSAEINPROGRESS) cout << "IN PROGRESS" << endl;
		else if (err == WSAEMFILE) cout << "EM FILE" << endl;
		else if (err == WSAENOBUFS) cout << "NOBUFS" << endl;
		else if (err == WSAEPROTONOSUPPORT) cout << "PROTO" << endl;
		else if (err == WSAEPROTOTYPE) cout << "PROTOTYPE" << endl;
		else if (err == WSAESOCKTNOSUPPORT) cout << "SOCK NO SUPP" << endl;
		else cout << "strange error" <<endl;

		cin >> cha;
		exit(1);
	}

    // initialize the serv_addr structure
    memset(line, 0, MAX_LINE);
    memset(&serv_addr, 0, sizeof(serv_addr));

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_addr.sin_port = htons(SERV_PORT);

	//	do the bind here

	err=bind(listen_fd, (sockaddr *)&serv_addr, sizeof(serv_addr));

	//	**

	if (err < 0) {
		err = WSAGetLastError();
		cout << " bind() failed, error = " << err << endl;
		cin >> cha;
		exit(1);
	}
	else {
		cout << "Server bound at port " << SERV_PORT << endl;
	}

	//	go into listening mode

	err=listen(listen_fd, 16);
	//	**

	// the biggest fd so far is maxfd
	maxfd = listen_fd;

	// initialize the client fds list (array) with -1
	for (i = 0; i < FD_SET_SIZE; i++) {
		client[i] = -1;
	}

	FD_ZERO(&rset);             // clear the read_set array
	FD_SET(listen_fd, &rset);   // declare the listen_fd to be of interest

	//	----------------------------------------------------------------

	while (1) {
        // the program will wait for activity on the read_set fds
		num_ready = select(maxfd + 1, &rset, NULL, NULL, NULL);

		if (FD_ISSET(listen_fd, &rset))	// new client connection request
		{
			cli_len = sizeof(cli_addr);

			//	here you can do the accept
            conn_fd=accept(listen_fd, (sockaddr *)&cli_addr, &cli_len);

			//	**

			cout << "accepted connection. fd = " << conn_fd << endl;

			strcpy(mess_to_client, "got connection request, client\n");
            send(conn_fd, mess_to_client, strlen(mess_to_client), 0);

			// look for an empty slot inn the client fds array
			for (i = 0; i < FD_SET_SIZE; i++) {
				if (client[i] < 0) {
					client[i] = conn_fd;	// save the fd here
					break;
				}
			}

			if (i == FD_SET_SIZE) {
				cout << " ** too many clients. will exit." << endl;
				exit(1);
			}

			FD_SET(conn_fd, &rset);  // declare the new fd to be of interest

			if (conn_fd > maxfd)	maxfd = conn_fd;    // adjust maxfd
			if (--num_ready <= 0)	continue;
		}

		// check all clients for data
        for (i = 0; i < FD_SET_SIZE; i++) {
			if (client[i] < 0)	continue;

			if (FD_ISSET(client[i], &rset)) {
                // check if we get an error, the client has disconnected
				if ((n = recv(client[i], line, MAX_LINE, 0) <= 0)) {
                    FD_CLR(client[i], &rset);    // this fd is no longer of interest
					closesocket(client[i]);         // close the socket
					client[i] = -1;
				}
				else {
				    strcpy(mess_to_client, "received message : ");
                    strcat(mess_to_client, line);
                    int ipAddr = cli_addr.sin_addr.s_addr;
                    strcat(mess_to_client, "\nfrom IP address : ");
                    strcat(mess_to_client, inet_ntoa(cli_addr.sin_addr));
                    cout << mess_to_client << endl;
                    strcpy(mess_to_client, "OK, client, got your message");
                    send(client[i], mess_to_client, strlen(mess_to_client), 0);
				}

                // no other fd with activity
				if (--num_ready <= 0)	break;
			}
		}   // end of clients array parsing
	}   // end of the while loop
	return 0;
}

//	====================================================================

