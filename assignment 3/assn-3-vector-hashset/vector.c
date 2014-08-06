#include "vector.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <search.h>

#define kInitialAllocation 4  //default allocLength for a vector

void VectorNew(vector *v, int elemSize, VectorFreeFunction freeFn, int initialAllocation)
{
  assert(elemSize > 0 && initialAllocation >= 0);
  v->elemSize = elemSize;
  v->logLength = 0;
  if(initialAllocation > 0)
    v->allocLength = initialAllocation;
  else
    v->allocLength = kInitialAllocation;
  v->elems = malloc(v->allocLength * elemSize);
  v->freeFn = freeFn;
  assert(v->elems != NULL);
}

void VectorDispose(vector *v)
{
  if(v->freeFn != NULL){
    for(int i = 0; i < v->logLength; i++){
      v->freeFn((char *) v->elems + i * v->elemSize);
    }
  }

  free(v->elems);
}

int VectorLength(const vector *v)
{ 
  return v->logLength; 
}

void *VectorNth(const vector *v, int position)
{
  //printf("In Nth function\n");
  assert(position >= 0 && position < v->logLength);
  //printf("After assert\n");
  //elem is the pointer pointing to the element at position.
  //the numbering begins with 0.
  void * elem = ((char *)v->elems + (position * v->elemSize));
  //printf("Elem found\n");
  return elem;
}

void expandVectorSize(vector * v){
  v->allocLength *= 2;
  v->elems = realloc(v->elems, v->allocLength * v->elemSize);
  assert(v->elems != NULL);
}

void VectorInsert(vector *v, const void *elemAddr, int position)
{
  assert(position >= 0 && position <= v->logLength);
  
  if(v->logLength == v->allocLength){
    expandVectorSize(v);
  }

  void * addrToInsert = (char *)v->elems + position * v->elemSize; //pointer pointing to the address of the location where the element is to be inserted.
  void * nextAddr = (char *)v->elems + (position+1) * v->elemSize;  //pointer pointing to the next address in the vector list

  int nBytesToMove = (v->logLength - position) * v->elemSize;  //number of bytes to shift to make space for new element.

  memmove(nextAddr, addrToInsert, nBytesToMove);
  memcpy(addrToInsert, elemAddr, v->elemSize);
  v->logLength++;
}

void VectorAppend(vector *v, const void *elemAddr)
{
  if(v->logLength == v->allocLength){
    expandVectorSize(v);
  }
  void * endAddr = (char *)v->elems + v->logLength * v->elemSize;
  memcpy(endAddr, elemAddr, v->elemSize);
  v->logLength++;
}

void VectorReplace(vector *v, const void *elemAddr, int position)
{
  assert(position >= 0 && position < v->logLength);
  void * posAddr = (char *)v->elems + position * v->elemSize;
  //If a free function is provided, free the contents pointed to at that position and replace.
  if(v->freeFn != NULL){
    v->freeFn(posAddr);
  }
  
  memcpy(posAddr, elemAddr, v->elemSize);
}

void VectorDelete(vector *v, int position)
{
  assert(position >= 0 && position < v->logLength);
  void * sourceAddr = (char *)v->elems + (position * v->elemSize);
  if(v->freeFn != NULL){
     v->freeFn(sourceAddr);
  }
  void * moveAddr = (char *)sourceAddr + v->elemSize; //getting the pointer to the next element.
  int bytesToMove = v->elemSize * (v->logLength - (position + 1)); //calculating the number of bytes to move.
  memmove(sourceAddr, moveAddr, bytesToMove);
  v->logLength--;
}

void VectorSort(vector *v, VectorCompareFunction compare)
{
  assert(compare != NULL);
  //printf("Entering qsort\n");
  qsort(v->elems, v->logLength, v->elemSize, compare);
  //printf("After qsort\n");
}

void VectorMap(vector *v, VectorMapFunction mapFn, void *auxData)
{
  assert(mapFn != NULL);
  for(int i = 0; i < v->logLength; i++){
    void * elemPos = (char *) v->elems + i * v->elemSize;
    mapFn(elemPos, auxData);
  }
  
}


static const int kNotFound = -1;
int VectorSearch(const vector *v, const void *key, VectorCompareFunction searchFn, int startIndex, bool isSorted)
{ 
  //printf("Beginning search\n");
  assert(startIndex >= 0 && startIndex < v->logLength);
  assert(key != NULL && searchFn != NULL);
  //printf("After assert in search\n");
  void * startAddr = (char *)v->elems + startIndex * v->elemSize; //If startIndex is 0, we start from the beginning.
  void * elem = NULL;
  size_t numElems = v->logLength - startIndex;

  if(isSorted){
    elem = bsearch(key, startAddr, v->logLength - startIndex, v->elemSize, searchFn);
  }
  else{
    elem = lfind(key, startAddr, &numElems, v->elemSize, searchFn);
  }

  if(elem != NULL)
     return ((char *)elem - (char *)v->elems) / v->elemSize;
  else
    return kNotFound;
} 

