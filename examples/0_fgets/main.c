#include <stdio.h>

int main(int argc, char* args[]){

  FILE* eFile = fopen("data/data2/data2.txt", "r");

  char buffer [100] = {' '};

  if (eFile == NULL)
    perror ("Error opening file");

  else while(!feof(eFile)){
    if( fgets(buffer, 100, eFile) == NULL ) break;
    fputs (buffer , stdout);
  }

  fclose(eFile);

}
