#include <stdint.h>
#include <string.h>
#define CT_ARRAY_IMPL
#include <CArray.h>
#define CT_ITERATOR_IMPL
#include <CIterator.h>
#define DEFER_IMPL
#include <defer/defer.h>
#include <hashmap.c/hashmap.h>

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

void calcDistance(const zippedValue_t* value, int* out) {
  *out = abs(*((int*)value->left) - *((int*)value->right));
}

void sumInts(const int* i, int* out) {
  *out += *i;
}

typedef struct {
  int value;
  int count;
} ListCount;

int ListCount_cmp(const void* a, const void* b, void* udate) {
  const ListCount* lca = a;
  const ListCount* lcb = b;
  return lcb->value - lca->value;
}

uint64_t ListCount_hash(const void* item, uint64_t seed0, uint64_t seed1) {
  const ListCount* c = item;
  return c->value;
}

int parseInput(
  array_t* list1,
  array_t* list2,
  const char* inputFileName,
  int intSize
) {
  FILE* f;
  f = fopen(inputFileName, "r");
  if (f == NULL) return 1;

  char c[intSize + 1];
  c[intSize] = '\0';
  int i;
  while (true) {
    if (!fread(&c, sizeof(char), intSize, f)) { break; }
    i = atoi(c);
    array_push(list1, &i);
    fseek(f, 3, SEEK_CUR); // spaces
    fread(&c, sizeof(char), intSize, f);
    i = atoi(c);
    array_push(list2, &i);
    fseek(f, 1, SEEK_CUR); // \n
  }

  fclose(f);

  return 0;
}

int intCmp(const int* a, const int* b) {
  return *a - *b;
}

void usage(const char* const* argv) {
  printf("Usage: %s <example|input\n", argv[0]);
  exit(1);
}

int main(int argc, const char* const* argv) {
  if (argc != 2) usage(argv);
  int intSize;
  char* inputFile;
  if (strcmp(argv[1], "example") == 0) {
    intSize = 1;
    inputFile = "input/day01-example.txt";
  } else if (strcmp(argv[1], "input") == 0) {
    intSize = 5;
    inputFile = "input/day01.txt";
  } else {
    usage(argv);
  }

  array_t* list1 = array_create(sizeof(int));
  array_t* list2 = array_create(sizeof(int));
  defer({
    array_destroy(list1);
    array_destroy(list2);
  });
  assert(list1->size == list2->size);

  if (parseInput(list1, list2, inputFile, intSize) != 0) {
    printf("Failed to parse file %s\n", inputFile);
    return 1;
  }

  array_sort(list1, (ArrayCmpFn) &intCmp, qsort);
  array_sort(list2, (ArrayCmpFn) &intCmp, qsort);

  iter_t* iter1 = array_createIterator(list1);
  iter_t* iter2 = array_createIterator(list2);
  iter_t* iterZipped = iter_zipped(iter1, iter2);
  defer({ iter_destroy(iterZipped); });

  array_t* arr = array_createWithCap(sizeof(int), list2->size);
  defer({ array_destroy(arr); });
  arr = iter_map(iterZipped, arr, (void(*)(const void*, void*))calcDistance);
  assert(arr != NULL);

  iter_t* summedIter = array_createIterator(arr);
  defer({ iter_destroy(summedIter); });
  int totalDistance = 0;
  iter_reduce(summedIter, &totalDistance, (void(*)(const void*, void*))sumInts);

  printf("Part 1: %i\n", totalDistance);

  struct hashmap* occurences = hashmap_new(sizeof(ListCount), list2->size / 3, 0, 0, ListCount_hash, ListCount_cmp, NULL, NULL);
  defer({ hashmap_free(occurences); });
  ListCount* cnt;
  int* value;
  for (int i = 0; i < list2->size; i++) {
    value = array_get(list2, i);
    cnt = (ListCount*) hashmap_get(occurences, &(ListCount){ .value = *(value) });
    if (cnt == NULL) {
      hashmap_set(occurences, &(ListCount){.value = *(value), .count = 1});
    } else {
      cnt->count += 1;
    }
  }

  unsigned long similarityScore = 0;
  unsigned int occurenceCount = 0;
  for (int i = 0; i < list1->size; i++) {
    value = array_get(list1, i);
    cnt = (ListCount*) hashmap_get(occurences, &(ListCount){.value = *value });
    if (cnt == NULL)
      occurenceCount = 0;
    else
      occurenceCount = cnt->count;

    similarityScore += (*value) * occurenceCount;
  }

  printf("Part 2: %lu\n", similarityScore);

  return 0;
}
