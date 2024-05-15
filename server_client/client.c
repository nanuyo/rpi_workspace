#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 8080
#define BUFFER_SIZE 1024
#define SERVER_ADDRESS "59.11.52.50"

int main() {
    int clientSocket;
    struct sockaddr_in serverAddr;
    char buffer[BUFFER_SIZE];

    // 소켓 생성
    if ((clientSocket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("Socket creation error.\n");
        return 1;
    }

    memset(&serverAddr, '0', sizeof(serverAddr));

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);

    // 서버 주소 설정
    if (inet_pton(AF_INET,  SERVER_ADDRESS , &serverAddr.sin_addr) <= 0) {
        printf("Invalid address/ Address not supported.\n");
        return 1;
    }

    // 서버에 연결
    if (connect(clientSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) {
        printf("Connection failed.\n");
        return 1;
    }

    printf("Connected to server.\n");

    // 사용자로부터 메시지 입력 및 서버로 전송
    while (1) {
        printf("Enter message: ");
        fgets(buffer, BUFFER_SIZE, stdin);

        send(clientSocket, buffer, strlen(buffer), 0);

        // 서버로부터 응답 수신 및 출력
        int bytesReceived = recv(clientSocket, buffer, BUFFER_SIZE, 0);
        if (bytesReceived <= 0) {
            printf("Server disconnected.\n");
            break;
        }
        buffer[bytesReceived] = '\0';
        printf("Server response: %s", buffer);
    }

    close(clientSocket);

    return 0;
}
