#include<stdio.h>
#include<stdlib.h>
#include<todo.h>

_Noreturn void TODORAW( const char *Function, const char *File, int Line )
{
        printf("Error: 1:%d:%s (%s) is unimplemented\n", Line, File, Function);
        exit( 1 );
} 
