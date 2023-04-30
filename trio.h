#include "spend_time.h"
#include <pthread.h>
#include <stdio.h>
typedef struct trio_t trio_t;
void init_trio(trio_t* t);
void trio_enter(trio_t* t, int my_type);
void trio_leave(trio_t* t, int my_type);