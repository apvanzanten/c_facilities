
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "stat.h"
#include "test_utils.h"

#include "darray.h"

#include "span.h"

#define OK STAT_OK

static Result setup(void ** env_p);
static Result teardown(void ** env_p);

static Result tst_create_destroy(void) {
  Result r = PASS;

  DAR_DArray arr = {0};
  EXPECT_EQ(&r, OK, DAR_create(&arr, sizeof(int)));

  EXPECT_NE(&r, NULL, arr.data);
  if(HAS_FAILED(&r)) return r;

  EXPECT_EQ(&r, sizeof(int), arr.element_size);
  EXPECT_EQ(&r, 0, arr.size);

  EXPECT_EQ(&r, OK, DAR_destroy(&arr));
  EXPECT_EQ(&r, NULL, arr.data);

  return r;
}

static Result tst_create_from_cstr(void) {
  Result r = PASS;

  DAR_DArray arr = {0};

  const char str[] = "red light abnormality";
  const int  len   = strlen(str);
  const int  size  = len + 1; // +1 for null termination

  EXPECT_EQ(&r, OK, DAR_create_from_cstr(&arr, str));
  if(HAS_FAILED(&r)) return r;

  EXPECT_EQ(&r, size, arr.size);
  EXPECT_EQ(&r, '\0', *(char *)DAR_get(&arr, len));
  if(HAS_FAILED(&r)) return r;

  EXPECT_STREQ(&r, str, arr.data);

  EXPECT_EQ(&r, OK, DAR_destroy(&arr));

  return r;
}

static Result tst_create_from_span(void) {
  Result r = PASS;

  DAR_DArray arr = {0};

  {
    const char     str[] = "The Shire's this way.";
    const SPN_Span span  = SPN_from_cstr(str);

    EXPECT_EQ(&r, OK, DAR_create_from_span(&arr, span));

    EXPECT_EQ(&r, span.len, arr.size);
    EXPECT_EQ(&r, span.element_size, arr.element_size);
    EXPECT_NE(&r, span.begin, arr.data);

    EXPECT_ARREQ(&r, char, span.begin, arr.data, span.len);

    EXPECT_EQ(&r, OK, DAR_destroy(&arr));
  }
  {
    const int      vals[] = {1, 2, 3, 4, 6, 7};
    const SPN_Span span =
        (SPN_Span){.begin = vals, .len = sizeof(vals) / sizeof(int), .element_size = sizeof(int)};

    EXPECT_EQ(&r, OK, DAR_create_from_span(&arr, span));

    EXPECT_EQ(&r, span.len, arr.size);
    EXPECT_EQ(&r, span.element_size, arr.element_size);
    EXPECT_NE(&r, span.begin, arr.data);

    EXPECT_ARREQ(&r, int, span.begin, arr.data, span.len);

    EXPECT_EQ(&r, OK, DAR_destroy(&arr));
  }
  {
    const double   vals[] = {1.0, 2.0, 3.0, 4.0, 6.0, 7.0};
    const SPN_Span span   = (SPN_Span){.begin        = vals,
                                       .len          = sizeof(vals) / sizeof(double),
                                       .element_size = sizeof(double)};

    EXPECT_EQ(&r, OK, DAR_create_from_span(&arr, span));

    EXPECT_EQ(&r, span.len, arr.size);
    EXPECT_EQ(&r, span.element_size, arr.element_size);
    EXPECT_NE(&r, span.begin, arr.data);

    EXPECT_ARREQ(&r, double, span.begin, arr.data, span.len);

    EXPECT_EQ(&r, OK, DAR_destroy(&arr));
  }

  return r;
}

typedef struct {
  double  numbers[1000];
  uint8_t bytes[10000];
} BigStruct;

