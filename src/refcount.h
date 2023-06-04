#ifndef CFAC_REFCOUNT_H
#define CFAC_REFCOUNT_H

#include <stddef.h>
#include <stdint.h>

typedef struct RC_RefCountedBlock {
  size_t  ref_count;
  uint8_t data[] __attribute__((aligned(sizeof(max_align_t))));
} __attribute__((aligned(sizeof(max_align_t)))) RC_RefCountedBlock;

typedef struct {
  RC_RefCountedBlock * block;
} RC_Ref;

typedef struct {
  RC_RefCountedBlock * block;
} RC_ConstRef;

RC_Ref RC_allocate(size_t element_size);

static inline RC_ConstRef RC_as_const(RC_Ref ref);

//  RC_[Const]Ref RC_copy(RC_[Const]Ref ref);
#define RC_copy(ref)                                                                               \
  _Generic((ref), RC_Ref: RC_INT_copy_nonconst, RC_ConstRef: RC_INT_copy_const)(ref)
static inline RC_Ref      RC_INT_copy_nonconst(RC_Ref orig);
static inline RC_ConstRef RC_INT_copy_const(RC_ConstRef orig);

//  void RC_release(RC_[Const]Ref ref);
#define RC_release(ref)                                                                            \
  _Generic((ref), RC_Ref: RC_INT_release_nonconst, RC_ConstRef: RC_INT_release_const)(ref)
static inline void RC_INT_release_nonconst(RC_Ref ref);
static inline void RC_INT_release_const(RC_ConstRef ref);

//  [const] void * RC_get(RC_[Const]Ref ref);
#define RC_get(ref)                                                                                \
  _Generic((ref), RC_Ref: RC_INT_get_nonconst, RC_ConstRef: RC_INT_get_const)(ref)
static inline void *       RC_INT_get_nonconst(RC_Ref ref);
static inline const void * RC_INT_get_const(RC_ConstRef ref);

//  size_t RC_get_ref_count(RC_[Const]Ref ref);
#define RC_get_ref_count(ref)                                                                      \
  _Generic((ref),                                                                                  \
      RC_Ref: RC_INT_get_ref_count_nonconst,                                                      \
      RC_ConstRef: RC_INT_get_ref_count_const)(ref)
static inline size_t RC_INT_get_ref_count_nonconst(RC_Ref ref);
static inline size_t RC_INT_get_ref_count_const(RC_ConstRef ref);

void RC_INT_free(RC_RefCountedBlock * block);

static inline RC_ConstRef RC_as_const(RC_Ref ref) { return (RC_ConstRef){.block = ref.block}; }

static inline RC_Ref RC_INT_copy_nonconst(RC_Ref orig) {
  orig.block->ref_count++;
  return orig;
}
static inline RC_ConstRef RC_INT_copy_const(RC_ConstRef orig) {
  orig.block->ref_count++;
  return orig;
}

static inline void RC_INT_release_nonconst(RC_Ref ref) {
  if((ref.block->ref_count--) <= 1) RC_INT_free(ref.block);
}
static inline void RC_INT_release_const(RC_ConstRef ref) {
  if((ref.block->ref_count--) <= 1) RC_INT_free(ref.block);
}

static inline void *       RC_INT_get_nonconst(RC_Ref ref) { return ((void *)(ref.block->data)); }
static inline const void * RC_INT_get_const(RC_ConstRef ref) {
  return ((const void *)(ref.block->data));
}

static inline size_t RC_INT_get_ref_count_nonconst(RC_Ref ref) {
  return (ref.block == NULL) ? 0 : ref.block->ref_count;
}
static inline size_t RC_INT_get_ref_count_const(RC_ConstRef ref) {
  return (ref.block == NULL) ? 0 : ref.block->ref_count;
}

#endif
