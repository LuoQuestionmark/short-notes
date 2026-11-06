#include "add_note.h"
#include "config.h"
#include "search_note.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static void print_help() {
    puts("short_notes: a simple program to add and search short notes");
    puts("usage:");
    puts("\tshort_notes [add|a]:\tadd notes");
    puts("\tshort_notes [search|s] keyword:\tsearch notes with keyword");
    puts("\tshort_notes [search|s] --title kw:\tsearch notes with keyword in "
         "title");
    puts("\tshort_notes [search|s] --tag kw:\tsearch notes with keyword "
         "in tag");
    puts("\tshort_notes [search|s] --content kw:\tsearch notes with "
         "keyword in content");
    puts("\tshort_notes config set <dir>:\t set notes repository to the "
         "given directory");
}

int main(int argc, char **argv) {
    int opt;

    bool adding_note    = false;
    bool searching_note = false;
    bool config_repo    = false;

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

    if (strncmp(argv[1], "config", sizeof("config")) == 0) {
        config_repo = true;
    }
    if (strncmp(argv[1], "c", sizeof("c")) == 0) {
        config_repo = true;
    }
    // postcondition: adding_note && searching_note = false

    if (!adding_note && !searching_note && !config_repo) {
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

    if (searching_note && searching_flag != SN_SEARCH_ALL) {
        if (argc < 4) {
            fputs("cannot search note since no keyword is given\n", stderr);
            exit(EXIT_FAILURE);
        }
        strncpy(searching_str, argv[3], sizeof(searching_str));
    }

    if (searching_note && searching_flag == SN_SEARCH_ALL) {
        strncpy(searching_str, argv[2], sizeof(searching_str));
    }

    if (searching_note) {
        char target_dir[512] = { 0 };

        if (get_note_repo(target_dir, sizeof(target_dir)) == false) {
            puts("no config file found, search file locally");
            getcwd(target_dir, sizeof(target_dir));
        } else {
            printf("config file found at %s\n", target_dir);
        }

        search_result_t *search_result = new_seach_result();
        search_note(searching_str, target_dir, search_result);
        search_filter(searching_flag, searching_str, search_result);

        search_result_print(search_result);
        search_result_free(search_result);
    }

    if (config_repo && argc != 4) {
        fputs("config repo with wrong number of arguments\n", stderr);
        exit(EXIT_FAILURE);
    }
    if (config_repo) {
        if (strncmp(argv[2], "set", strlen("set")) != 0) {
            fprintf(stderr, "unrecognized config command %s\n", argv[2]);
            exit(EXIT_FAILURE);
        }
    }
    if (config_repo) {
        if (set_note_repo(argv[3]) == false) {
            fprintf(stderr, "cannot set note repo at %s\n", argv[3]);
            exit(EXIT_FAILURE);
        }
    }

    return 0;
}
