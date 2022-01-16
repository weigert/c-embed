/*
  cembed - embed files into a c++ program
  zero dependencies, zero clutter in your program
  Author: Nicholas McDonald, 2022
*/

#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>

#define DIRENT_FILE 8
#define DIRENT_DIR 4
#define MAXPATH 512

int first = 0;

char* sanitize(char* s){

  size_t n = strlen(s)+1;
  char* ns = (char*)malloc(n);

  for(size_t i = 0; i < n; i++)
    ns[i] = (s[i] == '.' || s[i] == '/') ? '_' : s[i] ;
  return ns;

}

void embed(char* s){

  char* call = (char*)malloc(2*MAXPATH*sizeof(char));
  char* ns = sanitize(s);

  const char* format = \
  "objcopy --input binary --output elf64-x86-64 --binary-architecture i386 %s %s.o "\
  "--redefine-sym _binary_%s_start=cembed_%s_start "\
  "--redefine-sym _binary_%s_end=cembed_%s_end "\
  "--redefine-sym _binary_%s_size=cembed_%s_size ";

  int r;
  r = sprintf(call, format, s, s, ns, ns, ns, ns, ns, ns);
  system(call);
  r = sprintf(call, "mv %s.o cembed_tmp/%s.o", s, ns);
  system(call);

  if(first++ == 0) printf("%s", ns);
  else printf(",%s", ns);

  free(call);
  free(ns);

}

void tryiterdir(char* d){

  char* fullpath = (char*)malloc(MAXPATH*sizeof(char));

  DIR *dir;
  struct dirent *ent;

  if ((dir = opendir (d)) != NULL) {

    while ((ent = readdir (dir)) != NULL) {

      if(strcmp(ent->d_name, ".") == 0) continue;
      if(strcmp(ent->d_name, "..") == 0) continue;

      if(ent->d_type == DIRENT_FILE){
        strcpy(fullpath, d);
        strcat(fullpath, "/");
        strcat(fullpath, ent->d_name);
        embed(fullpath);
      }

      else if(ent->d_type == DIRENT_DIR){
        strcpy(fullpath, d);
        strcat(fullpath, "/");
        strcat(fullpath, ent->d_name);
        tryiterdir(fullpath);
      }

    }

    closedir (dir);

  }

  else {

    strcpy(fullpath, d);
    embed(fullpath);

  }

  free(fullpath);

}

int main(int argc, char* argv[]){

  if(argc <= 1)
    return 0;

  system("if [ ! -d cembed_tmp ]; then mkdir cembed_tmp; fi;");
  printf("-Dcembed=");

  for(int i = 1; i < argc; i++)
    tryiterdir(argv[i]);

  system("ld -relocatable cembed_tmp/*.o -o c-embed.o");
  system("rm -rf cembed_tmp");
  printf("\n");

  return 0;

}
