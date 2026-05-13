
#ifndef PLOT_H
#define PLOT_H

#include <stdio.h>

#define N 256

typedef struct {
    FILE *gp;
    int delay;
    double buf[N];
    int i;
} plot_t;

void plot_init(plot_t * plot);
void plot_update(plot_t *plot, double value);

#endif
