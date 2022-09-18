# c-embed

naturally embed read-only file/filesystem snapshots into any C99 program w. a single header, **zero dependencies** and **zero modifications** to your code at build time.

works with `C99` to `C++20` compilers

## usage

`c-embed` allows you to embed static, read-only snapshots of full filesystems into your C executable with almost zero effort.

`c-embed` builds an object file containing the static file system image using the embedder binary `c-embed`, which outputs the name of the embedded filesystem file (default: `c-embed.o`):

    c-embed myfile.txt <dir/file> <dir/file> ...
    > c-embed.o

*Note: Specifying a directory will recursively add all files (including in subdirectories) to the virtual file system.*

The single `c-embed.h` header exposes `stdio.h` style interface for accessing the embedded file system, which you simply link as an object file:

    /* main.c */

    #include <cembed.h>

    int main(){

      EFILE * pFile = eopen ("myfile.txt" , "r");

      //...

      eclose(pFile);
      return 0;

    }

**build**

    gcc main.c c-embed.o -o main

*Note: All files in the virtual filesystem are located relative to the working directory where the `c-embed` binary is executed.*

### building / installation

Build the `c-embed` executble and install the single header:

    sudo make all

### c-embed stdio.h interface

The `stdio.h` style interface for virtual filesystems implemented in `c-embed` contains the following:

    EFILE - embedded file pointer
    eopen - open embedded file
    eclose - close embedded file pointer
    eeof - end of file check
    egets - get string util eof or new line
    egetc - get current stream char promoted to int
    eerror - prints a descriptive error
    eread - read to memory address
    eseek - seek a position in the file
    etell - get the position in the file
    rewind - reset the position to the start of the file

in full analogy to their regular [`stdio.h` counterparts](https://cplusplus.com/reference/cstdio/).

### zero-modification embedding

If you use `stdio.h` for your filesystem-io, then your code requires **zero** modifications to use `c-embed` for embedding virtual file system snapshots. To facilitate this, `c-embed` only requires a minor modification to your build process:

    #include <stdio.h>

    int main(int argc, char* args[]){

      FILE* eFile = fopen("data/data.txt", "r");

      char buffer [100]{' '};

      if (eFile == NULL)
        perror ("Error opening file");

      else while(!feof(eFile)){
        if( fgets(buffer, 100, eFile) == NULL ) break;
        fputs (buffer , stdout);
      }

    }

Makefile - build with live file system:

    all:
      gcc main.c -o main

Makefile - build with  embedded virtual file system:

    DAT = data_path_1 data_path_2 # files to embed
    CEF = -DCEMBED_TRANSLATE -include /usr/local/include/c-embed.h

    all:
      gcc main.c $(shell c-embed $(DAT)) $(CEF) -o main

This allows for seamless transitioning between development and deployment builds without needing to modify the code at all.

*Note: The `CEMBED_TRANSLATE` preprocessor definition translates the `stdio.h` interface into the `c-embed` interface.*

## how it works

The `c-embed` binary builds two files, containing a filesystem indexing structure and a concatenation of the files we wish to embed.

The indexing structure uses a simple hash function to turn path strings into keys and store positions of binary data in the concatenated filesystem file.

The binary then uses `objcopy` to turn these two files into accessible objects, which we can easily link with our main executable and access via defined symbols.

The `c-embed.h` header then includes these symbols (one set of symbols for the indexing structure, one set for the file system) and implements the `stdio.h` style interface to interpret the data and access the embedded files.

### other details

The filesystem remains static in the object file, meaning that the file system need not exist as long as `c-embed.o` exists.

You can generate a static file system in one place and copy the `c-embed.o` file somewhere else and still have accessors given by the relative paths from the place where it was embedded.

The `c-embed.o` file represents the filesystem snapshot, and compiling your executable by linking this file embeds the snapshot.

## Motivation

Shipping data with programs is difficult when the goal is to distribute a single executable. For instance, shader programs for graphical applications are linked and compiled at runtime and therefore the data needs to be loaded in live.

A number of strategies exist to circumvent this problem, each with their own set of drawbacks which `c-embed` avoids.

### Strategies

1. Embed as string / const char* literal

    This is trash for obvious reasons. It requires the manual declaration, tracking and working with symbols. The contents have no syntax highlighting and are difficult to edit and swap out without a complex toolchain.

2. Base64-Encode and Embed as String Literal

    A number of other projects on Github use this approach for binary data. It is identical to the previous strategy.

Both of these solutions require modification / cluttering of your code with additional `#include` directives or symbols.

3. ASCII Files: `#include` as string literal

        const std::string file_content =
          #include "file"
        ;

    This is more flexible as it allows the data to remain in a separate file, albeit with the addition of the delimiters `R""(` and `)""`, which is ugly. This does not work for binary data. Note that here we are actually also managing file names, so changes to names or relative paths require modification of code. Additionally, *relative file paths must exist at compile time*.

### Solution

The goal of this tiny, single-header library is to leverage the c preprocessor to move the entire file system configuration to the build process and out of your code.

The `c-embed` binary uses objcopy to generate a single object file containing the relevant symbols for accessing the binary-encoded data.

Finally, an abstract `stdio.h` style interface is provided for retrieving the data with proper error handling.

The result is that `c-embed` does not strictly require that the file system itself exists at compile time or even relative to the build system; only at some point, somewhere.

Files stay as the files which they are, and can thus be manipulated appropriately by your editor of choice, with the embedding occuring at build time.

## Future Work

It would be interesting to consider if the file system could be made (temporarily) writeable in RAM. But this is beyond the purpose of this library.

Other necessary improvements include:
- active hash collision detection during embedding!
- somehow make it possible to link multiple file systems simultaneously

## License

MIT License
