#include <stdio.h>

int main(int argc, char* args[]){

  FILE* stream = fopen("data/data.txt", "r");
  if (stream == NULL){
    perror ("Error opening file");
    return 1;
  }

  char* line = NULL;
  size_t len = 0;
  ssize_t nread;

  while ((nread = getline(&line, &len, stream)) != -1) {
    fwrite(line, nread, 1, stdout);
  }

  return 0;

}
