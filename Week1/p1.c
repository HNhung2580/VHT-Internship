#include<stdio.h>
#include<pthread.h>
#include<stdint.h>
#include<time.h>
#include<stdlib.h>
#include<string.h>
#include<stdbool.h>
#include<unistd.h>

#define MAX 1000000000
struct timespec ts;
struct timespec tt,timerqst;

int co = 1;
long frequency = 1000000;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;  

// SAMPLE
void *sample_func(void *arg)
{   
  clock_gettime(CLOCK_REALTIME, &timerqst); 

  while(co == 1) 
  {      
    clock_gettime(CLOCK_REALTIME, &ts);  
    
    long tmp;
    if(timerqst.tv_nsec + frequency > MAX)
    {
      tmp = timerqst.tv_nsec;
      timerqst.tv_nsec = tmp + frequency - MAX;
      timerqst.tv_sec++;

      if(clock_nanosleep(CLOCK_REALTIME, TIMER_ABSTIME, &timerqst, NULL) != 0) co = 0;
      else co = 1;
    }
    else
    {
      timerqst.tv_nsec += frequency;
      if(clock_nanosleep(CLOCK_REALTIME, TIMER_ABSTIME, &timerqst, NULL) != 0) co = 0;
      else co = 1;
      }   
    }
}

// INPUT
void *input_func(void *arg)
{
  while(1)
  {
    pthread_mutex_lock(&mutex);
    FILE *f;
    f = fopen("freq.txt","r");

    int frequency_new = 0;
    fscanf(f,"%d", &frequency_new);
    fclose(f); 

    if(frequency == frequency_new) pthread_mutex_unlock(&mutex);
    else
    {
      frequency = frequency_new;
      pthread_mutex_unlock(&mutex);
    }
  }
}

//LOGGING
void *logging_func(void *arg)
{
  while(1)
  {
    FILE *f;
    f = fopen("time_and_interval.txt","a+");
    long diff_sec = ((long) ts.tv_sec) - tt.tv_sec ;
    long diff_nsec;

    if(tt.tv_nsec != ts.tv_nsec || tt.tv_sec != ts.tv_sec)
    {
      if(ts.tv_nsec > tt.tv_nsec)
        {
          diff_nsec = ts.tv_nsec - tt.tv_nsec;
        }
      else 
        {
          diff_nsec = MAX + ts.tv_nsec - tt.tv_nsec ;
          diff_sec = diff_sec - 1;
        }
       fprintf(f,"\n%ld.%09ld",diff_sec,diff_nsec);  
       tt.tv_nsec =ts.tv_nsec;
       tt.tv_sec = ts.tv_sec;

    } 
    fclose(f);
  } 
}

int main()
{ 
  pthread_t SAMPLE;
  pthread_t INPUT;
  pthread_t LOGGING;
  
  tt.tv_sec = 0;
  tt.tv_nsec = 0;
    
  pthread_mutex_init(&mutex, NULL); 
  
  pthread_create(&SAMPLE, NULL, sample_func,NULL);
  pthread_create(&INPUT,NULL,input_func,NULL);
  pthread_create(&LOGGING,NULL,logging_func,NULL);      
       
  pthread_join(INPUT,NULL);
  pthread_join(SAMPLE,NULL);
  pthread_join(LOGGING,NULL);

  pthread_mutex_destroy(&mutex);

  return 0;
    
}
