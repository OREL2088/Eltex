#include <stdio.h>
#include <stdlib.h>

#include "chat.h"

int main(int argc, char *argv[])
{
    struct chat chat;
    int result;

    if (argc != 2) {
        fprintf(stderr, "Usage: %s queue_name\n", argv[0]);
        return EXIT_FAILURE;
    }
    if (install_signal_handlers() == -1) {
        perror("sigaction");
        return EXIT_FAILURE;
    }
    if (open_chat(&chat, argv[1]) == -1)
        return EXIT_FAILURE;

    printf("Connected as %s. Press Ctrl+C to exit.\n",
           chat.owner ? "queue owner" : "peer");
    result = run_chat(&chat);
    close_chat(&chat);
    return result == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