static Result tst_large_elements(void) {
  Result r = PASS;

  DAR_DArray arr = {0};

  BigStruct elements[8] = {0};
  for(size_t i = 0; i < sizeof(elements) / sizeof(BigStruct); i++) {
    for(size_t j = 0; j < sizeof(elements[i].numbers) / sizeof(double); j++) {
      elements[i].numbers[j] = (double)i * (double)j;
    }
    for(size_t j = 0; j < sizeof(elements[i].bytes) / sizeof(uint8_t); j++) {
      elements[i].bytes[j] = i * j;
    }
  }

  EXPECT_EQ(&r, OK, DAR_create(&arr, sizeof(BigStruct)));

  EXPECT_NE(&r, NULL, arr.data);
  if(HAS_FAILED(&r)) return r;

  for(size_t i = 0; i < sizeof(elements) / sizeof(BigStruct); i++) {
    EXPECT_EQ(&r, OK, DAR_push_back(&arr, &elements[i]));
    if(HAS_FAILED(&r)) return r;
  }

  for(size_t i = 0; i < sizeof(elements) / sizeof(BigStruct); i++) {
    BigStruct * a = &elements[i];
    BigStruct * b = (BigStruct *)DAR_get(&arr, i);

    for(size_t j = 0; j < sizeof(elements[i].numbers) / sizeof(double); j++) {
      EXPECT_EQ(&r, a->numbers[j], b->numbers[j]);
      if(HAS_FAILED(&r)) return r;
    }
    for(size_t j = 0; j < sizeof(elements[i].bytes) / sizeof(uint8_t); j++) {
      EXPECT_EQ(&r, a->bytes[j], b->bytes[j]);
      if(HAS_FAILED(&r)) return r;
    }
  }

  EXPECT_EQ(&r, OK, DAR_destroy(&arr));

  return r;
}

static Result tst_fixture(void * env) {
  Result       r   = PASS;
  DAR_DArray * arr = env;

  EXPECT_NE(&r, arr, NULL);
  if(HAS_FAILED(&r)) return r;

  EXPECT_NE(&r, NULL, arr->data);
  EXPECT_EQ(&r, 0, arr->size);
  EXPECT_NE(&r, 0, arr->capacity_magnitude);
  EXPECT_EQ(&r, 8, arr->element_size);

  return r;
}

static Result tst_push_back(void * env) {
  Result       r   = PASS;
  DAR_DArray * arr = env;

  double vals[] = {1.0, 2.0, 3.0, 4.0};

  EXPECT_EQ(&r, OK, DAR_push_back(arr, &vals[0]));
  EXPECT_EQ(&r, 1, arr->size);
  EXPECT_EQ(&r, 0, memcmp(arr->data, &vals[0], sizeof(double)));
  if(HAS_FAILED(&r)) return r;

  EXPECT_EQ(&r, OK, DAR_push_back(arr, &vals[1]));
  EXPECT_EQ(&r, OK, DAR_push_back(arr, &vals[2]));
  EXPECT_EQ(&r, OK, DAR_push_back(arr, &vals[3]));
  EXPECT_EQ(&r, 4, arr->size);
  EXPECT_EQ(&r, 0, memcmp(arr->data, vals, sizeof(vals)));
  if(HAS_FAILED(&r)) return r;

  return r;
}

static Result tst_pop_back(void * env) {
  Result       r   = PASS;
  DAR_DArray * arr = env;

  double vals[] = {1.0, 2.0, 3.0, 4.0};

  EXPECT_EQ(&r, OK, DAR_push_back(arr, &vals[0]));
  EXPECT_EQ(&r, OK, DAR_push_back(arr, &vals[1]));
  EXPECT_EQ(&r, OK, DAR_push_back(arr, &vals[2]));
  EXPECT_EQ(&r, OK, DAR_push_back(arr, &vals[3]));
  EXPECT_EQ(&r, 4, arr->size);
  EXPECT_EQ(&r, 0, memcmp(arr->data, vals, sizeof(vals)));
  if(HAS_FAILED(&r)) return r;

  EXPECT_EQ(&r, OK, DAR_pop_back(arr));
  EXPECT_EQ(&r, 3, arr->size);
  EXPECT_EQ(&r, 0, memcmp(arr->data, vals, sizeof(double) * 3));

  EXPECT_EQ(&r, OK, DAR_pop_back(arr));
  EXPECT_EQ(&r, 2, arr->size);
  EXPECT_EQ(&r, 0, memcmp(arr->data, vals, sizeof(double) * 2));

  EXPECT_EQ(&r, OK, DAR_pop_back(arr));
  EXPECT_EQ(&r, 1, arr->size);
  EXPECT_EQ(&r, 0, memcmp(arr->data, vals, sizeof(double) * 1));

  EXPECT_EQ(&r, OK, DAR_pop_back(arr));
  EXPECT_EQ(&r, 0, arr->size);

  return r;
}

