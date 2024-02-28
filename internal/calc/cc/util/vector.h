#ifndef SMARTCALC_INTERNAL_UTIL_CC_CORE_VECTOR_H_
#define SMARTCALC_INTERNAL_UTIL_CC_CORE_VECTOR_H_

#include <stddef.h>
#include <stdlib.h>

static inline size_t* GetHeader(void* vec) {
  return ((size_t*)vec - 3);
}

static inline size_t VectorSize(void* vec) {
  return GetHeader(vec)[0];
}

static inline size_t VectorCap(void* vec) {
  return GetHeader(vec)[1];
}

static void* VectorInit(size_t member_size) {
  void* ptr = malloc(3 * sizeof(size_t) + 1);
  if (!ptr) {
    return NULL;
  }
  size_t* header = (size_t*)ptr;
  header[0] = header[1] = 0;
  header[2] = member_size;
  return (void*)(header + 3);
}

static inline void VectorDelete(void* vec) {
  free(GetHeader(vec));
}

static void* VectorAllocNewMember(void* vec, size_t member_size) {
  size_t* header = GetHeader(vec);
  size_t* size = header;
  size_t* cap = header + 1;
  if (member_size != header[2]) {
    return NULL;
  }
  if (*size == *cap) {
    *cap = (*cap * 2) + 1;
    void* new_ptr = realloc(header, 3 * sizeof(size_t) + (*cap * member_size));
    if (!new_ptr) {
      return NULL;
    }
    header = new_ptr;
    size = new_ptr;
    vec = (void*)(header + 3);
  }
  *size += 1;
  return vec;
}

#define VectorNew(_type) VectorInit(sizeof(_type))
#define VectorPush(_vec, _val) ((_vec = VectorAllocNewMember(_vec, sizeof(_val))) ? (_vec)[VectorSize(_vec) - 1] = _val, true : false)

#endif // SMARTCALC_INTERNAL_UTIL_CC_CORE_VECTOR_H_