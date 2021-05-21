#ifndef COMMON_H_
#define COMMON_H_

void _panic(const char* f, const char* format, ...);
#define panic(...) _panic(__func__, __VA_ARGS__)

#endif