static Result tst_capacity(void * env) {
  Result       r   = PASS;
  DAR_DArray * arr = env;

  const size_t num_elements = 1024;

  size_t capacity = DAR_get_capacity(arr);
  size_t size     = 0;

  while(size < num_elements) {
    const double val = 1.234;

    EXPECT_EQ(&r, OK, DAR_push_back(arr, &val));
    size++;

    if(capacity >= size) {
      EXPECT_EQ(&r, capacity, DAR_get_capacity(arr));
    } else {
      const size_t new_capacity = DAR_get_capacity(arr);
      EXPECT_TRUE(&r, capacity < new_capacity);
      capacity = new_capacity;
    }

    if(HAS_FAILED(&r)) return r;
  }

  do {
    EXPECT_EQ(&r, OK, DAR_pop_back(arr));
    size--;

    EXPECT_EQ(&r, OK, DAR_shrink_to_fit(arr));

    const size_t minimum_capacity = 8;

    if(((size * 2) > capacity) || (capacity <= minimum_capacity)) {
      EXPECT_EQ(&r, capacity, DAR_get_capacity(arr));
    } else {
      const size_t new_capacity = DAR_get_capacity(arr);
      EXPECT_TRUE(&r, capacity > new_capacity);
      capacity = new_capacity;
    }

    if(HAS_FAILED(&r)) return r;
  } while(size > 0);

  return r;
}

static Result tst_reserve(void * env) {
  Result       r   = PASS;
  DAR_DArray * arr = env;

  const size_t initial_capacity = DAR_get_capacity(arr);

  for(size_t num_elements = 16; num_elements < 10000; num_elements *= 1.2) {
    EXPECT_EQ(&r, OK, DAR_reserve(arr, num_elements));

    const size_t reserved_capacity = DAR_get_capacity(arr);

    for(size_t i = 1; i <= num_elements; i++) {
      const double val = (double)i;

      EXPECT_EQ(&r, OK, DAR_push_back(arr, &val));

      EXPECT_EQ(&r, i, arr->size);
      EXPECT_EQ(&r, reserved_capacity, DAR_get_capacity(arr));

      if(HAS_FAILED(&r)) return r;
    }

    while(arr->size != 0) {
      EXPECT_EQ(&r, OK, DAR_pop_back(arr));
      if(HAS_FAILED(&r)) return r;
    }

    EXPECT_EQ(&r, OK, DAR_shrink_to_fit(arr));
    EXPECT_EQ(&r, initial_capacity, DAR_get_capacity(arr));
    if(HAS_FAILED(&r)) return r;
  }

  return r;
}

static Result tst_resize(void * env) {
  Result       r   = PASS;
  DAR_DArray * arr = env;

  for(uint32_t initial_size = 16; initial_size < 1000; initial_size *= 1.2) {
    for(uint32_t new_size = 8; new_size < 1000; new_size *= 1.2) {
      for(uint32_t i = 0; i < initial_size; i++) {
        const double val = (double)i;
        EXPECT_EQ(&r, OK, DAR_push_back(arr, &val));

        EXPECT_EQ(&r, (i + 1), arr->size);

        if(HAS_FAILED(&r)) return r;
      }

      EXPECT_EQ(&r, initial_size, arr->size);
      if(HAS_FAILED(&r)) return r;

      EXPECT_EQ(&r, OK, DAR_resize(arr, new_size));
      EXPECT_EQ(&r, new_size, arr->size);
      EXPECT_TRUE(&r, DAR_get_capacity(arr) >= new_size);

      if(HAS_FAILED(&r)) return r;

      while(arr->size != 0) {
        EXPECT_EQ(&r, OK, DAR_pop_back(arr));
        if(HAS_FAILED(&r)) return r;
      }
      EXPECT_EQ(&r, OK, DAR_shrink_to_fit(arr));

      if(HAS_FAILED(&r)) return r;
    }
  }

  return r;
}

static Result tst_resize_zeroed(void * env) {
  Result       r   = PASS;
  DAR_DArray * arr = env;

  const double vals[1024] = {0};

  const uint32_t max_size = sizeof(vals) / sizeof(double);

  for(uint32_t new_size = 8; new_size < max_size; new_size *= 1.2) {
    // push some garbage data into array
    for(uint32_t i = 0; i < new_size; i++) {
      const double val = (double)i;
      EXPECT_EQ(&r, OK, DAR_push_back(arr, &val));
      if(HAS_FAILED(&r)) return r;
    }

    // resize back to 0, garbage data should still be in memory
    EXPECT_EQ(&r, OK, DAR_resize(arr, 0));
    EXPECT_EQ(&r, 0, arr->size);
    if(HAS_FAILED(&r)) return r;

    // resize zeroed, should overwrite garbage data with 0
    EXPECT_EQ(&r, OK, DAR_resize_zeroed(arr, new_size));
    EXPECT_EQ(&r, new_size, arr->size);
    EXPECT_TRUE(&r, DAR_get_capacity(arr) >= new_size);

    EXPECT_ARREQ(&r, double, (double *)arr->data, vals, new_size);

    // resize back to 0 to clean up
    EXPECT_EQ(&r, OK, DAR_resize(arr, 0));
    if(HAS_FAILED(&r)) return r;
  }

  return r;
}

