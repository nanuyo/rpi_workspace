#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define PORT 8080
#define BUFFER_SIZE 1024

int main() {
    int serverSocket, clientSocket;
    struct sockaddr_in serverAddr, clientAddr;
    socklen_t addrLen = sizeof(serverAddr);
    char buffer[BUFFER_SIZE];

    // 소켓 생성
    if ((serverSocket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("Socket creation error.\n");
        return 1;
    }

    memset(&serverAddr, '0', sizeof(serverAddr));

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serverAddr.sin_port = htons(PORT);

    // 소켓을 주소에 바인딩
    if (bind(serverSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) {
        printf("Binding failed.\n");
        return 1;
    }

    // 연결 요청 대기
    if (listen(serverSocket, 10) < 0) {
        printf("Listen failed.\n");
        return 1;
    }

    printf("Server listening on port %d...\n", PORT);

    // 클라이언트 연결 수락
    if ((clientSocket = accept(serverSocket, (struct sockaddr *)&clientAddr, &addrLen)) < 0) {
        printf("Accept failed.\n");
        return 1;
    }

    printf("Client connected.\n");

    // 클라이언트로부터 메시지 수신 및 에코
    while (1) {
        int bytesReceived = recv(clientSocket, buffer, BUFFER_SIZE, 0);
        if (bytesReceived <= 0) {
            printf("Client disconnected.\n");
            break;
        }
        buffer[bytesReceived] = '\0';
        printf("Received: %s", buffer);
        send(clientSocket, buffer, strlen(buffer), 0);
    }

    close(serverSocket);
    close(clientSocket);

    return 0;
}
