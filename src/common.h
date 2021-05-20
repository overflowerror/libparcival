#ifndef COMMON_H
#define COMMON_H

void _panic(const char* f, const char* format, ...);
#define panic(...) _panic(__func__, __VA_ARGS__)

#endif