static Result tst_resize_with_value(void * env) {
  Result       r   = PASS;
  DAR_DArray * arr = env;

  double         vals[1024] = {0};
  const uint32_t max_size   = sizeof(vals) / sizeof(double);

  // put test data in vals so we can compare against it
  const double test_value = 1.23456;
  for(uint32_t i = 0; i < max_size; i++) {
    vals[i] = test_value;
  }

  for(uint32_t new_size = 8; new_size < max_size; new_size *= 1.2) {
    // resize zeroed and then back to 0, memory now contains zeroes
    EXPECT_EQ(&r, OK, DAR_resize_zeroed(arr, new_size));
    EXPECT_EQ(&r, new_size, arr->size);
    EXPECT_EQ(&r, OK, DAR_resize(arr, 0));
    EXPECT_EQ(&r, 0, arr->size);

    // resize with value, should put value in memory
    EXPECT_EQ(&r, OK, DAR_resize_with_value(arr, new_size, &test_value));
    EXPECT_EQ(&r, new_size, arr->size);
    EXPECT_TRUE(&r, DAR_get_capacity(arr) >= new_size);
    if(HAS_FAILED(&r)) return r;

    EXPECT_ARREQ(&r, double, (double *)arr->data, vals, new_size);

    // resize back to 0 to clean up
    EXPECT_EQ(&r, OK, DAR_resize(arr, 0));
    if(HAS_FAILED(&r)) return r;
  }

  return r;
}

static Result tst_clear(void * env) {
  Result       r   = PASS;
  DAR_DArray * arr = env;

  // push some garbage data into array
  for(uint32_t i = 0; i < 100; i++) {
    const double val = (double)i;
    EXPECT_EQ(&r, OK, DAR_push_back(arr, &val));
    if(HAS_FAILED(&r)) return r;
  }

  EXPECT_EQ(&r, OK, DAR_clear(arr));
  EXPECT_EQ(&r, 0, arr->size);

  return r;
}

static Result tst_clear_and_shrink(void * env) {
  Result       r   = PASS;
  DAR_DArray * arr = env;

  const size_t   initial_capacity = DAR_get_capacity(arr);
  const uint32_t max_size         = 4096;

  for(uint32_t size = 16; size < max_size; size *= 1.2) {
    // push some garbage data into array
    for(uint32_t i = 0; i < size; i++) {
      const double val = (double)i;
      EXPECT_EQ(&r, OK, DAR_push_back(arr, &val));
      if(HAS_FAILED(&r)) return r;
    }

    EXPECT_EQ(&r, size, arr->size);
    EXPECT_TRUE(&r, DAR_get_capacity(arr) > initial_capacity);

    EXPECT_EQ(&r, OK, DAR_clear_and_shrink(arr));
    EXPECT_EQ(&r, 0, arr->size);
    EXPECT_EQ(&r, initial_capacity, DAR_get_capacity(arr));

    if(HAS_FAILED(&r)) return r;
  }

  return r;
}

static Result tst_get(void * env) {
  Result       r   = PASS;
  DAR_DArray * arr = env;

  double         vals[1024] = {0};
  const uint32_t max_size   = sizeof(vals) / sizeof(double);

  // put some interesting data in both vals and the array so we can compare
  vals[0] = 1.23456;
  for(uint32_t i = 1; i < max_size; i++) {
    vals[i] = vals[i - 1] * -1.1;
  }

  for(uint32_t i = 0; i < max_size; i++) {
    EXPECT_EQ(&r, OK, DAR_push_back(arr, &(vals[i])));

    EXPECT_EQ(&r, i + 1, arr->size);
    if(HAS_FAILED(&r)) return r;
  }

  for(uint32_t i = 0; i < max_size; i++) {
    double *       arr_val       = DAR_get(arr, i);
    const double * arr_val_const = DAR_get((const DAR_DArray *)arr, i);

    EXPECT_EQ(&r, 0, memcmp(&(vals[i]), arr_val, sizeof(double)));
    EXPECT_EQ(&r, 0, memcmp(&(vals[i]), arr_val_const, sizeof(double)));

    if(HAS_FAILED(&r)) return r;
  }

  return r;
}

