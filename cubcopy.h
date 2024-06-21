#ifndef CUBCOPY_H

struct CopyOpts {
  char *include_only;
  char *exclude;
};

int cc_copy(char *source_path, char *dest_path, struct CopyOpts opts);

#endif
