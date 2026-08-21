#include "add_note.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static void print_help() {
    // TODO
    puts("help msg");
}

int main(int argc, char **argv) {
    int opt;

    bool adding_note     = false;
    char note_title[256] = { 0 };

    while ((opt = getopt(argc, argv, "a:h")) != -1) {
        switch (opt) {
        case 'h':
            print_help();
            exit(EXIT_SUCCESS);
        case 'a':
            adding_note = true;
            strncpy(note_title, optarg, sizeof(note_title) - 1);
        default:
            break;
        }
    }

    if (adding_note) {
        add_note(note_title);
    }

    return 0;
}
