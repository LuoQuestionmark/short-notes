#include "config.h"
#include <assert.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

static bool sanitize_string(const char *str) {
    char c;
    for (size_t i = 0; i < strlen(str); i++) {
        c = str[i];
        if (c == '-' || c == '_' || c == '.' || c == ' ') continue;
        if (c >= 'A' && c <= 'z') continue;
        if (c >= '0' && c <= '9') continue;

        // otherwise unacceptable
        return false;
    }

    return true;
}

static bool is_suffix_txt(const char *str) {
    if (strlen(str) < 4) return false;
    return !strncmp(str + (strlen(str) - 4), ".txt", 4);
}

static void remove_redundant_space(char *str) {
    char temp[128]          = { 0 };
    bool last_char_is_space = false;

    for (int i = 0, j = 0; j < sizeof(temp); i++) {
        if (str[i] == ' ') {
            if (!last_char_is_space) {
                last_char_is_space = true;
            } else {
                continue;
            }
        } else {
            last_char_is_space = false;
        }

        temp[j++] = str[i];
        if (str[i] == '\0') {
            break;
        }
    }

    memcpy(str, temp, strlen(temp) + 1);
}

static char **prepare_execv_command(const char *editor,
                                    const char *title_buffer) {
    char **execv_strs = calloc(4, sizeof(char *));

    if (strncmp(editor, "vim", strlen("vim")) == 0) {
        execv_strs[0] = strdup("vim");
        execv_strs[1] = strdup("+$");
        execv_strs[2] = strdup(title_buffer);
    } else if (strncmp(editor, "vi", strlen("vi")) == 0) {
        execv_strs[0] = strdup("vi");
        execv_strs[1] = strdup("+$");
        execv_strs[2] = strdup(title_buffer);
    } else if (strncmp(editor, "nano", strlen("nano")) == 0) {
        execv_strs[0] = strdup("nano");
        execv_strs[1] = strdup("+999999");
        execv_strs[2] = strdup(title_buffer);
    } else if (strncmp(editor, "emacs", strlen("emacs")) == 0) {
        execv_strs[0] = strdup("emacs");
        execv_strs[1] = strdup("+999999");
        execv_strs[2] = strdup(title_buffer);
    } else {
        execv_strs[0] = strdup(editor);
        execv_strs[1] = strdup(title_buffer);
    }

    return execv_strs;
}

int add_note() {
    bool success = true;

    size_t title_len = 0;
    char *title      = NULL;

    size_t tags_len = 0;
    char *tags      = NULL;

    char filename_buffer[128] = { 0 };
    char editor_buffer[128]   = { 0 };

    // add repo dir if available
    if (get_note_repo(filename_buffer, sizeof(filename_buffer)) == false) {
        puts("no config file found, save file locally");
    }

    // input title
    printf("title: ");
    getline(&title, &title_len, stdin);
    if (!title) {
        perror("getline");
        success = false;
        goto clean_buffers;
    }
    title[strlen(title) - 1] = 0; // remove \n at the end

    // check string
    if (!sanitize_string(title)) {
        char err_msg[128] = { 0 };
        snprintf(err_msg, sizeof(err_msg) - 1, "sanitize_string \"%s\"", title);
        errno = EINVAL;
        perror(err_msg);
        success = false;
        goto clean_buffers;
    }

    // add .txt suffix
    strncat(filename_buffer, title,
            sizeof(filename_buffer) - strlen(filename_buffer) - 1);
    if (!is_suffix_txt(title)) {
        strcat(filename_buffer, ".txt");
    }

    // check if file exists
    if (access(filename_buffer, F_OK) == 0) {
        fprintf(stderr, "a file with the same title \"%s\" exists, stop\n",
                filename_buffer);
        perror("access");
        success = false;
        goto clean_buffers;
    }

    // input tag
    printf("tag (separate by space): ");
    getline(&tags, &tags_len, stdin);
    if (!tags) {
        perror("getline");
        success = false;
        goto clean_buffers;
    }
    tags[strlen(tags) - 1] = 0; // remove \n at the end
    remove_redundant_space(tags);

    // get datetime string
    time_t current_time;
    time(&current_time);

    printf("(press anykey to edit content)");
    getchar();

    // add content
    char *editor;
    if ((editor = getenv("EDITOR")) != NULL) {
        strncpy(editor_buffer, editor, sizeof(editor_buffer) - 1);
    } else {
        editor = "nano";
    }

    int pid;
    int child_status;
    if ((pid = fork()) == 0) {
        // child

        // prepare file with tags
        FILE *note = fopen(filename_buffer, "a+");
        if (note == NULL) {
            exit(EXIT_FAILURE);
        }
        fprintf(note, "title: %s\n", title);
        fprintf(note, "tags: %s\n", tags);
        fprintf(note, "date: %s", ctime(&current_time));
        fputs("----", note);
        fclose(note);

        // open editor
        char **params = prepare_execv_command(editor, filename_buffer);
        execvp(params[0], params);

        // execvp fails, clean the string list allocated to exec command
        perror("execvp");
        for (int i = 0; i < 4; i++) {
            if (params[i]) {
                free(params[i]);
            }
        }
        exit(EXIT_FAILURE);

    } else {
        // parent
        if (pid == -1) {
            perror("fork");
            success = false;
            goto clean_buffers;
        }
        wait(&child_status);
        if (WEXITSTATUS(child_status) == EXIT_SUCCESS) {
            printf("note added as %s\n", filename_buffer);
        }
    }

clean_buffers:
    if (title != NULL) {
        free(title);
    }
    if (tags != NULL) {
        free(tags);
    }

    return 0;
}
