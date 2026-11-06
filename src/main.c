#include "add_note.h"
#include "search_note.h"
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

    bool adding_note = false;

    bool searching_note     = false;
    char searching_str[256] = { 0 };

    // TODO: check long options s.t. --title --tag --content can be precised
    while ((opt = getopt(argc, argv, "ahs:")) != -1) {
        switch (opt) {
        case 'h':
            print_help();
            exit(EXIT_SUCCESS);
        case 'a':
            adding_note = true;
            break;
        case 's':
            searching_note = true;
            strncpy(searching_str, optarg, sizeof(searching_str) - 1);
            break;
        default:
            break;
        }
    }

    if (adding_note && searching_note) {
        fputs("cannot add and search note at the same time", stderr);
        return -1;
    }

    if (adding_note) {
        add_note();
    }

    if (searching_note) {
        char current_dir[512] = { 0 };
        getcwd(current_dir, sizeof(current_dir));

        search_result_t *search_result = new_seach_result();
        search_note(searching_str, current_dir, search_result);
        search_filter(SN_SEARCH_TITLE, searching_str, search_result);

        search_result_print(search_result);
        search_result_free(search_result);
    }

    return 0;
}
