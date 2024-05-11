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
        case LWS_CALLBACK_RECEIVE:
            // Handle received binary data
            if (len > 0) {
                unsigned char *data = (unsigned char *)in;
                // Write received data to a file (e.g., image.jpg)
                FILE *file = fopen("received_image.jpg", "ab");
                if (file == NULL) {
                    printf("Failed to open file for writing.\n");
                    return -1;
                }
                fwrite(data, 1, len, file);
                fclose(file);
            }
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

    info.port = 3000; // WebSocket server port
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
