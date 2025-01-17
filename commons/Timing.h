#pragma once

#ifdef __APPLE__

#include <sys/time.h>

typedef struct timeval timestamp_type;

static void get_timestamp(timestamp_type *t) {
  gettimeofday(t, NULL);
}

static double timestamp_diff_in_seconds(timestamp_type start,
                                        timestamp_type end) {
  /* Perform the carry for the later subtraction by updating start. */
  if (end.tv_usec < start.tv_usec) {
    int nsec = (start.tv_usec - end.tv_usec) / 1000000 + 1;
    start.tv_usec -= 1000000 * nsec;
    start.tv_sec += nsec;
  }
  if (end.tv_usec - start.tv_usec > 1000000) {
    int nsec = (end.tv_usec - start.tv_usec) / 1000000;
    start.tv_usec += 1000000 * nsec;
    start.tv_sec -= nsec;
  }

  return end.tv_sec - start.tv_sec + (end.tv_usec - start.tv_usec) * 1e-6;
}

static double timestamp_diff_in_milliseconds(timestamp_type start,
                                        timestamp_type end) {
  /* Perform the carry for the later subtraction by updating start. */
  if (end.tv_usec < start.tv_usec) {
    int nsec = (start.tv_usec - end.tv_usec) / 1000000 + 1;
    start.tv_usec -= 1000000 * nsec;
    start.tv_sec += nsec;
  }
  if (end.tv_usec - start.tv_usec > 1000000) {
    int nsec = (end.tv_usec - start.tv_usec) / 1000000;
    start.tv_usec += 1000000 * nsec;
    start.tv_sec -= nsec;
  }

  return (end.tv_sec - start.tv_sec) * 1000 + (end.tv_usec - start.tv_usec) * 1e-3;
}

#else

#include <time.h>

typedef struct timespec timestamp_type;

static void get_timestamp(timestamp_type *t) {
  clock_gettime(CLOCK_MONOTONIC_RAW, t);
}

static double timestamp_diff_in_seconds(timestamp_type start,
                                        timestamp_type end) {
  struct timespec temp;
  if ((end.tv_nsec - start.tv_nsec) < 0) {
    temp.tv_sec = end.tv_sec - start.tv_sec - 1;
    temp.tv_nsec = 1000000000 + end.tv_nsec - start.tv_nsec;
  } else {
    temp.tv_sec = end.tv_sec - start.tv_sec;
    temp.tv_nsec = end.tv_nsec - start.tv_nsec;
  }
  return (temp.tv_sec + 1e-9 * temp.tv_nsec);
}

static double timestamp_diff_in_milliseconds(timestamp_type start,
                                        timestamp_type end) {
  struct timespec temp;
  if ((end.tv_nsec - start.tv_nsec) < 0) {
    temp.tv_sec = end.tv_sec - start.tv_sec - 1;
    temp.tv_nsec = 1000000000 + end.tv_nsec - start.tv_nsec;
  } else {
    temp.tv_sec = end.tv_sec - start.tv_sec;
    temp.tv_nsec = end.tv_nsec - start.tv_nsec;
  }
  return (temp.tv_sec * 1000 + 1e-6 * temp.tv_nsec);
}

#endif
