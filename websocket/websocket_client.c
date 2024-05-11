#include <libwebsockets.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_FRAME_SIZE (10 * 1024 * 1024) // Maximum frame size for JPG file

static int callback(struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len);

static struct lws_protocols protocols[] = {
    {"binary", callback, 0, MAX_FRAME_SIZE},
    {NULL, NULL, 0, 0}
};

static const struct lws_extension exts[] = {
    {NULL, NULL, NULL} // End of extensions list
};

static int interrupted = 0;

static int callback(struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len) {
    switch (reason) {
        case LWS_CALLBACK_CLIENT_ESTABLISHED:
            printf("WebSocket connection established.\n");
            // Send the JPG file here
            FILE *file = fopen("image.jpg", "rb");
            if (file == NULL) {
                printf("Failed to open JPG file.\n");
                lws_close_reason(wsi, LWS_CLOSE_STATUS_NOSTATUS, NULL, 0);
                return -1;
            }
            fseek(file, 0, SEEK_END);
            long file_size = ftell(file);
            fseek(file, 0, SEEK_SET);

            unsigned char buf[1024];
            size_t n;
            while ((n = fread(buf, 1, sizeof(buf), file)) > 0) {
                lws_write(wsi, buf, n, LWS_WRITE_BINARY);
            }
            fclose(file);
            break;
        case LWS_CALLBACK_CLOSED:
            printf("WebSocket connection closed.\n");
            interrupted = 1;
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

    info.port = CONTEXT_PORT_NO_LISTEN; // This client does not listen for incoming connections
    info.protocols = protocols;
    info.extensions = exts;

    context = lws_create_context(&info);
    if (context == NULL) {
        printf("Failed to create LWS context.\n");
        return -1;
    }

    // Create the WebSocket client connection
    struct lws_client_connect_info client_info;
    memset(&client_info, 0, sizeof(client_info));
    client_info.context = context;
    client_info.address = "172.30.1.21"; // WebSocket server address
    client_info.port = 3000; // WebSocket server port
    client_info.path = "/";
    client_info.host = client_info.address;
    client_info.origin = client_info.address;
    client_info.protocol = protocols[0].name;

    struct lws *wsi = lws_client_connect_via_info(&client_info);
    if (wsi == NULL) {
        printf("Failed to connect to WebSocket server.\n");
        lws_context_destroy(context);
        return -1;
    }

    // Enter the event loop
    while (!interrupted) {
        lws_service(context, 50); // Process WebSocket events with a timeout of 50 ms
    }

    lws_context_destroy(context);

    return 0;
}
