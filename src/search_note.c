#include "search_note.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

search_result_t *new_seach_result() {
    search_result_t *ret = calloc(1, sizeof(search_result_t));
    ret->count           = 0;
    ret->head            = calloc(1, sizeof(search_cell_t));
    ret->size            = 1;

    return ret;
}

void search_result_add_content(search_result_t *self, const char **content) {
    assert(self);
    assert(self->size > 0);

    if (self->count == self->size) {
        // list full, double the size
        search_cell_t *realloc_cells =
            realloc(self->head, 2 * self->size * sizeof(search_cell_t));
        if (realloc_cells == NULL) {
            perror("realloc");
            exit(EXIT_FAILURE);
        }
    }
}

int search_note(const short search_type, search_result_t *result) {
    return -1;
}
