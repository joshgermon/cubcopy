# Cubcopy 🧸

Cubcopy is a C library for copying files from one place to another (without preserving directory structure). It is a highly simplified and barebones implementation of a file copying tool. It is currently created for a very specific and specialised use case within [Cubby](https://github.com/joshgermon/cubby), where directory structure is not required to be preserved or any other metadata (permissions, etc); However flexibility in filtering items and organisational configurations are a priority.

**Important Note:**
It is worth repeating with emphasis that this is a crude, tiny and simple  implementation and it is not intended for general use in its current form.

## Usage

Below is a very simple example of how to use Cubcopy.


```c
#include "cubcopy.h"

int main(int argc, char **argv) {
    char *src = argv[1];
    char *dest = argv[2];

    BackupOpts opts = {
        .include_only = "pdf"
    };

    return backup_directory(src, dest);
}

```

## Library

You want to use Cubcopy? 🤔 Well, OK  - The best package manager is a good ol' copy-paste. Add the source code to your project and #include it, then you're good to go.
