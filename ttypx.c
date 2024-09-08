#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <signal.h>

int main(int argc, char **argv) {
    if (argc > 1 && (strcmp(argv[1], "--help") == 0 || strcmp(argv[1], "-h") == 0)) {
        printf("%s - query pixel size of terminal\n", argv[0]);
        puts("Copyright ⓒ 2024 Michael Homer");
        printf("Usage: %s [-g]\n", argv[0]);
        puts("\nOptions:");
        puts("    -g, --geom    Output in geometry format WIDTHxHEIGHT");
        puts("    -h, --help    This help text");
        return 0;
    }
    signal(SIGTTOU, SIG_IGN);
    struct termios term, restore;
    char buf[64];
    // Find or open TTY
    int fd = -1;
    if (isatty(0)) fd = 0;
    else if (isatty(1)) fd = 1;
    else if (isatty(2)) fd = 2;
    else fd = open("/dev/tty", O_RDWR);
    if (fd < 0) {
        fprintf(stderr, "%s: Error: Could not find tty", argv[0]);
        return 1;
    }
    // Set terminal state
    int r = tcgetattr(fd, &term);
    r = tcgetattr(fd, &restore);
    term.c_lflag &= ~(ICANON|ECHO);
    tcsetattr(fd, TCSANOW, &term);
    // Write window query
    write(fd, "\033[14t", 5);
    // Read CSI response
    int read_bytes = read(fd, buf, 63);
    // Restore terminal state
    tcsetattr(fd, TCSANOW, &restore);
    if (read_bytes < 0) {
        fprintf(stderr, "%s: Error: Could not read tty response", argv[0]);
        return 1;
    }
    buf[read_bytes] = 0;
    // buf is expected to hold something like \e[4;600;1200t
    if (buf[0] != '\033' || buf[1] != '[' || buf[2] != '4' || buf[3] != ';') {
        fprintf(stderr, "%s: Error: Unexpected CSI response", argv[0]);
        return 1;
    }
    // Read height & width out of buffer
    int i = 4;
    int height = 0, width = 0;
    for (; buf[i] != ';' && i < read_bytes; i++) {
        height = height * 10;
        height = height + (buf[i] - '0');
    }
    i++;
    for (; buf[i] != 't' && i < read_bytes; i++) {
        width = width * 10;
        width = width + (buf[i] - '0');
    }
    // Display dimensions
    if (argc > 1 && (strcmp(argv[1], "-g") == 0 || strcmp(argv[1], "--geom") == 0)) {
        printf("%ix%i\n", width, height);
    } else {
        printf("%i %i\n", width, height);
    }
    return 0;
}
