#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>

#define PORT 8080
#define BUFFER_SIZE 1024

void serve_html(int client_socket)
{
    FILE *html_file = fopen("form.html", "r");
    if (html_file == NULL)
    {
        perror("Error opening HTML file");
        exit(EXIT_FAILURE);
    }

    char line[BUFFER_SIZE];
    while (fgets(line, sizeof(line), html_file) != NULL)
    {
        send(client_socket, line, strlen(line), 0);
    }

    fclose(html_file);
}

int main()
{
    int server_fd, new_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    char buffer[BUFFER_SIZE] = {0};
    char *response;
    char *success_response = "HTTP/1.1 200 OK\nContent-Type: text/html\n\n<html><body><h1>Telephone number received successfully!</h1></body></html>\n";
    char *error_response = "HTTP/1.1 400 Bad Request\nContent-Type: text/html\n\n<html><body><h1>Error: Invalid telephone number format!</h1></body></html>\n";

    // Create a socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Set socket options to allow address reuse
    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)))
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Bind the socket to localhost port 8080
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    // Listen for incoming connections
    if (listen(server_fd, 3) < 0)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    while (1)
    {
        // Accept incoming connections
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0)
        {
            perror("accept");
            exit(EXIT_FAILURE);
        }

        // Read the request from the client
        read(new_socket, buffer, BUFFER_SIZE);
        printf("Received request:\n%s\n", buffer);

        // Check if the request is for the HTML form
        if (strstr(buffer, "GET /form.html") != NULL)
        {
            // Serve the HTML form
            serve_html(new_socket);
            close(new_socket);
            continue;
        }

        // Extract telephone number from the request
        char *tel_num_start = strstr(buffer, "tel=");
        if (tel_num_start != NULL)
        {
            tel_num_start += 4; // Move past "tel="
            char *tel_num_end = strchr(tel_num_start, '&');
            if (tel_num_end == NULL)
            {
                tel_num_end = strchr(tel_num_start, ' '); // End of the line
            }
            if (tel_num_end != NULL)
            {
                *tel_num_end = '\0'; // Null-terminate the telephone number
                printf("Telephone number: %s\n", tel_num_start);
                // Here you can process the telephone number as needed
                // For now, let's just send a success response
                response = success_response;
            }
            else
            {
                // Invalid telephone number format
                response = error_response;
            }
        }
        else
        {
            // Telephone number not found in the request
            response = error_response;
        }

        // Send the response to the client
        send(new_socket, response, strlen(response), 0);
        printf("Response sent.\n");

        close(new_socket);
    }

    close(server_fd);

    return 0;
}
