#pragma once

#include <stdbool.h>

bool get_config_file(char *buffer, int buflen);
bool get_note_repo(char *buffer, int buflen);
bool set_note_repo(const char *path);
