#include "cubcopy.h"
#include <stdio.h>
#include <time.h>

int main(int argc, char *argv[]) {
  // Example usage
  struct CopyOpts opts = {.include_only = NULL, .exclude = NULL};

  if (argc != 3) {
    printf("Usage: %s <source_dir> <target_dir>\n", argv[0]);
    return 1;
  }

  // TODO: Validate directories are valid
  char *source_dir = argv[1];
  char *target_dir = argv[2];

  printf("============\n");
  printf("Source directory: %s\n", source_dir);
  printf("Target directory: %s\n", target_dir);
  printf("============\n");

  clock_t start_time = clock();

  cc_copy(source_dir, target_dir, opts);

  clock_t end_time = clock();
  double time_taken = ((double)(end_time - start_time)) / CLOCKS_PER_SEC;

  printf("============\n");
  printf("Total time taken: %.6f seconds\n", time_taken);
  printf("============\n");
  return 0;
}
