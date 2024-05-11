#include <libwebsockets.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_FRAME_SIZE (10 * 1024 * 1024) // Maximum frame size for JPG file

static int callback_http(struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len);
static int callback_websocket(struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len);

static struct lws_protocols protocols[] = {
    {"http", callback_http, 0, 0},
    {"websocket", callback_websocket, 0, MAX_FRAME_SIZE},
    {NULL, NULL, 0, 0}
};

static const struct lws_extension exts[] = {
    {NULL, NULL, NULL} // End of extensions list
};

static int interrupted = 0;

static int callback_http(struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len) {
    switch (reason) {
        default:
            break;
    }

    return 0;
}

static int callback_websocket(struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len) {
    switch (reason) {
        case LWS_CALLBACK_ESTABLISHED:
            printf("WebSocket connection established.\n");
            break;
        case LWS_CALLBACK_RECEIVE:
            // Handle incoming messages here
            break;
        case LWS_CALLBACK_CLOSED:
            printf("WebSocket connection closed.\n");
            break;
        default:
            break;
    }

    return 0;
}

int main() {
    struct lws_context *context;
    struct lws_context_creation_info info;
    memset(&info, 0, sizeof(info));

    info.port = 3000/*CONTEXT_PORT_NO_LISTEN*/; // This server does not listen for HTTP connections
    info.protocols = protocols;
    info.extensions = exts;

    context = lws_create_context(&info);
    if (context == NULL) {
        printf("Failed to create LWS context.\n");
        return -1;
    }

    // Enter the event loop
    while (!interrupted) {
        lws_service(context, 50); // Process WebSocket events with a timeout of 50 ms
    }

    lws_context_destroy(context);

    return 0;
}
