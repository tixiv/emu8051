
#include "plot.h"
#include <string.h>

void plot_init(plot_t * plot){
    memset(plot, 0, sizeof(plot_t));

    //plot->gp = popen("gnuplot -persistent", "w");
    plot->gp = popen("gnuplot", "w");

    fprintf(plot->gp, "set yrange [-0.5:2]\n");
    //fprintf(plot->gp, "set y2range [0:1000]\n");
}

void plot_update(plot_t *plot, double value) {
    if (plot->delay == 0)
    {
        plot->delay = 10000;

        plot->buf[plot->i % N] = value;

        fprintf(plot->gp,
            "plot '-' with lines axes x1y1 title 'akk'" 
            // ", '-' with lines axes x1y2 title 'counter'"
            "\n");

        for (int j = 0; j < N; j++) {
            int idx = (plot->i + j) % N;
            fprintf(plot->gp, "%d %f\n", j, plot->buf[idx]);
        }
        fflush(plot->gp);
        fprintf(plot->gp, "e\n");

        plot->i++;
    }
    else
    {
        plot->delay--;
    }
}
