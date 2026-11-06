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
    int searching_flag      = 0;

    if (argc == 1) {
        print_help();
        exit(EXIT_SUCCESS);
    }

    // precondition: argc > 1
    if ((strncmp(argv[1], "help", sizeof("help")) == 0) ||
        (strncmp(argv[1], "h", sizeof("h")) == 0)) {
        print_help();
        exit(EXIT_SUCCESS);
    }

    if (strncmp(argv[1], "add", sizeof("add")) == 0) {
        adding_note = true;
    }
    if (strncmp(argv[1], "a", sizeof("a")) == 0) {
        adding_note = true;
    }

    if (strncmp(argv[1], "search", sizeof("search")) == 0) {
        searching_note = true;
    }
    if (strncmp(argv[1], "s", sizeof("s")) == 0) {
        searching_note = true;
    }
    // postcondition: adding_note && searching_note = false

    if (!adding_note && !searching_note) {
        fprintf(stderr, "unrecognized option %s\n", argv[1]);
        exit(EXIT_FAILURE);
    }

    if (adding_note) {
        add_note();
        exit(EXIT_SUCCESS);
    }

    if (searching_note && argc <= 2) {
        fputs("cannot search note since no keyword is given\n", stderr);
        exit(EXIT_FAILURE);
    }
    // postcondition: searching_note && argc >= 3

    if (searching_note) {
        if (strncmp(argv[2], "--title", sizeof("--title")) == 0) {
            searching_flag = SN_SEARCH_TITLE;
        } else if (strncmp(argv[2], "--tag", sizeof("--tag")) == 0) {
            searching_flag = SN_SEARCH_TAG;
        } else if (strncmp(argv[2], "--content", sizeof("--content")) == 0) {
            searching_flag = SN_SEARCH_CONTENT;
        } else {
            searching_flag = SN_SEARCH_ALL;
        }
    }
    // postcondition: searching_flag is set

    if (searching_flag != SN_SEARCH_ALL) {
        if (argc < 4) {
            fputs("cannot search note since no keyword is given\n", stderr);
            exit(EXIT_FAILURE);
        }
        strncpy(searching_str, argv[3], sizeof(searching_str));
    } else {
        strncpy(searching_str, argv[2], sizeof(searching_str));
    }
    // postcondition: searching_str is set

    if (searching_note) {
        char current_dir[512] = { 0 };
        getcwd(current_dir, sizeof(current_dir));

        search_result_t *search_result = new_seach_result();
        search_note(searching_str, current_dir, search_result);
        search_filter(searching_flag, searching_str, search_result);

        search_result_print(search_result);
        search_result_free(search_result);
    }

    return 0;
}
