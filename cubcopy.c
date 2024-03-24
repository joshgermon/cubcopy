#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>
#include "cubcopy.h"

const char *get_file_ext(const char *filename) {
  const char *dot = strrchr(filename, '.'); // Find the last occurrence of '.'
  if (!dot || dot == filename)
    return NULL;  // If there's no dot or dot is at the beginning, return NULL
  return dot + 1; // Return the extension (skip the dot)
}

int file_matches_filter(const char *filename, BackupOpts *opts) {
  // Can be expanded, right now just checking file extension
  if (opts->include_only != NULL) {
    const char *file_ext = get_file_ext(filename);
    if (file_ext == NULL)
      return 0;
    if (strcmp(file_ext, opts->include_only) == 0)
      return 1;
    return 0;
  }
  return 1;
}

int copy_file(const char *src_path, const char *dest_path) {
  FILE *source_file = fopen(src_path, "rb");
  FILE *destination_file = fopen(dest_path, "w+b");

  if (source_file == NULL || destination_file == NULL) {
    perror("Error opening file");
    return -1;
  }

  char buffer[CHUNK_SIZE];
  size_t bytes_read;
  while ((bytes_read = fread(buffer, 1, sizeof(buffer), source_file)) > 0) {
    fwrite(buffer, 1, bytes_read, destination_file);
  }
  fclose(source_file);
  fclose(destination_file);

  return 0;
}

void backup_files(DirTree *tree, const char *destination) {
  int files_copied = 0;
  printf("Backing up files...\n");
  for (int i = 0; i < tree->file_count; i++) {
    FileDetails *file = &(tree->files[i]);

    ssize_t full_dest_path_len =
        strlen(destination) + strlen(file->file_name) + 4;
    char full_dest_path[full_dest_path_len];
    snprintf(full_dest_path, full_dest_path_len, "%s/%s", destination,
             file->file_name);

    clock_t start_time = clock();
    if (copy_file(file->file_path, full_dest_path) == -1) {
      printf("Failed to copy file '%s', skipping...\n", file->file_name);
      continue;
    }
    clock_t end_time = clock();
    double time_taken = ((double)(end_time - start_time)) / CLOCKS_PER_SEC;
    printf("Copied file '%s' (%d of %d) in %f\n", file->file_name,
           ++files_copied, tree->file_count, time_taken);
  }
  printf("Backup complete - %d files backed up\n", files_copied);
}

DirTree *walk_directory(char *parent_dir, DirTree *tree, BackupOpts *opts) {
  DIR *dir = opendir(parent_dir);
  if (!dir) {
    perror("opendir");
    return tree; // Return the current file list
  }

  struct dirent *entry;
  while ((entry = readdir(dir)) != NULL) {
    char new_path[1024];

    ssize_t path_size = strlen(parent_dir) + strlen(entry->d_name) + 2;
    snprintf(new_path, path_size, "%s/%s", parent_dir, entry->d_name);

    // Skip current and parent directory entries
    if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
      continue;
    }

    struct stat path_stat;
    if (stat(new_path, &path_stat) == -1) {
      perror("stat");
      continue;
    }

    if (S_ISDIR(path_stat.st_mode)) {
      walk_directory(new_path, tree, opts);
    } else {
      // Check if file *should* be copied
      if (!file_matches_filter(entry->d_name, opts)) {
        printf("File does not match filter, skipping..\n");
        continue;
      }

      ssize_t file_name_len = strlen(entry->d_name) + 1;
      char *file_name = malloc(file_name_len);
      strncpy(file_name, entry->d_name, file_name_len);

      ssize_t file_path_len = strlen(new_path) + 1;
      char *file_path = malloc(file_path_len);
      strncpy(file_path, new_path, file_path_len);

      // Assign the file details
      FileDetails fi = {.file_name = file_name,
                        .size = path_stat.st_size,
                        .file_path = file_path};

      // Check if tree can hold more files, if not resize
      if (tree->files_capacity < tree->file_count + 1) {
        ssize_t new_cap = tree->files_capacity * 2;
        printf("No room! Resizing from %d to %zd\n", tree->files_capacity,
               new_cap);
        tree->files = realloc(tree->files, new_cap * sizeof(FileDetails));
        tree->files_capacity = new_cap;
      }

      tree->total_size += path_stat.st_size;
      tree->files[tree->file_count] = fi;
      tree->file_count++;
    }
  }
  closedir(dir);
  return tree;
}

void backup_directory(char *source_dir, char *target_dir, BackupOpts *opts) {
  FileDetails *files_found = malloc(INITIAL_CAPACITY * sizeof(FileDetails));
  DirTree tree = {
      .total_size = 0,
      .file_count = 0,
      .files_capacity = INITIAL_CAPACITY,
      .files = files_found,
  };

  walk_directory(source_dir, &tree, opts);

  backup_files(&tree, target_dir);

  for (int i = 0; i < tree.file_count; i++) {
    free(tree.files[i].file_path);
    free(tree.files[i].file_name);
  }

  free(tree.files);
}

int main() {
  BackupOpts opts = {.include_only = "c", .exclude = NULL};

  clock_t start_time = clock();
  backup_directory("/home/joshua/projects", "/home/joshua/test-dest", &opts);
  clock_t end_time = clock();
  double time_taken = ((double)(end_time - start_time)) / CLOCKS_PER_SEC;

  printf("============\n");
  printf("Total time taken: %.2f seconds\n", time_taken);
  return 0;
}
