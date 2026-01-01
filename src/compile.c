#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>

#define MAX_FILES 16
#define MAX_PATH 1024

char *all_object_files[MAX_FILES];
char *engine_files[MAX_FILES];
char *other_files[MAX_FILES];

int all_count = 0, engine_count = 0, other_count = 0;

char *safe_strdup(const char *s, int *count)
{
        if (*count >= MAX_FILES)
        {
                fprintf(stderr, "Warning: file array limit reached!\n");
                return NULL;
        }
        (*count)++;
        return strdup(s);
}

void categorize_object_file(const char *file)
{
        if (strncmp(file, "obj/engine/", 11) == 0)
        {
                engine_files[engine_count++] = strdup(file);
        }
        else
        {
                other_files[other_count++] = strdup(file);
        }
}

void mkdir_for_file(const char *file)
{
        char dir[MAX_PATH];
        strncpy(dir, file, MAX_PATH);
        char *last_slash = strrchr(dir, '/');
        if (last_slash)
        {
                *last_slash = 0;
                char cmd[MAX_PATH + 16];
                snprintf(cmd, sizeof(cmd), "mkdir -p %s", dir);
                system(cmd);
        }
}

void compile_file(const char *src_path, const char *compiler, const char *include, int is_c_file)
{
        const char *rel_path = src_path + strlen("./src/");
        char output_file[MAX_PATH];
        snprintf(output_file, sizeof(output_file), "obj/%s.o", rel_path);

        if (is_c_file)
        {
                size_t len = strlen(output_file);
                if (len >= 2)
                        strcpy(output_file + len - 2, ".o");
        }
        else
        {
                size_t len = strlen(output_file);
                if (len >= 2)
                        strcpy(output_file + len - 2, ".o");
        }

        mkdir_for_file(output_file);

        char cmd[MAX_PATH * 3];
        if (is_c_file)
                snprintf(cmd, sizeof(cmd), "%s %s -c %s -o %s -Wall -Wextra -fno-ident -fno-asynchronous-unwind-tables -g0", compiler, include, src_path, output_file);
        else
                snprintf(cmd, sizeof(cmd), "%s -c %s -o %s", compiler, src_path, output_file);

        printf("%s %s -> %s\n", is_c_file ? "Compiling" : "Assembling", src_path, output_file);
        system(cmd);

        all_object_files[all_count++] = strdup(output_file);
        categorize_object_file(output_file);
}

void list_files_recursive(const char *dir)
{
        DIR *dp = opendir(dir);
        if (!dp)
                return;

        struct dirent *entry;
        while ((entry = readdir(dp)) != NULL)
        {
                if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
                        continue;

                char path[MAX_PATH];
                snprintf(path, sizeof(path), "%s/%s", dir, entry->d_name);

                struct stat st;
                if (stat(path, &st) < 0)
                        continue;

                if (S_ISDIR(st.st_mode))
                {
                        list_files_recursive(path);
                }
                else if (S_ISREG(st.st_mode))
                {
                        size_t len = strlen(path);
                        if (len > 2 && strcmp(path + len - 2, ".c") == 0)
                        {
                                compile_file(path, "gcc", "-I./src", 1);
                        }
                        else if (len > 2 && strcmp(path + len - 2, ".s") == 0)
                        {
                                compile_file(path, "clang", NULL, 0);
                        }
                }
        }

        closedir(dp);
}

int compile(void)
{
        system("mkdir -p obj");
        system("mkdir -p bin");

        printf("Compiling source files...\n");
        list_files_recursive("./src");

        printf("\n=== Categorized Object Files ===\n");
        printf("Engine files:\n");
        for (int i = 0; i < engine_count; i++)
                printf("  %s\n", engine_files[i]);
        printf("Other files:\n");
        for (int i = 0; i < other_count; i++)
                printf("  %s\n", other_files[i]);

        printf("\n=== Linking ===\n");
        char link_cmd[MAX_PATH * MAX_FILES] = "cc -o bin/main";
        for (int i = 0; i < engine_count; i++)
        {
                strcat(link_cmd, " ");
                strcat(link_cmd, engine_files[i]);
        }
        for (int i = 0; i < other_count; i++)
        {
                strcat(link_cmd, " ");
                strcat(link_cmd, other_files[i]);
        }
        strcat(link_cmd, " -lSDL2 -lSDL2_image -lSDL2_ttf -lSDL2_mixer -lm");

        int result = system(link_cmd);
        if (result == 0)
        {
                printf("Linking successful: bin/main\n");
        }
        else
        {
                printf("Linking failed!\n");
        }

        return 0;
}
