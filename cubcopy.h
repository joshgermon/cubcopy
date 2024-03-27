#ifndef CUBCOPY_H

#define MAX_PATH_LENGTH 1024
#define DEFAULT_CHUNK_SIZE 4096
#define MAX_CHUNK_SIZE 4000000
#define INITIAL_CAPACITY 1024

struct {
  char *include_only;
  char *exclude;
} typedef BackupOpts;

struct {
  int size;
  char *file_path;
  char *file_name;
  int is_dir;
} typedef FileDetails;

struct {
  int total_size;
  int file_count;
  int files_copied;
} typedef DirStats;

void backup_directory(char *source_dir, char *target_dir, BackupOpts *opts);

#endif
