/*
  cembed - embed files into a c++ program
  zero dependencies, zero clutter in your program
  Author: Nicholas McDonald, 2022
*/

#include <stdio.h>
#include <cstring>
#include <map>
#include <iostream>

#ifndef CEMBED
#define CEMBED

// Macro Iteration / ForEach

#define FE_0x00(CALLBACK)
#define FE_0x01(CALLBACK, X)      CALLBACK(X)
#define FE_0x02(CALLBACK, X, ...) CALLBACK(X)FE_0x01(CALLBACK, __VA_ARGS__)
#define FE_0x03(CALLBACK, X, ...) CALLBACK(X)FE_0x02(CALLBACK, __VA_ARGS__)
#define FE_0x04(CALLBACK, X, ...) CALLBACK(X)FE_0x03(CALLBACK, __VA_ARGS__)
#define FE_0x05(CALLBACK, X, ...) CALLBACK(X)FE_0x04(CALLBACK, __VA_ARGS__)
#define FE_0x06(CALLBACK, X, ...) CALLBACK(X)FE_0x05(CALLBACK, __VA_ARGS__)
#define FE_0x07(CALLBACK, X, ...) CALLBACK(X)FE_0x06(CALLBACK, __VA_ARGS__)
#define FE_0x08(CALLBACK, X, ...) CALLBACK(X)FE_0x07(CALLBACK, __VA_ARGS__)
#define FE_0x09(CALLBACK, X, ...) CALLBACK(X)FE_0x08(CALLBACK, __VA_ARGS__)
#define FE_0x0A(CALLBACK, X, ...) CALLBACK(X)FE_0x09(CALLBACK, __VA_ARGS__)
#define FE_0x0B(CALLBACK, X, ...) CALLBACK(X)FE_0x0A(CALLBACK, __VA_ARGS__)
#define FE_0x0C(CALLBACK, X, ...) CALLBACK(X)FE_0x0B(CALLBACK, __VA_ARGS__)
#define FE_0x0D(CALLBACK, X, ...) CALLBACK(X)FE_0x0C(CALLBACK, __VA_ARGS__)
#define FE_0x0E(CALLBACK, X, ...) CALLBACK(X)FE_0x0D(CALLBACK, __VA_ARGS__)
#define FE_0x0F(CALLBACK, X, ...) CALLBACK(X)FE_0x0E(CALLBACK, __VA_ARGS__)

#define GET_MACRO(_0,_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,NAME,...) NAME
#define FOR_EACH(action,...)            \
  GET_MACRO(_0, __VA_ARGS__,            \
    FE_0x0F, FE_0x0E, FE_0x0D, FE_0x0C, \
    FE_0x0B, FE_0x0A, FE_0x09, FE_0x08, \
    FE_0x07, FE_0x06, FE_0x05, FE_0x04, \
    FE_0x03, FE_0x02, FE_0x01, FE_0x00) \
    (action,__VA_ARGS__)

// Extern Symbol Set Definition Macro

#define cembed_start(file)  cembed_##file##_start
#define cembed_end(file)    cembed_##file##_end
#define cembed_size(file)   cembed_##file##_size

#define cembed_define(file)       \
  extern char cembed_start(file); \
  extern char cembed_end(file);   \
  extern char cembed_size(file);

// Raw File Pointer Container

typedef size_t epos_t;

struct EFILE {

  EFILE(){}
  EFILE(char* p, char* e, char* s){
    pos = p; end = e; size = s;
  };

  char* pos = NULL;
  char* end = NULL;
  char* size = NULL;
  int err = 0;

};

#define cembed_efile(file) EFILE(\
  &cembed_start(file),     \
  &cembed_end(file),       \
  &cembed_size(file)       \
)

// Embedded File Symbol Creation

#ifdef cembed
FOR_EACH(cembed_define, cembed)
#endif

// Embedded File Mapping

#define cembed_tuple(file) { #file, cembed_efile(file) }
#define cembed_tuple_map_entry(file) cembed_tuple(file),

struct cekeycomp {
  bool operator () (const char* lhs, const char* rhs)
    const { return strcmp(lhs, rhs) < 0; }
};
typedef std::map<const char*, EFILE, cekeycomp> cemap;

static cemap cesym = {
#ifdef cembed
FOR_EACH(cembed_tuple_map_entry, cembed)
#endif
};

#define cembed_map(file) \
  cesym.emplace( #file, cembed_efile(file) );

// Error Handling

__thread int eerrcode;
#define ethrow(err) { (eerrcode = (err)); return NULL; }
#define eerrno (eerrcode)

#define EERRCODE_NOFILE 0

const char* eerrstr(int e){
switch(e){
  case 0: return "No file found.";
  default: return "Unknown cembed error code.";
};
};

#define eerror(c) printf("%s: (%u) %s\n", c, eerrcode, eerrstr(eerrcode))

int ferror ( EFILE * stream ){
  if(stream == NULL) return 0;
  return stream->err;
}

// Point or Underscore Access

char* cefile(const char* f){
  size_t n = strlen(f)+1;
  char* c = new char[n];
  for(size_t i = 0; i < n; i++)
    c[i] = (f[i] == '.' || f[i] == '/')?'_':f[i];
  return c;
}

EFILE* eopen(const char* file, const char* mode){
  auto search = cesym.find(cefile(file));
  if(search == cesym.end())
    ethrow(EERRCODE_NOFILE);
  return &cesym[cefile(file)];
}

// Actual File Useage

void fclose(EFILE* stream){}

bool feof(EFILE* stream){
  if(stream == NULL) return false;
  return (stream->end == stream->pos);
}

int fgetpos(EFILE* stream, epos_t* pos){

  if(stream->end <= stream->pos){
    pos = NULL;
    return 1;
  }

  *pos = (epos_t)(stream->end - stream->pos);
  return 0;

}

char* fgets ( char* str, int num, EFILE* stream ){

  if(feof(stream))
    return NULL;

  for(int i = 0; i < num && !feof(stream) && *(stream->pos) != '\r'; i++)
    str[i] = *(stream->pos++);

  return str;

}

#define CEMBED_GETLINE_NBUFCHUNK 128

size_t getline ( char** line, size_t* n, EFILE*& stream ){

  if(stream == NULL)
    return -1;

  if(feof(stream))
    return -1;

  *n = CEMBED_GETLINE_NBUFCHUNK;
  *line = (char*)malloc(CEMBED_GETLINE_NBUFCHUNK);

  size_t i = 0;
  while(!feof(stream)){
    (*line)[i++] = *(stream->pos);
    if(i > *n){
      *n += CEMBED_GETLINE_NBUFCHUNK;
      *line = (char*)realloc(*line, *n);
    }
    if(*(stream->pos) == '\n'){
      stream->pos++;
      break;
    }
    stream->pos++;
  }

  *line = (char*)realloc(*line, i);
  *n = i;
  return i;

}

#ifdef CEMBED_TRANSLATE
#define FILE EFILE
#define fopen eopen
#endif

#endif
