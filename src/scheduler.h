#ifndef _SCHEDULER_H_
#define _SCHEDULER_H_

typedef struct {
  /* period in ticks */
  int period;
  /* ticks until next activation */
  int delay;
  /* function pointer */
  void (*func)(void);
  /* activation counter */
  int exec;
} Sched_Task_t;

void Sched_Init(void);
void Sched_Schedule(void);
void Sched_Dispatch(void);

int Sched_AddT(void (*f)(void), int d, int p, int pri);

#endif
