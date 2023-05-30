#ifndef CFAC_REFCOUNT_H
#define CFAC_REFCOUNT_H

#include <stddef.h>
#include <stdint.h>

typedef struct RC_Entry {
  size_t  ref_count;
  uint8_t data[] __attribute__((aligned(sizeof(max_align_t))));
} __attribute__((aligned(sizeof(max_align_t)))) RC_Entry;

typedef struct {
  RC_Entry * entry;
} RC_Ref;

typedef struct {
  RC_Entry * entry;
} RC_ConstRef;

RC_Ref RC_allocate(size_t element_size);

static inline RC_ConstRef RC_as_const(RC_Ref ref);

//  RC_[Const]Ref RC_copy(RC_[Const]Ref ref);
#define RC_copy(ref)                                                                               \
  _Generic((ref), RC_Ref: RC_IMPL_copy_nonconst, RC_ConstRef: RC_IMPL_copy_const)(ref)
static inline RC_Ref      RC_IMPL_copy_nonconst(RC_Ref orig);
static inline RC_ConstRef RC_IMPL_copy_const(RC_ConstRef orig);

//  void RC_release(RC_[Const]Ref ref);
#define RC_release(ref)                                                                            \
  _Generic((ref), RC_Ref: RC_IMPL_release_nonconst, RC_ConstRef: RC_IMPL_release_const)(ref)
static inline void RC_IMPL_release_nonconst(RC_Ref ref);
static inline void RC_IMPL_release_const(RC_ConstRef ref);

//  [const] void * RC_get(RC_[Const]Ref ref);
#define RC_get(ref)                                                                                \
  _Generic((ref), RC_Ref: RC_IMPL_get_nonconst, RC_ConstRef: RC_IMPL_get_const)(ref)
static inline void *       RC_IMPL_get_nonconst(RC_Ref ref);
static inline const void * RC_IMPL_get_const(RC_ConstRef ref);

//  size_t RC_get_ref_count(RC_[Const]Ref ref);
#define RC_get_ref_count(ref)                                                                      \
  _Generic((ref),                                                                                  \
      RC_Ref: RC_IMPL_get_ref_count_nonconst,                                                      \
      RC_ConstRef: RC_IMPL_get_ref_count_const)(ref)
static inline size_t RC_IMPL_get_ref_count_nonconst(RC_Ref ref);
static inline size_t RC_IMPL_get_ref_count_const(RC_ConstRef ref);

void RC_IMPL_free(RC_Entry * entry);

static inline RC_ConstRef RC_as_const(RC_Ref ref) { return (RC_ConstRef){.entry = ref.entry}; }

static inline RC_Ref RC_IMPL_copy_nonconst(RC_Ref orig) {
  orig.entry->ref_count++;
  return orig;
}
static inline RC_ConstRef RC_IMPL_copy_const(RC_ConstRef orig) {
  orig.entry->ref_count++;
  return orig;
}

static inline void RC_IMPL_release_nonconst(RC_Ref ref) {
  if((ref.entry->ref_count--) <= 1) RC_IMPL_free(ref.entry);
}
static inline void RC_IMPL_release_const(RC_ConstRef ref) {
  if((ref.entry->ref_count--) <= 1) RC_IMPL_free(ref.entry);
}

static inline void *       RC_IMPL_get_nonconst(RC_Ref ref) { return ((void *)(ref.entry->data)); }
static inline const void * RC_IMPL_get_const(RC_ConstRef ref) {
  return ((const void *)(ref.entry->data));
}

static inline size_t RC_IMPL_get_ref_count_nonconst(RC_Ref ref) {
  return (ref.entry == NULL) ? 0 : ref.entry->ref_count;
}
static inline size_t RC_IMPL_get_ref_count_const(RC_ConstRef ref) {
  return (ref.entry == NULL) ? 0 : ref.entry->ref_count;
}

#endif