static Result tst_get_checked(void * env) {
  Result       r   = PASS;
  DAR_DArray * arr = env;

  const double   vals[]   = {1.0, 2.0, 3.0, 4.0, 5.0};
  const uint32_t num_vals = sizeof(vals) / sizeof(double);

  EXPECT_EQ(&r, OK, DAR_push_back(arr, &vals[0]));
  EXPECT_EQ(&r, OK, DAR_push_back(arr, &vals[1]));
  EXPECT_EQ(&r, OK, DAR_push_back(arr, &vals[2]));
  EXPECT_EQ(&r, OK, DAR_push_back(arr, &vals[3]));
  EXPECT_EQ(&r, OK, DAR_push_back(arr, &vals[4]));

  if(HAS_FAILED(&r)) return r;

  for(uint32_t i = 0; i < num_vals; i++) {
    double *       arr_val       = NULL;
    const double * arr_val_const = NULL;

    EXPECT_EQ(&r, OK, DAR_get_checked(arr, i, (void **)(&arr_val)));
    EXPECT_EQ(&r, OK, DAR_get_checked((const DAR_DArray *)arr, i, (const void **)(&arr_val_const)));

    EXPECT_EQ(&r, 0, memcmp(&(vals[i]), arr_val, sizeof(double)));
    EXPECT_EQ(&r, 0, memcmp(&(vals[i]), arr_val_const, sizeof(double)));

    EXPECT_EQ(&r, STAT_ERR_ARGS, DAR_get_checked((DAR_DArray *)NULL, i, (void **)(&arr_val)));
    EXPECT_EQ(&r,
              STAT_ERR_ARGS,
              DAR_get_checked((const DAR_DArray *)NULL, i, (const void **)(&arr_val_const)));

    EXPECT_EQ(&r, STAT_ERR_ARGS, DAR_get_checked(arr, i, NULL));
    EXPECT_EQ(&r, STAT_ERR_ARGS, DAR_get_checked((const DAR_DArray *)arr, i, NULL));

    if(HAS_FAILED(&r)) return r;
  }

  for(uint32_t i = num_vals; i < num_vals * 2; i++) {
    double *       arr_val       = NULL;
    const double * arr_val_const = NULL;

    EXPECT_EQ(&r, STAT_ERR_RANGE, DAR_get_checked(arr, i, (void **)(&arr_val)));
    EXPECT_EQ(&r,
              STAT_ERR_RANGE,
              DAR_get_checked((const DAR_DArray *)arr, i, (const void **)(&arr_val_const)));

    EXPECT_EQ(&r, NULL, arr_val);
    EXPECT_EQ(&r, NULL, arr_val_const);

    if(HAS_FAILED(&r)) return r;
  }

  return r;
}

static Result tst_set(void * env) {
  Result       r   = PASS;
  DAR_DArray * arr = env;

  const double   vals[]   = {1.0, 2.0, 3.0, 4.0, 5.0};
  const uint32_t num_vals = sizeof(vals) / sizeof(double);

  EXPECT_EQ(&r, OK, DAR_resize_zeroed(arr, num_vals));
  if(HAS_FAILED(&r)) return r;

  for(uint32_t i = 0; i < num_vals; i++) {
    DAR_set(arr, i, (const void *)&vals[i]);
  }

  EXPECT_EQ(&r, arr->size, num_vals);

  EXPECT_EQ(&r, 0, memcmp(vals, arr->data, num_vals * sizeof(double)));

  return r;
}

static Result tst_set_checked(void * env) {
  Result       r   = PASS;
  DAR_DArray * arr = env;

  const double   vals[]   = {1.0, 2.0, 3.0, 4.0, 5.0};
  const uint32_t num_vals = sizeof(vals) / sizeof(double);

  EXPECT_EQ(&r, OK, DAR_resize_zeroed(arr, num_vals));
  if(HAS_FAILED(&r)) return r;

  for(uint32_t i = 0; i < num_vals; i++) {
    EXPECT_EQ(&r, OK, DAR_set_checked(arr, i, (const void *)&vals[i]));

    EXPECT_EQ(&r, STAT_ERR_ARGS, DAR_set_checked(NULL, i, (const void *)&vals[i]));
    EXPECT_EQ(&r, STAT_ERR_ARGS, DAR_set_checked(arr, i, NULL));

    if(HAS_FAILED(&r)) return r;
  }

  EXPECT_EQ(&r, arr->size, num_vals);

  EXPECT_EQ(&r, 0, memcmp(vals, arr->data, num_vals * sizeof(double)));

  for(uint32_t i = num_vals; i < num_vals * 2; i++) {
    EXPECT_EQ(&r, STAT_ERR_RANGE, DAR_set_checked(arr, i, (const void *)&vals[0]));

    if(HAS_FAILED(&r)) return r;
  }

  return r;
}

