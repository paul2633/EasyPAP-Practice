#include <omp.h>
#include <stdio.h>

int main ()
{
#pragma omp parallel
#pragma omp critical
  {
    printf ("Bonjour ! (thread n°%d)\n", omp_get_thread_num ());
    // #pragma omp barrier
    printf ("Au revoir ! (thread n°%d)\n", omp_get_thread_num ());
  }
  return 0;
}
