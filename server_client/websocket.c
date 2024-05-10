#include <stdio.h>
#include <stdlib.h>
#include <libwebsockets.h>

// WebSocket client connection context
struct connection_data {
    struct lws_context *context;
    struct lws *wsi;
};

// Callback function for handling WebSocket protocol
static int websocket_callback(struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len) {
    switch (reason) {
        case LWS_CALLBACK_CLIENT_ESTABLISHED:
            printf("Connected to WebSocket server\n");
            break;
        case LWS_CALLBACK_CLIENT_WRITEABLE:
            // You can send data here
            break;
        case LWS_CALLBACK_CLOSED:
            printf("Connection closed\n");
            lws_cancel_service(lws_get_context(wsi));
            break;
        default:
            break;
    }
    return 0;
}

// Main function to establish WebSocket connection
int main() {
    struct lws_context_creation_info info;
    struct lws_client_connect_info connect_info;
    struct lws_context *context;
    struct connection_data conn_data;

    // Initialize libwebsockets context creation info
    memset(&info, 0, sizeof(info));
    info.port = CONTEXT_PORT_NO_LISTEN;
    info.protocols = lws_get_client_protocol();
    info.gid = -1;
    info.uid = -1;

    // Create libwebsockets context
    context = lws_create_context(&info);
    if (!context) {
        fprintf(stderr, "Failed to create libwebsocket context\n");
        return 1;
    }

    // Initialize connection information
    memset(&connect_info, 0, sizeof(connect_info));
    connect_info.context = context;
    connect_info.address = "172.30.1.22";
    connect_info.port = 3000; // Or the port of your WebSocket server
    connect_info.path = "/"; // Path of your WebSocket endpoint
    connect_info.host = connect_info.address;
    connect_info.origin = connect_info.address;
    connect_info.protocol = "ws";

    // Connect to WebSocket server
    conn_data.context = context;
    conn_data.wsi = lws_client_connect_via_info(&connect_info);
    if (!conn_data.wsi) {
        fprintf(stderr, "Failed to connect to server\n");
        lws_context_destroy(context);
        return 1;
    }

    // Main event loop
    while (lws_service(context, 0) >= 0 && !lws_cancel_service(context)) {}

    // Clean up
    lws_context_destroy(context);

    return 0;
}
