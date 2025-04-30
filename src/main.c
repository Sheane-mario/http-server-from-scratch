#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <arpa/inet.h>

char *directory = NULL;

void handle_client(int client_fd) {
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

        char body[1024];

        // Endpoints
        char endpoint[] = "/user-agent";
        char endpoint_echo[] = "/echo/";
        char endpoint_files[] = "/files/";
        // End of endpoints

        // Request headers
        char header[] = "User-Agent: ";
        // End of request headers

        // Accept from / endpoint and send status OK
        if (strcmp(path, "/") == 0) {
            char index[] = 
                "HTTP/1.1 200 OK\r\n"
                "Content-Type: text/plain\r\n"
                "Content-Length: 1\r\n"
                "\r\n"
                "/";
            send(client_fd, index, strlen(index), 0);
        }

        // if the request path is /user-agent
        if (strcmp(path, endpoint) == 0 ) {
            char *line = strtok(req_buf, "\r\n");
            while (line != NULL) {
                char header_type[256];
                char header_content[1024];
                char res_buf[4096];
                strncpy(header_type, line, strlen(header));
                if (strcmp(header_type, header) == 0) {
                    strncpy(header_content, line + strlen(header), strlen(line) - strlen(header_type));
                    snprintf(res_buf, sizeof(res_buf), res_temp, strlen(header_content), header_content);
                    send(client_fd, res_buf, strlen(res_buf), 0);
                }
                line = strtok(NULL, "\r\n");
            }
        }

        // if path length is < /echo/ endpoint
        if (strlen(path) - strlen(endpoint_echo) < 0) {
           send(client_fd, res_b, strlen(res_b), 0);
        }

        char file_req_endpt[20];
        strncpy(file_req_endpt, path, strlen(endpoint_files));
        if (strcmp(file_req_endpt, endpoint_files) == 0) {
            char req_file[2048];
            strncpy(req_file, path + strlen(endpoint_files), strlen(path) - strlen(endpoint_files));
            // construct full file path directory/file_path
            char file_path[4096];
            snprintf(file_path, sizeof(file_path), "%s/%s", directory, req_file);

            // handling POST request
            if (strcmp(method, "POST") == 0) {
               char *body = strstr(req_buf, "\r\n\r\n");
               if (body != NULL) {
                    body += 4; // skip \r\n\r\n 
               }
               int bd_len = strlen(body);

               // open a file for writing the request body
               FILE *fp = fopen(file_path, "wb");
               if (fp == NULL) {
                    char res_svr_err[] = 
                        "HTTP/1.1 500 Internal Server Error\r\n"
                        "Content-Type: text/plain\r\n"
                        "Content-Length: 21\r\n"
                        "\r\n"
                        "Internal Server Error";
                    send(client_fd, res_svr_err, strlen(res_svr_err), 0);
               } else {
                    fwrite(body, 1, bd_len, fp); 
                    fclose(fp);
                    // response for created 
                    char res_crtd[] = 
                        "HTTP/1.1 201 Created\r\n\r\n";
                    send(client_fd, res_crtd, strlen(res_crtd), 0);
               }
            }

            // handling GET request
            FILE *fp = fopen(file_path, "rb");
            if (fp == NULL) {
                send(client_fd, res_b, strlen(res_b), 0);
            } else {
                // get file size
                fseek(fp, 0, SEEK_END);
                long fsize = ftell(fp);
                rewind(fp);

                // read file contents
                char *file_buf = malloc(fsize);
                fread(file_buf, 1, fsize, fp);
                fclose(fp);

                // Good response template
                char res_temp_file[] = 
                    "HTTP/1.1 200 OK\r\n"
                    "Content-Type: application/octet-stream\r\n"
                    "Content-Length: %ld\r\n"
                    "\r\n"
                    "%s";
                char res_file[2048];
                snprintf(res_file, sizeof(res_file), res_temp_file, fsize, file_buf);
                send(client_fd, res_file, strlen(res_file), 0);
                free(file_buf);
            }
        }
        
        // drop endpoints not from /echo/
        char req_endpt[20];
        // extract prefix of requested endpoint to check whether it mathes /echo/
        strncpy(req_endpt, path, strlen(endpoint_echo));
        if (strcmp(req_endpt, endpoint_echo) != 0) {
            send(client_fd, res_b, strlen(res_b), 0);
        } else {
            // To store client requesting file; like /echo/index.php
            char req_fl_buf[2048];
            strncpy(req_fl_buf, path + strlen(endpoint_echo), strlen(path) - strlen(endpoint_echo));
            // Prepare the response by inserting into the res template
            char *pcmprs = strstr(req_buf, "Accept-Encoding: ");
            if (pcmprs != NULL) {
                pcmprs += 17;
            }
            printf("%s\n", req_fl_buf);
//            printf("%s %ld\n", pcmprs, strlen(pcmprs));
            if (pcmprs != NULL) {
                char *cmpr = strtok(pcmprs, ", ");
                int f = 0; // set to 1 when gzip is included otherwise set to 0
                while (cmpr != NULL) {
                    if (strcmp(cmpr, "gzip") == 0) {
                        f = 1;
                        break;
                    }
                    cmpr = strtok(NULL, ", ");
                }
                if (f == 1) {
                    char res_enc_hdr[] = 
                        "HTTP/1.1 200 OK\r\n"
                        "Content-Type: text/plain\r\n"
                        "Content-Encoding: gzip\r\n"
                        "\r\n";
                    send(client_fd, res_enc_hdr, strlen(res_enc_hdr), 0);
                } else {
                    char res_enc_hdr[] = 
                        "HTTP/1.1 200 OK\r\n"
                        "Content-Type: text/plain\r\n"
                        "\r\n";
                    send(client_fd, res_enc_hdr, strlen(res_enc_hdr), 0);

                }
            }
            char res[4096];
            snprintf(res, sizeof(res), res_temp, strlen(req_fl_buf), req_fl_buf);
            send(client_fd, res, strlen(res), 0);
        }
        close(client_fd);
}


int main(int argc, char *argv[])
{
	// Disable output buffering
	setbuf(stdout, NULL);
	setbuf(stderr, NULL);

    for (int i = 1; i < argc - 1; i++) {
        if (strcmp(argv[i], "--directory") == 0) {
            directory = argv[i + 1];  
            break;
        }
    }

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

    while (1) {
        int client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_addr_len);
        printf("Client connected\n");

        pid_t pid = fork();
        // child process
        if (pid == 0) {
            close(server_fd); // child doesn't need listener
            handle_client(client_fd);
            exit(0);
        } else if (pid > 0) {
            // parent process
            close(client_fd); // parent doesn't need client fd
        } else {
            printf("fork failed: %s \n", strerror(errno));
            close(client_fd);
        }

    }


//    if (bd_len < 0) {
//        send(client_fd, res_b, strlen(res_b), 0);
//    } else {
        // strncpy copy or extract a substring and store the result in a different char array 
//        strncpy(bd, path + strlen(pref), bd_len);
//    }

//    char res_buf[4096];
    // snprintf() can insert data into the format specifiers like (%s %d) in a template and store the final result in char array 
//    snprintf(res_buf, sizeof(res_buf), res_temp, strlen(bd), bd);

//    char path_pref[1024];

//    if (strcmp(strncpy(path_pref, path, strlen(pref)), pref) != 0) {
//        send(client_fd, res_b, strlen(res_b), 0);
//    } else {
//        send(client_fd, res_buf, strlen(res_buf), 0);
//    }

    // close the connection
	close(server_fd);

	return 0;
}
