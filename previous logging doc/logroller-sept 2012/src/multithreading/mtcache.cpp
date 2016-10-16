#include <cassert>
#include <iostream>
#include <sstream>
#include <pthread.h>

const unsigned int array_size = 1024 * 1024 * 10;
volatile unsigned int current_index = 0;

typedef unsigned int array_type;
array_type * array = 0;

void write_in_array(unsigned int nb_write)
{
  for (unsigned int i = 0; i < nb_write; i++) {
    unsigned int index = __sync_fetch_and_add(&current_index, 1);
    if (index >= array_size) {
      if (index == array_size) {
	unsigned new_index = __sync_fetch_and_sub(&current_index, array_size);
	index = 0;
      } else {
	index -= array_size;
      }
    }

    array[index] = (array_type)i;
  }   
}

void write_in_array_once_per_cache_line(unsigned int nb_write)
{
  const unsigned int step = 64 / sizeof(array_type);
  nb_write /= step;
  unsigned max = array_size / step;

  for (unsigned int i = 0; i < nb_write; i++) {
    unsigned int index = __sync_fetch_and_add(&current_index, 1);
    if (index >= max) {
      if (index == max) {
	unsigned new_index = __sync_fetch_and_sub(&current_index, max);
	index = 0;
      } else {
	index -= max;
      }
    }
    array[index * step] = (array_type)i;
  }   
}

void write_in_array_cache_optimized(unsigned int nb_write)
{
  const unsigned int step = 64 / sizeof(array_type);
  nb_write /= step;
  unsigned max = array_size / step;

  for (unsigned int i = 0; i < nb_write; i++) {
    unsigned int index = __sync_fetch_and_add(&current_index, 1);
    if (index >= max) {
      if (index == max) {
	unsigned new_index = __sync_fetch_and_sub(&current_index, max);
	index = 0;
      } else {
	index -= max;
      }
    }
    for (unsigned int j = 0; j < step; ++j)
      array[index * step + j] = (array_type)j;
  }   
}

volatile bool start = false;

void* thread_proc(void * p)
{
  while (!start)
    usleep(10 * 1000);

  int n = *((int*)p);
  for (int i = 0; i < 10; i++) {
    if (n == 0)
      write_in_array(array_size);
    else if (n == 1)
      write_in_array_once_per_cache_line(array_size);
    else
      write_in_array_cache_optimized(array_size);
  }
  return 0;
}

using namespace std;
int main(int argc, char *argv[])
{
  int n = 0;
  if (argc > 1) {
    std::istringstream iss(argv[1]);
    if (!iss >> n) return -1;
  }
  array = new array_type[array_size];

  pthread_t t1, t2, t3;
  pthread_create(&t1, NULL, &thread_proc, &n);
  pthread_create(&t2, NULL, &thread_proc, &n);
  pthread_create(&t3, NULL, &thread_proc, &n);

  // commit the memory
  write_in_array(array_size);
  current_index = 0;

  start = true;
  thread_proc(&n); // thread 4

  pthread_join(t1, NULL);
  pthread_join(t2, NULL);
  pthread_join(t3, NULL);

  delete [] array;
}
