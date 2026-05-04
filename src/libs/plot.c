#include "../core/axton.h"
#include <stdio.h>
#include <math.h>

typedef struct plot {
    char *title;
    char *xlabel;
    char *ylabel;
    object *series;
} plot;

object *pltfigure(void) {
    plot *p = malloc(sizeof(plot));
    p->title = strdup("");
    p->xlabel = strdup("");
    p->ylabel = strdup("");
    p->series = makelist();
    object *obj = makenative(p, NULL);
    obj->type = 61;
    return obj;
}

object *plttitle(object *fig, char *title) {
    plot *p = (plot*)fig->native.data;
    free(p->title);
    p->title = strdup(title);
    return fig;
}

object *pltxlabel(object *fig, char *label) {
    plot *p = (plot*)fig->native.data;
    free(p->xlabel);
    p->xlabel = strdup(label);
    return fig;
}

object *pltylabel(object *fig, char *label) {
    plot *p = (plot*)fig->native.data;
    free(p->ylabel);
    p->ylabel = strdup(label);
    return fig;
}

object *pltplot(object *fig, object *x, object *y, char *label) {
    plot *p = (plot*)fig->native.data;
    object *series = makedict();
    dictset(series, makestring("x"), x);
    dictset(series, makestring("y"), y);
    if (label) dictset(series, makestring("label"), makestring(label));
    listappend(p->series, series);
    return fig;
}

object *pltscatter(object *fig, object *x, object *y, char *label) {
    return pltplot(fig, x, y, label);
}

object *plthist(object *fig, object *data, int bins) {
    plot *p = (plot*)fig->native.data;
    if (data->type != 5) throwexception("hist needs list");
    float minv = 1e100, maxv = -1e100;
    for (int i = 0; i < data->list.count; i++) {
        float val = (data->list.items[i]->type == 0) ? data->list.items[i]->ival : data->list.items[i]->fval;
        if (val < minv) minv = val;
        if (val > maxv) maxv = val;
    }
    float binwidth = (maxv - minv) / bins;
    int *counts = calloc(bins, sizeof(int));
    for (int i = 0; i < data->list.count; i++) {
        float val = (data->list.items[i]->type == 0) ? data->list.items[i]->ival : data->list.items[i]->fval;
        int bin = (int)((val - minv) / binwidth);
        if (bin >= bins) bin = bins - 1;
        if (bin >= 0) counts[bin]++;
    }
    object *binedges = makelist();
    object *bincounts = makelist();
    for (int i = 0; i <= bins; i++) {
        listappend(binedges, makefloat(minv + i * binwidth));
    }
    for (int i = 0; i < bins; i++) {
        listappend(bincounts, makeint(counts[i]));
    }
    dictset(p->series, makestring("hist_edges"), binedges);
    dictset(p->series, makestring("hist_counts"), bincounts);
    free(counts);
    return fig;
}

object *pltshow(object *fig) {
    plot *p = (plot*)fig->native.data;
    printf("\n=== PLOT ===\n");
    printf("Title: %s\n", p->title);
    printf("XLabel: %s\n", p->xlabel);
    printf("YLabel: %s\n", p->ylabel);
    for (int i = 0; i < p->series->list.count; i++) {
        object *series = p->series->list.items[i];
        object *x = dictget(series, makestring("x"));
        object *y = dictget(series, makestring("y"));
        object *label = dictget(series, makestring("label"));
        if (x && y) {
            printf("Series %d: %s\n", i, label ? label->sval : "");
            int n = (x->list.count < y->list.count) ? x->list.count : y->list.count;
            for (int j = 0; j < n && j < 10; j++) {
                printf("  (%g, %g)\n", 
                    (x->list.items[j]->type == 0) ? x->list.items[j]->ival : x->list.items[j]->fval,
                    (y->list.items[j]->type == 0) ? y->list.items[j]->ival : y->list.items[j]->fval);
            }
            if (n > 10) printf("  ...\n");
        }
    }
    if (dictget(p->series, makestring("hist_edges"))) {
        object *edges = dictget(p->series, makestring("hist_edges"));
        object *counts = dictget(p->series, makestring("hist_counts"));
        printf("Histogram:\n");
        for (int i = 0; i < counts->list.count; i++) {
            printf("  %g - %g: %ld\n",
                (edges->list.items[i]->type == 0) ? edges->list.items[i]->ival : edges->list.items[i]->fval,
                (edges->list.items[i+1]->type == 0) ? edges->list.items[i+1]->ival : edges->list.items[i+1]->fval,
                counts->list.items[i]->ival);
        }
    }
    printf("============\n");
    return fig;
}

object *pltsavefig(object *fig, char *path) {
    return pltshow(fig);
}

void registerplotlib(environment *env) {
    object *pltmod = makemodule("plt", NULL);
    envset(pltmod->module.exports, "figure", makebuiltin(pltfigure), 0);
    envset(pltmod->module.exports, "title", makebuiltin(plttitle), 0);
    envset(pltmod->module.exports, "xlabel", makebuiltin(pltxlabel), 0);
    envset(pltmod->module.exports, "ylabel", makebuiltin(pltylabel), 0);
    envset(pltmod->module.exports, "plot", makebuiltin(pltplot), 0);
    envset(pltmod->module.exports, "scatter", makebuiltin(pltscatter), 0);
    envset(pltmod->module.exports, "hist", makebuiltin(plthist), 0);
    envset(pltmod->module.exports, "show", makebuiltin(pltshow), 0);
    envset(pltmod->module.exports, "savefig", makebuiltin(pltsavefig), 0);
    envset(env, "plt", pltmod, 0);
}
