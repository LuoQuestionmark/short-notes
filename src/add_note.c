#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

static bool sanitize_string(const char *str) {
    char err_msg[128] = { 0 };

    char c;
    for (size_t i = 0; i < strlen(str); i++) {
        c = str[i];
        if (c == '-' || c == '_' || c == '.') continue;
        if (str[i] >= 'A' && str[i] <= 'z') continue;

        // otherwise unacceptable
        snprintf(err_msg, strlen(err_msg),
                 "invalid string \"%s\" containing char \"%c\"", str, c);
        errno = EINVAL;
        perror(err_msg);

        return false;
    }

    return true;
}

static bool is_suffix_txt(const char *str) {
    if (strlen(str) < 4) return false;
    return !strncmp(str + (strlen(str) - 4), ".txt", 4);
}

int add_note(const char *title) {
    char title_buffer[128], editor_buffer[128];

    if (!sanitize_string(title)) {
        return -1;
    }

    strcpy(title_buffer, title);
    if (!is_suffix_txt(title)) {
        strcat(title_buffer, ".txt");
    }

    printf("adding note with title %s...\n", title_buffer);

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
        char *params[] = { editor, title_buffer, NULL };
        execvp(params[0], params);
    } else {
        wait(&child_status);
        if (WEXITSTATUS(child_status) == EXIT_SUCCESS) {
            puts("note added");
        }
    }

    return 0;
}