static Result tst_push_back_array(void * env) {
  Result       r   = PASS;
  DAR_DArray * arr = env;

  const double   vals_a[]   = {1.0, 2.0, 3.0, 4.0, 5.0};
  const double   vals_b[]   = {-1.0, -2.0, -3.0};
  const uint32_t num_vals_a = sizeof(vals_a) / sizeof(double);
  const uint32_t num_vals_b = sizeof(vals_b) / sizeof(double);
  const uint32_t num_zeroes = 5;

  EXPECT_EQ(&r, OK, DAR_push_back_array(arr, vals_a, num_vals_a));
  EXPECT_EQ(&r, num_vals_a, arr->size);
  if(HAS_FAILED(&r)) return r;

  EXPECT_EQ(&r, OK, DAR_resize_zeroed(arr, num_vals_a + num_zeroes));
  EXPECT_EQ(&r, num_vals_a + num_zeroes, arr->size);
  if(HAS_FAILED(&r)) return r;

  EXPECT_EQ(&r, OK, DAR_push_back_array(arr, vals_b, num_vals_b));
  EXPECT_EQ(&r, num_vals_a + num_zeroes + num_vals_b, arr->size);
  if(HAS_FAILED(&r)) return r;

  EXPECT_EQ(&r, 0, memcmp(DAR_get(arr, 0), vals_a, num_vals_a));
  EXPECT_EQ(&r, 0, memcmp(DAR_get(arr, num_vals_a + num_zeroes), vals_b, num_vals_b));

  return r;
}

static Result tst_push_back_span(void * env) {
  Result       r   = PASS;
  DAR_DArray * arr = env;

  const double   vals_a[]   = {1.0, 2.0, 3.0, 4.0, 5.0};
  const double   vals_b[]   = {-1.0, -2.0, -3.0};
  const uint32_t num_vals_a = sizeof(vals_a) / sizeof(double);
  const uint32_t num_vals_b = sizeof(vals_b) / sizeof(double);
  const uint32_t num_zeroes = 5;

  SPN_Span span_a = {.begin = vals_a, .element_size = sizeof(vals_a[0]), .len = num_vals_a};
  SPN_Span span_b = {.begin = vals_b, .element_size = sizeof(vals_b[0]), .len = num_vals_b};

  EXPECT_EQ(&r, OK, DAR_push_back_span(arr, span_a));
  EXPECT_EQ(&r, num_vals_a, arr->size);
  if(HAS_FAILED(&r)) return r;

  EXPECT_EQ(&r, OK, DAR_resize_zeroed(arr, num_vals_a + num_zeroes));
  EXPECT_EQ(&r, num_vals_a + num_zeroes, arr->size);
  if(HAS_FAILED(&r)) return r;

  EXPECT_EQ(&r, OK, DAR_push_back_span(arr, span_b));
  EXPECT_EQ(&r, num_vals_a + num_zeroes + num_vals_b, arr->size);
  if(HAS_FAILED(&r)) return r;

  EXPECT_EQ(&r, 0, memcmp(DAR_get(arr, 0), vals_a, num_vals_a));
  EXPECT_EQ(&r, 0, memcmp(DAR_get(arr, num_vals_a + num_zeroes), vals_b, num_vals_b));

  return r;
}

static Result tst_push_back_darray(void * env) {
  Result       r         = PASS;
  DAR_DArray * arr       = env;
  DAR_DArray   other_arr = {0};

  const double   vals_a[]   = {1.0, 2.0, 3.0, 4.0, 5.0};
  const double   vals_b[]   = {-1.0, -2.0, -3.0};
  const uint32_t num_vals_a = sizeof(vals_a) / sizeof(double);
  const uint32_t num_vals_b = sizeof(vals_b) / sizeof(double);

  EXPECT_EQ(&r, OK, DAR_create(&other_arr, sizeof(double)));
  if(HAS_FAILED(&r)) return r;

  EXPECT_EQ(&r, OK, DAR_push_back_array(arr, vals_a, num_vals_a));
  EXPECT_EQ(&r, num_vals_a, arr->size);
  if(HAS_FAILED(&r)) return r;

  EXPECT_EQ(&r, OK, DAR_push_back_array(&other_arr, vals_b, num_vals_b));
  EXPECT_EQ(&r, num_vals_b, other_arr.size);
  if(HAS_FAILED(&r)) return r;

  EXPECT_EQ(&r, OK, DAR_push_back_darray(arr, &other_arr));
  EXPECT_EQ(&r, num_vals_a + num_vals_b, arr->size);
  if(HAS_FAILED(&r)) return r;

  EXPECT_EQ(&r, 0, memcmp(DAR_get(arr, 0), vals_a, num_vals_a));
  EXPECT_EQ(&r, 0, memcmp(DAR_get(arr, num_vals_a), vals_b, num_vals_b));

  EXPECT_EQ(&r, OK, DAR_destroy(&other_arr));

  return r;
}

