#ifndef TODO_H
#define TODO_H

_Noreturn void TODORAW(const char *Function, const char *File, int Line);
#define TODO() TODORAW(__func__, __FILE__, __LINE__)

#endif
