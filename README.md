# Cubcopy 🧸

Cubcopy is a C library for copying files from one place to another. It is a highly simplified and barebones implementation of a file copying library. It is currently created for a very specific and specialised use case within [Cubby](https://github.com/joshgermon/cubby) to copy a lot of medium-size files and where a hardened and bullet-proof library is not required. However flexibility in filtering items and organisational configurations are a priority including hooks into the copying process.

> \[!IMPORTANT]\
> It is worth repeating with emphasis that this is a crude, tiny and simple implementation and was created to solve a specific problem in a hobby project

## Usage

Below is a very simple example of how to use Cubcopy.


```c
#include "cubcopy.h"

int main(int argc, char **argv) {
    char *src = argv[1];
    char *dest = argv[2];

    CopyOpts opts = {
        .include_only = "pdf" // Copies only .pdf files
    };

    // Recursively copies source to directory
    // Note: opts not required, cc_copy(src, dest, NULL);
    return cc_copy(src, dest, opts);
}

```

## Library

You want to use Cubcopy? 🤔 Well, OK  - The best package manager is a good ol' copy-paste. Add the source code to your project and #include it, then you're good to go.
