#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

int main()
{
	// Disable output buffering
	setbuf(stdout, NULL);
	setbuf(stderr, NULL);

	// You can use print statements as follows for debugging, they'll be visible when running tests.
	printf("Logs from your program will appear here!\n");

	// Uncomment this block to pass the first stage

	int server_fd, client_addr_len;
	struct sockaddr_in client_addr;

	server_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (server_fd == -1)
	{
		printf("Socket creation failed: %s...\n", strerror(errno));
		return 1;
	}

	// Since the tester restarts your program quite often, setting SO_REUSEADDR
	// ensures that we don't run into 'Address already in use' errors
	int reuse = 1;
	if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0)
	{
		printf("SO_REUSEADDR failed: %s \n", strerror(errno));
		return 1;
	}

	struct sockaddr_in serv_addr = {
		.sin_family = AF_INET,
		.sin_port = htons(4221),
		.sin_addr = {htonl(INADDR_ANY)},
	};

	if (bind(server_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) != 0)
	{
		printf("Bind failed: %s \n", strerror(errno));
		return 1;
	}

	int connection_backlog = 5;
	if (listen(server_fd, connection_backlog) != 0)
	{
		printf("Listen failed: %s \n", strerror(errno));
		return 1;
	}

	printf("Waiting for a client to connect...\n");
	client_addr_len = sizeof(client_addr);

	int client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_addr_len);
	printf("Client connected\n");

    // Good response template
    char res_temp[] = 
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/plain\r\n"
        "Content-Length: %d\r\n"
        "\r\n"
        "%s";
    // Bad response
    char res_b[] = 
        "HTTP/1.1 404 Not Found\r\n"
        "Content-Type: text/plain\r\n"
        "Content-Length: 9\r\n"
        "\r\n"
        "Not Found";

    // place to store the client request
    char req_buf[4096]; 
    // receive the request from the client
    recv(client_fd, req_buf, sizeof(req_buf), 0);

    // print the clint request
    printf("-----HTTP REQUEST-----\n");
    printf("%s \n", req_buf);
    printf("\n----------------------\n");

    // arrays to store the http method and url path
    char method[16], path[1024];

    sscanf(req_buf, "%s %s", method, path);

    char bd[1024];
    char pref[] = "/echo/";
    int bd_len = strlen(path) - strlen(pref);
    // strncpy copy or extract a substring and store the result in a different char array 
    strncpy(bd, path + strlen(pref), bd_len);

//    if (strcmp(path, "/") == 0 && strcmp(method, "GET") == 0) {
        // send the response to the client
//        send(client_fd, res_buf, strlen(res_buf), 0);
//    } else {
//        send(client_fd, res_buf_b, strlen(res_buf_b), 0);
//    }

    char res_buf[4096];
    // snprintf() can insert data into the format specifiers like (%s %d) in a template and store the final result in char array 
    snprintf(res_buf, sizeof(res_buf), res_temp, strlen(bd), bd);

    send(client_fd, res_buf, strlen(res_buf), 0);

    // close the connection
	close(server_fd);

	return 0;
}
