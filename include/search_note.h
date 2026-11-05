#pragma once

enum SN_SEARCH_RANGE {
    SN_SEARCH_TITLE   = 0b001,
    SN_SEARCH_TAG     = 0b010,
    SN_SEARCH_CONTENT = 0b100
};

typedef struct search_cell_s search_cell_t;
struct search_cell_s {
    const char **content;
};

typedef struct search_result_s search_result_t;
struct search_result_s {
    int count;
    int size;
    search_cell_t *head;
};

search_result_t *new_seach_result();
void search_result_add_content(search_result_t *self, const char **content);

int search_note(const short search_type, search_result_t *result);