static Result tst_create_from(void * env) {
  Result       r         = PASS;
  DAR_DArray * arr       = env;
  DAR_DArray   other_arr = {0};

  const double   vals[]   = {1.0, 2.0, 3.0, 4.0, 5.0};
  const uint32_t num_vals = sizeof(vals) / sizeof(double);

  EXPECT_EQ(&r, OK, DAR_push_back_array(arr, vals, num_vals));
  EXPECT_EQ(&r, num_vals, arr->size);
  if(HAS_FAILED(&r)) return r;

  EXPECT_EQ(&r, OK, DAR_create_from(&other_arr, (const DAR_DArray *)arr));
  EXPECT_NE(&r, NULL, other_arr.data);
  EXPECT_NE(&r, arr->data, other_arr.data);
  EXPECT_EQ(&r, arr->size, other_arr.size);
  EXPECT_EQ(&r, arr->element_size, other_arr.element_size);
  if(HAS_FAILED(&r)) return r;

  EXPECT_EQ(&r, 0, memcmp(arr->data, other_arr.data, arr->size));

  EXPECT_EQ(&r, OK, DAR_destroy(&other_arr));

  return r;
}

static Result tst_equals(void * env) {
  Result       r         = PASS;
  DAR_DArray * arr       = env;
  DAR_DArray   other_arr = {0};

  const double   vals_a[]   = {1.0, 2.0, 3.0, 4.0, 5.0};
  const double   vals_b[]   = {-1.0, -2.0, -3.0, -4.0, -5.0};
  const uint32_t num_vals_a = sizeof(vals_a) / sizeof(double);
  const uint32_t num_vals_b = sizeof(vals_b) / sizeof(double);

  EXPECT_EQ(&r, OK, DAR_create(&other_arr, sizeof(double)));
  if(HAS_FAILED(&r)) return r;

  EXPECT_EQ(&r, OK, DAR_push_back_array(arr, vals_a, num_vals_a));
  EXPECT_EQ(&r, num_vals_a, arr->size);
  if(HAS_FAILED(&r)) return r;

  EXPECT_EQ(&r, OK, DAR_push_back_array(&other_arr, vals_a, num_vals_a));
  EXPECT_EQ(&r, num_vals_a, other_arr.size);
  if(HAS_FAILED(&r)) return r;

  EXPECT_TRUE(&r, DAR_equals((const DAR_DArray *)arr, (const DAR_DArray *)&other_arr));
  if(HAS_FAILED(&r)) return r;

  EXPECT_EQ(&r, OK, DAR_clear_and_shrink(&other_arr));
  EXPECT_FALSE(&r, DAR_equals((const DAR_DArray *)arr, (const DAR_DArray *)&other_arr));

  EXPECT_EQ(&r, OK, DAR_push_back_array(&other_arr, vals_b, num_vals_b));
  EXPECT_EQ(&r, num_vals_b, other_arr.size);
  if(HAS_FAILED(&r)) return r;

  EXPECT_FALSE(&r, DAR_equals((const DAR_DArray *)arr, (const DAR_DArray *)&other_arr));

  EXPECT_EQ(&r, OK, DAR_destroy(&other_arr));

  return r;
}

static Result tst_first_last(void * env) {
  Result       r   = PASS;
  DAR_DArray * arr = env;

  const double vals[]   = {1.0, 2.0, 3.0, 4.0, 5.0};
  const size_t num_vals = sizeof(vals) / sizeof(double);

  EXPECT_EQ(&r, OK, DAR_push_back_array(arr, vals, num_vals));
  if(HAS_FAILED(&r)) return r;

  EXPECT_EQ(&r, vals[0], *((double *)DAR_first(arr)));
  EXPECT_EQ(&r, vals[0], *((const double *)DAR_first((const DAR_DArray *)arr)));

  EXPECT_EQ(&r, vals[num_vals - 1], *((double *)DAR_last(arr)));
  EXPECT_EQ(&r, vals[num_vals - 1], *((const double *)DAR_last((const DAR_DArray *)arr)));

  EXPECT_EQ(&r, (((double *)DAR_last(arr)) + 1), DAR_end(arr));
  EXPECT_EQ(&r,
            (((const double *)DAR_last((const DAR_DArray *)arr)) + 1),
            DAR_end((const DAR_DArray *)arr));

  return r;
}

