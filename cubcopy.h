#ifndef CUBCOPY_H

#define MAX_PATH_LENGTH 1024
#define DEFAULT_CHUNK_SIZE 8096
#define INITIAL_CAPACITY 1024

struct CopyOpts {
  char *include_only;
  char *exclude;
};

struct FileNode {
  int size;
  char *file_path;
  char *dest_path;
  char *file_name;
  int is_dir;
};

void copy_directory(char *source_dir, char *target_dir, struct CopyOpts *opts);

#endif
