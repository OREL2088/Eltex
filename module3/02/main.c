#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pubsub.h"

static void print_usage(const char *program)
{
    fprintf(stderr,
            "Usage:\n"
            "  %s -b\n"
            "  %s -p <topic>\n"
            "  %s -s <topic> [topic ...]\n",
            program, program, program);
}

int main(int argc, char **argv)
{
    if (install_signal_handler() == -1) {
        perror("sigaction");
        return EXIT_FAILURE;
    }
    if (argc == 2 && strcmp(argv[1], "-b") == 0)
        return run_broker();
    if (argc == 3 && strcmp(argv[1], "-p") == 0)
        return run_publisher(argv[2]);
    if (argc >= 3 && strcmp(argv[1], "-s") == 0)
        return run_subscriber(argc - 2, argv + 2);

    print_usage(argv[0]);
    return EXIT_FAILURE;
}
