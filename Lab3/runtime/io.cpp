#include <stdio.h>
#include <stdlib.h>
#include "io.h"

void input(int *i)
{
    scanf("input:%d", i);
}


void output(int *i)
{
    printf("output:%d\n", *i);
}

void obc_check_error(int *line,int *pos,char* str){
    //char* str = "obc array: [a]";
    printf("%s [OutBound Check Error] at Line:%d, Pos:%d\n",str,*line,*pos);
    exit(-1);
}
