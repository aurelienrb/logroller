#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>

const unsigned int data_size = 1024 * 1024 * 100;

volatile unsigned int current_index = 0;

volatile int start_threads = 0;

void* thread_proc(void* param)
{
  char * data;
  unsigned int index;

  while (!start_threads)
    usleep(1000 * 10);

  data = (char*) param;
  index = 0;
  /*__sync_fetch_and_add(&current_index, 1);*/

  while (index < data_size)
    {
      data[index] = 0;
      /*index = __sync_fetch_and_add(&current_index, 1);*/
      index = __sync_add_and_fetch(&index, 1);
    }

  return NULL;
}

void print_time_diff(struct timespec *t1, struct timespec *t2)
{
  double diff_sec = (t2->tv_sec - t1->tv_sec )
    + ( t2->tv_nsec - t1->tv_nsec ) / 1E9;
  printf("%.3lf\n", diff_sec);
}

int main()
{
  char *data1, *data2, *data3, *data4;
  unsigned int i;
  struct timespec begin, end;
  pthread_t t1, t2, t3, t4;

  data1 = malloc(data_size);
  data2 = malloc(data_size);
  data3 = malloc(data_size);
  data4 = malloc(data_size);

  /* warm up */
  for (i = 0; i < data_size; i++)
    {
      data1[i] = 0;
      data2[i] = 0;
      data3[i] = 0;
      data4[i] = 0;
    }

  /* chaque thread écrit dans son propre buffer */
  for (i = 0; i < 5; ++i)
    {
      start_threads = 0;
      pthread_create(&t1, NULL, &thread_proc, data1);
      pthread_create(&t2, NULL, &thread_proc, data2);
      pthread_create(&t3, NULL, &thread_proc, data3);
      pthread_create(&t4, NULL, &thread_proc, data4);
      sleep(1);

      current_index = 0;
      start_threads = 1;
      clock_gettime(CLOCK_MONOTONIC, &begin);
      pthread_join(t1, NULL);
      clock_gettime(CLOCK_MONOTONIC, &end);
      pthread_detach(t2);
      print_time_diff(&begin, &end);
    }
  printf("-----\n");
  /* chaque thread écrit dans le même buffer */
  for (i = 0; i < 5; ++i)
    {
      start_threads = 0;
      pthread_create(&t1, NULL, &thread_proc, data1);
      pthread_create(&t2, NULL, &thread_proc, data1);
      pthread_create(&t3, NULL, &thread_proc, data1);
      pthread_create(&t4, NULL, &thread_proc, data1);
      sleep(1);

      current_index = 0;
      start_threads = 1;
      clock_gettime(CLOCK_MONOTONIC, &begin);
      pthread_join(t1, NULL);
      clock_gettime(CLOCK_MONOTONIC, &end);
      pthread_detach(t2);
      print_time_diff(&begin, &end);
    }

  return 0;
}
