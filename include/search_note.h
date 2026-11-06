#pragma once

enum SN_SEARCH_RANGE {
    SN_SEARCH_TITLE   = 0b001,
    SN_SEARCH_TAG     = 0b010,
    SN_SEARCH_CONTENT = 0b100
};

typedef struct search_result_s search_result_t;
struct search_result_s {
    int count;
    int size;
    char **data;
};

search_result_t *new_seach_result();
void search_result_free(search_result_t *result);
void search_result_add_content(search_result_t *result, const char *content);
void search_result_print(search_result_t *result);

// search all the notes in the dir and save the result in `result`
int search_note(const char *keyword, const char *dir, search_result_t *result);
// filter notes (refined search) with given result
int search_filter(const short search_type, const char *keyword,
                  search_result_t *result);