static Result tst_many_random_push_pop(void * env) {
  Result       r   = PASS;
  DAR_DArray * arr = env;

  double       vals[1000] = {0};
  const size_t max_size   = sizeof(vals) / sizeof(double);

  const size_t num_iterations = 25000;

  size_t current_size = 0;
  size_t target_size  = 0;

  for(size_t it = 0; it < num_iterations; it++) {
    if(current_size == target_size) {
      target_size = (size_t)(((double)rand() / (double)RAND_MAX) * (double)(max_size - 1));
    }

    const double delta = ((double)target_size - (double)current_size) / (double)max_size;

    // number centered on 0.5 where smaller/bigger means we need more pop/push
    const double guide = delta + 0.5;

    const bool is_push = (current_size == 0) || ((current_size < max_size) &&
                                                 (((double)rand() / (double)RAND_MAX) < guide));

    if(is_push) {
      vals[current_size] = (double)rand();

      EXPECT_EQ(&r, OK, DAR_push_back(arr, &vals[current_size]));
      current_size++;

      EXPECT_EQ(&r, current_size, arr->size);
    } else {
      // pop
      EXPECT_EQ(&r, OK, DAR_pop_back(arr));
      current_size--;

      EXPECT_EQ(&r, current_size, arr->size);
      EXPECT_EQ(&r, OK, DAR_shrink_to_fit(arr));
    }
    if(HAS_FAILED(&r)) return r;

    EXPECT_ARREQ(&r, double, arr->data, vals, current_size);
    if(HAS_FAILED(&r)) return r;
  }

  return r;
}

static Result tst_to_span(void * env) {
  Result       r   = PASS;
  DAR_DArray * arr = env;

  const double vals[]   = {1.0, 2.0, 3.0, 4.0, 5.0};
  const size_t num_vals = sizeof(vals) / sizeof(double);

  EXPECT_EQ(&r, OK, DAR_push_back_array(arr, vals, num_vals));
  if(HAS_FAILED(&r)) return r;

  const SPN_Span span = DAR_to_span(arr);

  EXPECT_EQ(&r, arr->size, span.len);
  EXPECT_EQ(&r, arr->element_size, span.element_size);
  EXPECT_EQ(&r, arr->data, span.begin);

  return r;
}

int main(void) {
  Test tests[] = {
      tst_create_destroy,
      tst_create_from_cstr,
      tst_create_from_span,
      tst_large_elements,
  };

  TestWithFixture tests_with_fixture[] = {
      tst_fixture,
      tst_push_back,
      tst_pop_back,
      tst_capacity,
      tst_reserve,
      tst_resize,
      tst_resize_zeroed,
      tst_resize_with_value,
      tst_clear,
      tst_clear_and_shrink,
      tst_get,
      tst_get_checked,
      tst_set,
      tst_set_checked,
      tst_push_back_array,
      tst_push_back_span,
      tst_push_back_darray,
      tst_create_from,
      tst_equals,
      tst_first_last,

      // run this a couple times (gets new seed every time)
      tst_many_random_push_pop,
      tst_many_random_push_pop,
      tst_many_random_push_pop,
      tst_many_random_push_pop,
      tst_many_random_push_pop,

      tst_to_span,
  };

  const Result test_res = run_tests(tests, sizeof(tests) / sizeof(Test));
  const Result test_with_fixture_res =
      run_tests_with_fixture(tests_with_fixture,
                             sizeof(tests_with_fixture) / sizeof(TestWithFixture),
                             setup,
                             teardown);

  return ((test_res == PASS) && (test_with_fixture_res == PASS)) ? 0 : 1;
}

static Result setup(void ** env_p) {
  Result        r     = PASS;
  DAR_DArray ** arr_p = (DAR_DArray **)env_p;

  // use both time and clock so we get a different seed even if we call this many times per second
  srand(time(NULL) + clock());

  EXPECT_NE(&r, NULL, arr_p);
  if(HAS_FAILED(&r)) return r;

  *arr_p = malloc(sizeof(DAR_DArray));
  EXPECT_NE(&r, NULL, *arr_p);
  if(HAS_FAILED(&r)) return r;

  EXPECT_EQ(&r, OK, DAR_create(*arr_p, sizeof(double)));

  return r;
}

static Result teardown(void ** env_p) {
  Result        r     = PASS;
  DAR_DArray ** arr_p = (DAR_DArray **)env_p;

  EXPECT_NE(&r, NULL, arr_p);
  if(HAS_FAILED(&r)) return r;

  EXPECT_EQ(&r, OK, DAR_destroy(*arr_p));

  free(*arr_p);
  *arr_p = NULL;

  return r;
}