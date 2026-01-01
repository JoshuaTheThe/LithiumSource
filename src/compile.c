#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <time.h>

#define MAX_PATH 1024

typedef struct
{
        char **files;
        size_t count;
        size_t capacity;
} FileArray;

void file_array_init(FileArray *arr)
{
        arr->files = NULL;
        arr->count = 0;
        arr->capacity = 0;
}

void file_array_add(FileArray *arr, const char *file)
{
        if (arr->count == arr->capacity)
        {
                arr->capacity = arr->capacity ? arr->capacity * 2 : 16;
                arr->files = realloc(arr->files, arr->capacity * sizeof(char *));
        }
        arr->files[arr->count++] = strdup(file);
}

void file_array_free(FileArray *arr)
{
        for (size_t i = 0; i < arr->count; i++)
                free(arr->files[i]);
        free(arr->files);
        arr->files = NULL;
        arr->count = arr->capacity = 0;
}

FileArray all_object_files;
FileArray engine_files;
FileArray other_files;

void categorize_object_file(const char *file)
{
        if (strncmp(file, "obj/engine/", 11) == 0)
                file_array_add(&engine_files, file);
        else
                file_array_add(&other_files, file);
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

int needs_rebuild(const char *src, const char *dst)
{
        struct stat st_src, st_dst;
        if (stat(src, &st_src) < 0)
                return 0;
        if (stat(dst, &st_dst) < 0)
                return 1;
        return st_src.st_mtime > st_dst.st_mtime;
}

void compile_file(const char *src_path, const char *compiler, const char *include, int is_c_file)
{
        const char *rel_path = src_path + strlen("./src/");
        char output_file[MAX_PATH];
        snprintf(output_file, sizeof(output_file), "obj/%s.o", rel_path);

        size_t len = strlen(output_file);
        if (len >= 2)
                strcpy(output_file + len - 2, ".o");

        if (!needs_rebuild(src_path, output_file))
        {
                file_array_add(&all_object_files, output_file);
                categorize_object_file(output_file);
                return;
        }

        mkdir_for_file(output_file);

        char cmd[MAX_PATH * 3];
        if (is_c_file)
                snprintf(cmd, sizeof(cmd), "%s %s -c %s -o %s -Wall -Wextra -fno-ident -fno-asynchronous-unwind-tables -g0", compiler, include, src_path, output_file);
        else
                snprintf(cmd, sizeof(cmd), "%s -c %s -o %s", compiler, src_path, output_file);

        printf("%s %s -> %s\n", is_c_file ? "Compiling" : "Assembling", src_path, output_file);
        if (system(cmd) != 0)
        {
                fprintf(stderr, "Failed to build %s\n", src_path);
                exit(1);
        }

        file_array_add(&all_object_files, output_file);
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
                        list_files_recursive(path);
                else if (S_ISREG(st.st_mode))
                {
                        size_t len = strlen(path);
                        if (len > 2 && strcmp(path + len - 2, ".c") == 0)
                                compile_file(path, "gcc", "-I./src", 1);
                        else if (len > 2 && strcmp(path + len - 2, ".s") == 0)
                                compile_file(path, "clang", NULL, 0);
                }
        }

        closedir(dp);
}

void compile(void)
{
        system("mkdir -p obj");
        system("mkdir -p bin");

        file_array_init(&all_object_files);
        file_array_init(&engine_files);
        file_array_init(&other_files);

        printf("Compiling source files...\n");
        list_files_recursive("./src");

        printf("\n=== Categorized Object Files ===\n");
        printf("Engine files:\n");
        for (size_t i = 0; i < engine_files.count; i++)
                printf("  %s\n", engine_files.files[i]);
        printf("Other files:\n");
        for (size_t i = 0; i < other_files.count; i++)
                printf("  %s\n", other_files.files[i]);

        printf("\n=== Linking ===\n");
        char link_cmd[MAX_PATH * 4] = "cc -o bin/main";
        for (size_t i = 0; i < engine_files.count; i++)
        {
                strcat(link_cmd, " ");
                strcat(link_cmd, engine_files.files[i]);
        }
        for (size_t i = 0; i < other_files.count; i++)
        {
                strcat(link_cmd, " ");
                strcat(link_cmd, other_files.files[i]);
        }
        strcat(link_cmd, " -lSDL2 -lSDL2_image -lSDL2_ttf -lSDL2_mixer -lm");

        if (system(link_cmd) != 0)
        {
                fprintf(stderr, "Linking failed!\n");
                exit(1);
        }
        printf("Linking successful: bin/main\n");

        file_array_free(&all_object_files);
        file_array_free(&engine_files);
        file_array_free(&other_files);
        exit(0);
}

#ifdef FIRST

int main(void)
{
        compile();
}

#endif
