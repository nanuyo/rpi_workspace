#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#define PORT 8080
#define BUFFER_SIZE 4096

void serve_html(int client_socket);
void handle_post_request(int client_socket, char *buffer);

int main()
{
    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    socklen_t addrlen = sizeof(address);
    char buffer[BUFFER_SIZE] = {0};

    system("sudo nmcli device wifi hotspot ifname wlan0 con-name MyHostspot ssid MyHostspotSSID password 12345678");
    system("sudo nmcli connection modify MyHostspot ipv4.addresses 192.168.1.1/24");
    // nmcli dev wifi list
    system("sudo nmcli connection modify MyHostspot ipv4.method shared");

    system("sudo nmcli connection up MyHostspot");

    // 소켓 생성
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // 소켓 옵션 설정
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1)
    {
        perror("setsockopt SO_REUSEADDR");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

#ifdef SO_REUSEPORT
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt)) == -1)
    {
        perror("setsockopt SO_REUSEPORT");
        close(server_fd);
        exit(EXIT_FAILURE);
    }
#endif

    // 주소와 포트 설정
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // 소켓을 지정된 주소에 바인딩
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
    {
        perror("bind failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // 연결 요청 수신 대기 상태로 설정
    if (listen(server_fd, 3) < 0)
    {
        perror("listen");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("Server is listening on port %d\n", PORT);

    while (1)
    {
        // 클라이언트 연결 요청 수락
        new_socket = accept(server_fd, (struct sockaddr *)&address, &addrlen);
        if (new_socket < 0)
        {
            perror("accept");
            close(server_fd);
            exit(EXIT_FAILURE);
        }

        // 연결된 클라이언트의 IP 주소와 포트 번호 출력
        printf("Connection accepted from %s:%d\n",
               inet_ntoa(address.sin_addr), ntohs(address.sin_port));

        // 요청 읽기
        int valread = read(new_socket, buffer, BUFFER_SIZE);
        if (valread < 0)
        {
            perror("read");
            close(new_socket);
            continue;
        }
        buffer[valread] = '\0';
        printf("Received request:\n%s\n", buffer);

        // 요청이 HTML 폼을 위한 것인지 확인
        if (strstr(buffer, "GET /form.html") != NULL)
        {
            // HTML 폼 전송
            serve_html(new_socket);
        }
        else if (strstr(buffer, "POST /save") != NULL)
        {
            // POST /save 요청 처리
            handle_post_request(new_socket, buffer);
        }
        else
        {
            // 다른 요청에 대한 기본 응답
            const char *not_found_response = "HTTP/1.1 404 Not Found\r\n"
                                             "Content-Type: text/html\r\n\r\n"
                                             "<html><body><h1>404 Not Found</h1></body></html>";
            send(new_socket, not_found_response, strlen(not_found_response), 0);
        }

        close(new_socket);
    }

    close(server_fd);
    return 0;
}

void serve_html(int client_socket)
{
    FILE *html_file = fopen("form.html", "r");
    if (html_file == NULL)
    {
        perror("Error opening HTML file");
        exit(EXIT_FAILURE);
    }

    char line[BUFFER_SIZE];

    // HTTP 응답 헤더 전송
    const char *header = "HTTP/1.1 200 OK\r\n"
                         "Content-Type: text/html\r\n\r\n";
    send(client_socket, header, strlen(header), 0);

    // HTML 파일 내용 전송
    while (fgets(line, sizeof(line), html_file) != NULL)
    {
        send(client_socket, line, strlen(line), 0);
    }

    fclose(html_file);
}

void handle_post_request(int client_socket, char *buffer)
{
    char *success_response = "HTTP/1.1 200 OK\nContent-Type: text/html\n\n<html><body><h1>Wi-Fi configuration saved successfully!</h1></body></html>\n";
    char *error_response = "HTTP/1.1 400 Bad Request\nContent-Type: text/html\n\n<html><body><h1>Error: Unable to save Wi-Fi configuration!</h1></body></html>\n";

    // Extract JSON data from the request body
    char *json_start = strstr(buffer, "\r\n\r\n");
    if (json_start != NULL)
    {
        json_start += 4; // Skip past the "\r\n\r\n" to get to the body

        printf("Received JSON data:\n%s\n", json_start);

        char *ssid = NULL;
        char *password = NULL;

        // Extract keys and values from JSON data
        char *token = strtok(json_start, ",{}\":");
        while (token != NULL)
        {
            if (strcmp(token, "ssid") == 0)
            {
                token = strtok(NULL, ",{}\":");
                ssid = strdup(token);
            }
            else if (strcmp(token, "password") == 0)
            {
                token = strtok(NULL, ",{}\":");
                password = strdup(token);
            }
            else
            {
                token = strtok(NULL, ",{}\":");
            }
        }

        if (ssid != NULL && password != NULL)
        {
            // Use nmcli to configure Wi-Fi
            char command[BUFFER_SIZE];
            snprintf(command, sizeof(command), "nmcli dev wifi connect \"%s\" password \"%s\"", ssid, password);
            int result = system(command);

            free(ssid);
            free(password);

            if (result == 0)
            {
                // Send success response
                send(client_socket, success_response, strlen(success_response), 0);
                printf("Wi-Fi configuration saved and applied successfully.\n");
            }
            else
            {
                // Failed to configure Wi-Fi
                send(client_socket, error_response, strlen(error_response), 0);
                printf("Failed to apply Wi-Fi configuration.\n");
            }
        }
        else
        {
            // Missing ssid or password in JSON data
            send(client_socket, error_response, strlen(error_response), 0);
            printf("Missing ssid or password in JSON data.\n");
        }
    }
    else
    {
        // Invalid request format
        send(client_socket, error_response, strlen(error_response), 0);
        printf("Invalid request format.\n");
    }
}
