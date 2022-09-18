/*
# c-embed
# embed virtual file systems into an c program
# - at build time
# - with zero dependencies
# - with zero code modifications
# - with zero clutter in your program
# author: nicholas mcdonald 2022
*/

#ifndef CEMBED
#define CEMBED

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <stdbool.h>

u_int32_t hash(char * key){   // Hash Function: MurmurOAAT64
  u_int32_t h = 3323198485ul;
  for (;*key;++key) {
    h ^= *key;
    h *= 0x5bd1e995;
    h ^= h >> 15;
  }
  return h;
}

typedef size_t epos_t;

struct EMAP_S {     // Map Indexing Struct
  u_int32_t hash;
  u_int32_t pos;
  u_int32_t size;
};
typedef struct EMAP_S EMAP;

struct EFILE_S {    // Virtual File Stream
  char* pos;
  char* end;
  size_t size;
  int err;
};
typedef struct EFILE_S EFILE;

// Error Handling

__thread int eerrcode = 0;
#define ethrow(err) { (eerrcode = (err)); return NULL; }
#define eerrno (eerrcode)

#define EERRCODE_SUCCESS 0
#define EERRCODE_NOFILE 1
#define EERRCODE_NOMAP 2
#define EERRCODE_NULLSTREAM 3
#define EERRCODE_OOBSTREAMPOS 4

const char* eerrstr(int e){
switch(e){
  case 0: return "Success.";
  case 1: return "No file found.";
  case 2: return "Mapping stucture error.";
  case 3: return "File stream pointer is NULL.";
  case 4: return "File stream pointer is out-of-bounds.";
  default: return "Unknown cembed error code.";
};
};

#define eerror(c) printf("%s: (%u) %s\n", c, eerrcode, eerrstr(eerrcode))

// File Useage

#ifndef CEMBED_BUILD

extern char cembed_map_start; // Embedded Indexing Structure
extern char cembed_map_end;
extern char cembed_map_size;

extern char cembed_fs_start;  // Embedded Virtual File System
extern char cembed_fs_end;
extern char cembed_fs_size;

EFILE* eopen(const char* file, const char* mode){

  EMAP* map = (EMAP*)(&cembed_map_start);
  const char* end = &cembed_map_end;

  if( map == NULL || end == NULL )
    ethrow(EERRCODE_NOMAP);

  const u_int32_t key = hash((char*)file);
  while( ((char*)map != end) && (map->hash != key) )
    map++;

  if(map->hash != key)
    ethrow(EERRCODE_NOFILE);

  EFILE* e = (EFILE*)malloc(sizeof *e);
  e->pos = (&cembed_fs_start + map->pos);
  e->end = (&cembed_fs_start + map->pos + map->size);
  e->size = map->size;

  return e;

}

void eclose(EFILE* e){
  free(e);
  e = NULL;
}

bool eeof(EFILE* e){
  if(e == NULL){
    ethrow(EERRCODE_NULLSTREAM);
    return true;
  }
  if(e->end < e->pos){
    ethrow(EERRCODE_OOBSTREAMPOS);
    return true;
  }
  return (e->end == e->pos);
}

// REVIEW

int egetpos(EFILE* e, epos_t* pos){

  if(e->end <= e->pos){
    pos = NULL;
    return 1;
  }

  *pos = (epos_t)(e->end - e->pos);
  return 0;

}

char* egets ( char* str, int num, EFILE* stream ){

  if(eeof(stream))
    return NULL;

  for(int i = 0; i < num && !eeof(stream) && *(stream->pos) != '\r'; i++)
    str[i] = *(stream->pos++);

  return str;

}

// Preprocessor Translation

#ifdef CEMBED_TRANSLATE
#define FILE EFILE
#define fopen eopen
#define fclose eclose
#define feof eeof
#define fgets egets
#define perror eerror
#define getline egetline
#endif

#endif
#endif
