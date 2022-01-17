# c-embed

Embed read-only filesystems into any C++11 program w. a single header, zero dependencies and zero modifications to your code.

## Usage

`c-embed` allows you to embed static, read-only snapshots of full filesystems into your C++ executable with almost zero effort. It provides a natural `stdio.h` style interface to retrieve and process data.

`c-embed` is extremely easy to use and integrate into your project:

    #include <stdio.h>
    //...
    FILE * pFile = fopen ("myfile.txt" , "r");

becomes

    #include <stdio.h>
    #include <cembed.h>
    //...
    EFILE * pFile = eopen ("myfile.txt" , "r");

All other `<stdio.h>` functions can then operate on the `EFILE*` pointer normally.

To facilitate this, `c-embed` requires a minor modification to your build process.

### Build Process Modification

To make a static file system available to c-embed, we simply run the embedder binary `c-embed` (see below):

    c-embed <dir/file> <dir/file> <dir/file> ...

which outputs an object file `c-embed.o`, which represents a snapshot of our file system with all files and directories built into it.

Additionally, it prints a `#define` compiler flag to the console which needs to be passed to your compiler in order to make the files visible through the `<cembed.h>` header. Something like:

    -Dcembed=file1,file2,file3,...

To finally provide the filesystem in your project, we build with the filesystem object file and with the compiler flag:

    g++ -std=c++11 main.cpp -o main c-embed.o -Dcembed=file1,...

#### Integration in Make

We can modify a makefile very simply to do this for us:

    CC = g++ -std=c++11
    DAT = data           #data directory to embed

    .PHONY: embedded
    embedded: CF = $(shell c-embed $(DAT)) c-embed.o
    embedded: all

    build:
      $(CC) main.cpp -o main ${CF}

    all: build

Running `make all` then build the system with a relative file system, while `make embedded` builds it with `data` embedded as a static file system. Note that we still require the `EFILE*` and `eopen` accessors in this case.

#### Zero-Mod Accessors

If you additionally define the macro `CEMBED_TRANSLATE`, then all occurences of `FILE` are replaced with `EFILE` and all occurences of `fopen` become `eopen`. This should be done stably after including of all `<stdio.h>` headers.

This effectively makes the system fully translated and moves the entire embedding to the build process.

The following code:

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

with the following makefile:

    CC = g++ -std=c++11
    DAT = data           #data directory to embed

    CIF = -include /usr/local/include/c-embed.h
    .PHONY: embedded relative

    embedded: CF = $(shell c-embed $(DAT)) c-embed.o -DCEMBED_TRANSLATE
    embedded: all

    relative: CF =
    relative: CIF =
    relative: all

    build:
    	$(CC) main.cpp -o main ${CF} $(CIF)

    all: build

Can be built once using `make relative` to run the system with a relative file system, and `make embedded` to generate the binary with an embedded file system. See `/example/` for a working example and try it out yourself!

#### Other Details

Note that the paths of all files and directories are accessible in your C++ code via the relative path from where `c-embed` was executed.

The filesystem remains static in the object file, meaning that the file system need not exist as long as `c-embed.o` exists.

Additionally, you can generate a static file system in one place and copy the `c-embed.o` file somewhere else and still have accessors given by the relative paths from the place where it was embedded.

### Building / Installation

Build and Install c-embed:

    sudo make all

#### Details

c-embed consists of a single encoding binary `cembed` and a single header file `cembed.h`. To build the program, run:

    # build embedder
    gcc c-embed.c -o c-embed

    # alternatively: make
    make build

The c-embed binary and header can be installed using the makefile (with privilege):

    sudo make install

You can change the install and include directories in the makefile (or just do it yourself, it's like 2 lines).

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

The goal of this tiny, single-header library is to leverage the c++ preprocessor to move the entire file system configuration to the build process and out of your code.

The `c-embed` binary uses objcopy to generate a single object file containing the relevant symbols for accessing the hex-encoded data.

The preprocessor then utilizes the passed `-Dcembed` definition to extract the symbols and make them accessible through a map.

Finally, an abstract `stdio.h` style interface is provided for retrieving the data with proper error handling.

The result is that `c-embed` does not strictly require that the file system itself exists at compile time or even relative to the build system; only at some point, somewhere.

Files stay as the files which they are, and can thus be manipulated appropriately by your editor of choice, with the embedding occuring at build time.

### Limitations

Because `c-embed` uses the preprocessor to expand the `-Dcembed=...` macro into symbols and map them for access, we are limited by the lack of preprocess macro-iteration and therefore can only embed as many files as we are willing to copy and paste lines in `c-embed.h`. Currently I have 16 embeddable files. See below for a potential long-term fix.

Additionally, because filesystem accesor tokens `/` and `.` are not valid identifier tokens in C/C++, they must be converted, in this case to underscores `_`. Therefore, embedding files with ambiguous (for C/C++) names like `data_txt,data.txt` will result in undefined behavior and should be avoided. This is potentially also fixable (see below).

## Future Work

instead of requiring the c++ preprocessor to expand the set of defined file macros, which does not allow for looping and thus limits the total number of embeddable files, we could instead additionally embed a single globally available indexing structure which could be used for generating accessors instead.

Additionally, it would be interesting to consider if the file system could be made (temporarily) writeable in RAM. But this is beyond the purpose of this library.

## License

MIT License
