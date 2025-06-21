// #include <stdio.h>
// #include <stdlib.h>

int main(void) {
    int x;
    int i;
    x = 1;

    while (i < 10) {
        i = i + 1;
        x = x * i;
    }

    output(x);

    return 0;
}
