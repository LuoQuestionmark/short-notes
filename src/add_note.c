#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

static bool sanitize_string(const char *str) {
    char c;
    for (size_t i = 0; i < strlen(str); i++) {
        c = str[i];
        if (c == '-' || c == '_' || c == '.' || c == ' ') continue;
        if (str[i] >= 'A' && str[i] <= 'z') continue;

        // otherwise unacceptable
        return false;
    }

    return true;
}

static bool is_suffix_txt(const char *str) {
    if (strlen(str) < 4) return false;
    return !strncmp(str + (strlen(str) - 4), ".txt", 4);
}

int add_note() {
    bool success = true;

    size_t title_len = 0;
    char *title      = NULL;

    size_t tags_len = 0;
    char *tags      = NULL;

    char title_buffer[128]  = { 0 };
    char editor_buffer[128] = { 0 };

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
        snprintf(err_msg, strlen(err_msg), "sanitize_string \"%s\"", title);
        errno = EINVAL;
        perror(err_msg);
        success = false;
        goto clean_buffers;
    }

    // add .txt suffix
    strncpy(title_buffer, title, sizeof(title_buffer) - 1);
    if (!is_suffix_txt(title)) {
        strcat(title_buffer, ".txt");
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

    printf("(press anykey to edit content)");
    getchar();

    // add content
    char *editor;
    if ((editor = getenv("EDITOR")) != NULL) {
        strncpy(editor_buffer, editor, sizeof(editor_buffer) - 1);
    } else {
        editor = "vi";
    }

    int pid;
    int child_status;
    if ((pid = fork()) == 0) {
        // child

        // prepare file with tags
        FILE *note = fopen(title_buffer, "a+");
        if (note == NULL) {
            exit(EXIT_FAILURE);
        }
        fprintf(note, "tags: %s\n", tags);
        fclose(note);

        // open editor
        char *params[] = { editor, title_buffer, NULL };
        execvp(params[0], params);
    } else {
        // parent
        if (pid == -1) {
            perror("fork");
            success = false;
            goto clean_buffers;
        }
        wait(&child_status);
        if (WEXITSTATUS(child_status) == EXIT_SUCCESS) {
            puts("note added");
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
