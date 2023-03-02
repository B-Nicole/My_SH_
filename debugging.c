#include <stdio.h>

// https://stackoverflow.com/questions/1644868/define-macro-for-debug-printing-in-c
#define dbg(fmt, ...) \
        do { fprintf(stderr, "%s:%d:%s(): " fmt, __FILE__, \
            __LINE__, __func__, __VA_ARGS__); } while (0)


int main() {
    dbg("%s", "testing 1 2");
    return 0;
}


// valgrind ./nameofexecutable