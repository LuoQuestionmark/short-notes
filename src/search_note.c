#include "search_note.h"
#include <assert.h>
#include <dirent.h>
#include <regex.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#define FILE_CONTENT_BUFFER_SIZE 65536

static bool file_is_txt(const char *filename) {
    regex_t preg;
    regcomp(&preg, ".*txt", REG_NOSUB);
    int match = regexec(&preg, filename, 0, NULL, 0);
    regfree(&preg);

    return (match == 0);
}

static bool file_is_note(const char *filecontent) {
    regex_t preg;
    regcomp(&preg, "title:.*\ntags:.*\n", REG_NOSUB | REG_NEWLINE);
    int match = regexec(&preg, filecontent, 0, NULL, 0);
    regfree(&preg);

    return (match == 0);
}

static bool file_contains_keyword(const char *filecontent,
                                  const char *keyword) {
    char pattern[256] = { 0 };
    snprintf(pattern, sizeof(pattern) - 1, ".*%s.*", keyword);

    regex_t preg;
    regcomp(&preg, pattern, REG_NOSUB);
    int match = regexec(&preg, filecontent, 0, NULL, 0);
    regfree(&preg);

    return (match == 0);
}

static bool note_contains_title(const char *note, const char *title) {
    char pattern[256] = { 0 };
    snprintf(pattern, sizeof(pattern) - 1, "title:.*%s.*\n", title);
    regex_t preg;
    regcomp(&preg, pattern, REG_NOSUB | REG_NEWLINE);
    int match = regexec(&preg, note, 0, NULL, 0);
    regfree(&preg);

    return (match == 0);
}

static bool note_contains_tag(const char *note, const char *tag) {
    char pattern[256] = { 0 };
    // tag should be separated by space
    snprintf(pattern, sizeof(pattern) - 1, "tags:.* %s[ \n].*", tag);

    regex_t preg;
    regcomp(&preg, pattern, REG_NOSUB | REG_NEWLINE);
    int match = regexec(&preg, note, 0, NULL, 0);
    regfree(&preg);

    return (match == 0);
}

static bool note_contains_content(const char *note, const char *content) {
    char pattern[256] = { 0 };
    // tag should be separated by space
    snprintf(pattern, sizeof(pattern) - 1, "----\n.*%s", content);

    regex_t preg;
    regcomp(&preg, pattern, REG_NOSUB | REG_NEWLINE);
    int match = regexec(&preg, note, 0, NULL, 0);
    regfree(&preg);

    return (match == 0);
}

search_result_t *new_seach_result() {
    search_result_t *ret = calloc(1, sizeof(search_result_t));
    ret->count           = 0;
    ret->data            = calloc(1, sizeof(char *));
    ret->size            = 1;

    return ret;
}

void search_result_free(search_result_t *result) {
    if (result == NULL) return;

    for (int i = 0; i < result->count; i++) {
        if (result->data[i] != NULL) {
            free(result->data[i]);
        }
    }

    free(result->data);
    free(result);
}

void search_result_add_content(search_result_t *self, const char *content) {
    assert(self);
    assert(self->size > 0);

    if (self->count == self->size) {
        // list full, double the size
        char **realloc_cells =
            realloc(self->data, 2 * self->size * sizeof(char *));
        if (realloc_cells == NULL) {
            perror("realloc");
            exit(EXIT_FAILURE);
        }
        self->data = realloc_cells;
    }

    self->data[self->count++] = strdup(content);
}

void search_result_print(const search_result_t *result) {
    if (result == NULL) return;

    if (result->count - result->hidden == 0) {
        puts("no matched result");
        return;
    }

    printf("total %d results found, listed here:\n",
           result->count - result->hidden);

    for (int i = 0; i < result->count; i++) {
        if (result->data[i] == NULL) continue;

        puts("--------");
        puts(result->data[i]);
    }
}

int search_note(const char *keyword, const char *dir, search_result_t *result) {
    assert(keyword);
    assert(dir);

    DIR *current_dir = opendir(dir);
    if (current_dir == NULL) {
        perror("opendir");
        return -1;
    }
    struct dirent *dirent;

    chdir(dir); // TODO: verify if this needed to be switched back

    char *file_content_buffer = calloc(FILE_CONTENT_BUFFER_SIZE, sizeof(char));
    if (file_content_buffer == NULL) {
        perror("calloc");
        goto free_dir;
    }

    while ((dirent = readdir(current_dir)) != NULL) {
        if (dirent->d_type != DT_REG) continue;     // only check regular files
        if (!file_is_txt(dirent->d_name)) continue; // check suffix txt

        FILE *file = fopen(dirent->d_name, "r");
        if (file == NULL) {
            perror("fopen");
            goto free_buffer;
        }

        memset(file_content_buffer, 0, FILE_CONTENT_BUFFER_SIZE);
        if (fread(file_content_buffer, sizeof(char),
                  FILE_CONTENT_BUFFER_SIZE - 1, file) == 0) {
            perror("fread");
            fclose(file);
            continue;
        }

        if (!file_is_note(file_content_buffer)) {
            fclose(file);
            continue;
        }

        if (file_contains_keyword(file_content_buffer, keyword)) {
            search_result_add_content(result, file_content_buffer);
        }

        fclose(file);
    }

free_buffer:
    free(file_content_buffer);

free_dir:
    closedir(current_dir);
    return 0;
}

int search_filter(enum SN_SEARCH_RANGE option, const char *keyword,
                  search_result_t *result) {
    bool (*predicate)(const char *note, const char *keyword);

    switch (option) {
    case SN_SEARCH_TITLE:
        predicate = note_contains_title;
        break;
    case SN_SEARCH_TAG:
        predicate = note_contains_tag;
        break;
    case SN_SEARCH_CONTENT:
        predicate = note_contains_content;
        break;
    case SN_SEARCH_ALL:
        return 0; // no filterage
    }

    for (int i = 0; i < result->count; i++) {
        if (predicate(result->data[i], keyword) == false) {
            // predicate is false; search result does not match
            result->hidden += 1;
            free(result->data[i]);

            // need to set this datum to NULL to prevend double free
            result->data[i] = NULL;
        }
    }

    return 0;
}
