#include "str_util.h"

#include <string.h>
#include <stdlib.h>

char* StrDup(const char* src) {
  size_t src_len = strlen(src);
  char* str = (char*)malloc(src_len + 1);
  if (!str) {
    return NULL;
  }
  strcpy(str, src);
  return str;
}

char* StrInsert(char* restrict src, const char* restrict str, size_t idx) {
  size_t src_len = strlen(src);
  size_t str_len = strlen(str);
  char* new_str = (char*)malloc(src_len + str_len + 1);
  if (!new_str) {
    return NULL;
  }
  memcpy(new_str, src, idx);
  memcpy(new_str + idx, str, str_len);
  memcpy(new_str + idx + str_len, src + idx, src_len - idx);
  new_str[src_len + str_len] = '\0';
  free(src);
  return new_str;
}