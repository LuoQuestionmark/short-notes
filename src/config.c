#include "config.h"
#include <assert.h>
#include <dirent.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

const char config_file_name[] = ".sn_config";

bool get_config_file(char *buffer, int buflen) {
    assert(buffer);

    char current_dir[512] = { 0 };
    if (getcwd(current_dir, sizeof(current_dir)) == NULL) {
        perror("getcwd");
        return NULL;
    }

    bool continue_backwards = false;
    bool found              = false;
    ino_t last_ino          = 0;

    do {
        DIR *dir = opendir(current_dir);
        if (dir == NULL) {
            perror("opendir");
            return NULL;
        }
        continue_backwards = false;

        struct dirent *dirent = NULL;
        while ((dirent = readdir(dir)) != NULL) {
            if (strncmp(dirent->d_name, "..", sizeof("..")) == 0 &&
                dirent->d_type == DT_DIR) {
                // notice that for root dir, its parent is itself, so check the
                // ino number until it loop
                if (last_ino != dirent->d_ino) {
                    last_ino           = dirent->d_ino;
                    continue_backwards = true;
                }
                continue_backwards = false;
            }

            if (strncmp(dirent->d_name, config_file_name,
                        strlen(config_file_name)) == 0 &&
                dirent->d_type == DT_REG) {
                found = true;

                strncat(current_dir, "/",
                        sizeof(current_dir) - strlen(current_dir) - 1);
                strncat(current_dir, config_file_name,
                        sizeof(current_dir) - strlen(current_dir) - 1);
            }
        }

        closedir(dir);

        if (found) break;

        strncat(current_dir, "/..", sizeof(current_dir) - 1 - strlen("/.."));
    } while (continue_backwards);

    if (found) {
        if (realpath(current_dir, buffer) == NULL) {
            perror("realpath");
            return false;
        }
    }

    return found;
}

bool get_note_repo(char *buffer, int buflen) {
    char config_filepath[512] = { 0 };
    if (get_config_file(config_filepath, sizeof(config_filepath)) == false) {
        return false;
    }

    FILE *config_file = fopen(config_filepath, "r");
    if (config_file == NULL) {
        perror("fopen");
        return false;
    }

    char config_buffer[512] = { 0 }, repo[512] = { 0 };
    fread(config_buffer, sizeof(char), sizeof(config_buffer), config_file);

    sscanf(config_buffer, "repo: %s\n", repo);
    fclose(config_file);

    DIR *repo_dir = opendir(repo);
    if (repo_dir) {
        closedir(repo_dir);
    } else {
        // perror("cannot open repo with opendir");
        return false;
    }

    strncpy(buffer, repo, buflen);
    return true;
}

bool set_note_repo(const char *path) {
    if (path == NULL) {
        fputs("invalid path 'NULL'", stderr);
        return false;
    }
    DIR *target_dir = opendir(path);
    if (target_dir) {
        closedir(target_dir);
    } else {
        fprintf(stderr, "cannot open given path %s\n", path);
        perror("opendir");
    }
    // postcondition: path is valid

    char config_filepath[512] = { 0 };
    if (get_config_file(config_filepath, sizeof(config_filepath)) == false) {
        puts("no config file found, create one in current working directory");
        FILE *config = fopen(config_file_name, "w");

        if (config == NULL) {
            perror("fopen");
            return false;
        }
        fclose(config);
        realpath(config_file_name, config_filepath);
    } else {
        printf("a config file found at at %s\n", config_filepath);
    }
    // postcondition: config_filepath set

    char *real_repo_path = realpath(path, NULL);

    FILE *config = fopen(config_filepath, "w");
    if (config == NULL) {
        perror("fopen");
        free(real_repo_path);
        return false;
    }

    fprintf(config, "repo: %s\n", real_repo_path);
    fclose(config);

    printf("config repo directory at %s, file saved as %s\n", real_repo_path,
           config_filepath);

    free(real_repo_path);
    return true;
}
