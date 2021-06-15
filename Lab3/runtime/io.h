#ifndef _SAFEC_RUNTIME__IO_H
#define _SAFEC_RUNTIME__IO_H

#ifdef __cplusplus
extern "C" {
#endif

void input(int *);
void output(int *);

void obc_check_error(int *,int *,char*);
#ifdef __cplusplus
}
#endif

#endif
