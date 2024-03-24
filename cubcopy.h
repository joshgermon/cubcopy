#ifndef CUBCOPY_H

#define CHUNK_SIZE 4096
#define INITIAL_CAPACITY 1024

struct {
  char *include_only;
  char *exclude;
} typedef BackupOpts;

struct {
  int size;
  char *file_path;
  char *file_name;
} typedef FileDetails;

struct {
  int total_size;
  int file_count;
  int files_capacity;
  FileDetails *files;
} typedef DirTree;

void backup_directory(char *source_dir, char *target_dir, BackupOpts *opts);

#endif
