#include "hashset.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>

int kinitialAllocation = 4;

void HashSetNew(hashset *h, int elemSize, int numBuckets,
		HashSetHashFunction hashfn, HashSetCompareFunction comparefn, HashSetFreeFunction freefn)
{
  assert(elemSize > 0 && numBuckets > 0);
  assert(hashfn != NULL && comparefn != NULL);
  h->elemSize = elemSize;
  h->numBuckets = numBuckets;
  h->hashFn = hashfn;
  h->compareFn = comparefn;
  h->freeFn = freefn;
  h->elems = malloc(sizeof(vector) * h->numBuckets);
  for(int i=0; i < h->numBuckets; i++)
    VectorNew(&h->elems[i], h->elemSize, NULL, 0);
}

void HashSetDispose(hashset *h)
{
  for(int i = 0; i< h->numBuckets; i++){
    VectorDispose(&h->elems[i]);
  }
  free(h->elems);
}

int HashSetCount(const hashset *h)
{ 
  int count = 0;
  for(int i = 0; i < h->numBuckets; i++){
      count += VectorLength(&h->elems[i]);
  }
  return count; 
}

void HashSetMap(hashset *h, HashSetMapFunction mapfn, void *auxData)
{
  assert(mapfn != NULL);
  for(int i = 0; i < h->numBuckets; i++){
    VectorMap(&h->elems[i], mapfn, auxData);
  }
}

void HashSetEnter(hashset *h, const void *elemAddr)
{
  assert(elemAddr != NULL);
  int bucket = h->hashFn(elemAddr, h->numBuckets);
  assert(bucket >= 0 && bucket < h->numBuckets);
  if(VectorLength(&h->elems[bucket]) != 0){
    int found = VectorSearch(&h->elems[bucket], elemAddr, h->compareFn, 0, false);
    if(found >= 0)
      VectorReplace(&h->elems[bucket], elemAddr, found);
    else
      VectorAppend(&h->elems[bucket], elemAddr);
  }
  else{
    VectorAppend(&h->elems[bucket], elemAddr);
  }
}

void *HashSetLookup(const hashset *h, const void *elemAddr)
{ 
  assert(elemAddr != NULL);
  int bucket = h->hashFn(elemAddr, h->numBuckets);
  assert(bucket >= 0 && bucket < h->numBuckets);
  void * elemFoundAt = NULL;
  if(VectorLength(&h->elems[bucket]) != 0){
    int found = VectorSearch(&h->elems[bucket], elemAddr, h->compareFn, 0, false);
    if(found >= 0){
      elemFoundAt = VectorNth(&h->elems[bucket], found);
    }
  }
  return elemFoundAt; 
}
