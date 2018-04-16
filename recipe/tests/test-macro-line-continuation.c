#include <stdio.h>

#define makro() "Hello " \
                "World"

void main() {
    printf(makro());
}
