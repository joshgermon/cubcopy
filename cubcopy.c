#include "cubcopy.h"
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

struct FileNode {
  struct FileNode *first_child;
  struct FileNode *next_sibling;
  FileDetails file_details;
};

// Function to print the tree structure
void print_tree(struct FileNode *root, int depth) {
  if (root == NULL)
    return;

  for (int i = 0; i < depth; i++) {
    printf("  "); // Adjust spacing for depth
  }

  printf("%s (%d)\n", root->file_details.file_name, root->file_details.size);

  // Recursively print children and siblings
  print_tree(root->first_child, depth + 1);
  print_tree(root->next_sibling, depth);
}

struct FileNode *create_file_node(FileDetails file_details) {
  struct FileNode *new_node = malloc(sizeof(struct FileNode));
  new_node->first_child = NULL;
  new_node->next_sibling = NULL;
  new_node->file_details = file_details;
  return new_node;
}

// TODO: Fix this function
void free_file_tree(struct FileNode *root) {
  if (root == NULL)
    return;

  // Free the children recursively
  free_file_tree(root->first_child);
  // Free the next sibling recursively
  free_file_tree(root->next_sibling);

  // Free the file details
  if (strcmp(root->file_details.file_name, "root") != 0) {
    free(root->file_details.file_path);
    free(root->file_details.file_name);
  }

  // Free the node itself
  free(root);
}

void add_child(struct FileNode *node, struct FileNode *child) {
  if (node->first_child == NULL) {
    node->first_child = child;
  } else {
    struct FileNode *sibling = node->first_child;
    while (sibling->next_sibling != NULL) {
      sibling = sibling->next_sibling;
    }
    sibling->next_sibling = child;
  }
}

const char *get_file_ext(const char *filename) {
  const char *dot = strrchr(filename, '.'); // Find the last occurrence of '.'
  if (!dot || dot == filename)
    return NULL;  // If there's no dot or dot is at the beginning, return NULL
  return dot + 1; // Return the extension (skip the dot)
}

// TODO: Function a bit of a mess
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

  char buffer[DEFAULT_CHUNK_SIZE];
  size_t bytes_read;
  while ((bytes_read = fread(buffer, 1, sizeof(buffer), source_file)) > 0) {
    fwrite(buffer, 1, bytes_read, destination_file);
  }
  fclose(source_file);
  fclose(destination_file);

  return 0;
}

void copy_contents(struct FileNode *node, char *dest_path, DirStats *stats) {
  if (node == NULL)
    return;

  char node_dest_path[MAX_PATH_LENGTH];
  snprintf(node_dest_path, MAX_PATH_LENGTH, "%s/%s", dest_path,
           node->file_details.file_name);

  // Copy the current file or directory
  if (node->file_details.is_dir) {
    // TODO: Retain permissions
    // If it's a directory, create the directory in the destination
    mkdir(node_dest_path, 0755);
    // Recursively copy the contents of this current dir if it's not empty
    if (node->first_child != NULL)
      copy_contents(node->first_child, node_dest_path, stats);
  } else {
    // If it's a file, copy it to the destination path
    copy_file(node->file_details.file_path, node_dest_path);
    stats->files_copied++;
    printf("Copied file: %s (%d of %d)\n", node->file_details.file_path, stats->files_copied, stats->file_count);
  }

  // Move to the next sibling and continue copying
  copy_contents(node->next_sibling, dest_path, stats);
}

void build_filetree(struct FileNode *parent_dir, DirStats *stats, BackupOpts *opts) {
  char *parent_path = parent_dir->file_details.file_path;
  DIR *dir = opendir(parent_path);
  if (!dir) {
    perror("opendir");
    return;
  }

  struct dirent *entry;
  while ((entry = readdir(dir)) != NULL) {
    // Skip the current and parent directories
    if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
      continue;

    char *new_path = malloc(MAX_PATH_LENGTH);
    snprintf(new_path, MAX_PATH_LENGTH, "%s/%s", parent_path, entry->d_name);

    struct stat path_stat;
    if (stat(new_path, &path_stat) == -1) {
      perror("stat");
      free(new_path);
      continue;
    }

    int is_dir = S_ISDIR(path_stat.st_mode);

    if(!is_dir && !file_matches_filter(entry->d_name, opts)) {
      free(new_path);
      continue;
    }

    ssize_t file_name_len = strlen(entry->d_name) + 1;
    char *file_name = malloc(file_name_len);
    strncpy(file_name, entry->d_name, file_name_len);

    FileDetails fi = {.file_name = file_name,
                      .size = path_stat.st_size,
                      .file_path = new_path,
                      .is_dir = is_dir};

    struct FileNode *file_node = create_file_node(fi);
    add_child(parent_dir, file_node);

    if (is_dir) {
      build_filetree(file_node, stats, opts);
    } else {
      stats->total_size += path_stat.st_size;
      stats->file_count += 1;
    }
  }
  closedir(dir);
}

void backup_directory(char *source_dir, char *target_dir, BackupOpts *opts) {
  DirStats stats = {.total_size = 0, .file_count = 0, .files_copied = 0};
  // TODO: Need a better way to initialise on the root node
  FileDetails fi = {
      .file_name = "root", .size = 0, .file_path = source_dir, .is_dir = 1};

  struct FileNode *tree = create_file_node(fi);
  build_filetree(tree, &stats, opts);

  print_tree(tree, 0);

  printf("Copying files...\n");
  copy_contents(tree->first_child, target_dir, &stats);

  printf("Total files copied: %d of %d\n", stats.files_copied, stats.file_count);

  free_file_tree(tree);
}

int main(int argc, char *argv[]) {
  // Example usage
  BackupOpts opts = {.include_only = NULL, .exclude = NULL};

  if (argc != 3) {
    printf("Usage: %s <source_dir> <target_dir>\n", argv[0]);
    return 1;
  }

  char *source_dir = argv[1];
  char *target_dir = argv[2];

  printf("============\n");
  printf("Source directory: %s\n", source_dir);
  printf("Target directory: %s\n", target_dir);
  printf("============\n");

  clock_t start_time = clock();

  backup_directory(source_dir, target_dir, &opts);

  clock_t end_time = clock();
  double time_taken = ((double)(end_time - start_time)) / CLOCKS_PER_SEC;

  printf("============\n");
  printf("Total time taken: %.2f seconds\n", time_taken);
  printf("============\n");
  return 0;
}
