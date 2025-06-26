// #include <stdio.h>
// #include <stdlib.h>

/*
 *
 *
 *
 * */

int main(void) {
    int x;
    int i;
    x = 1;

    while (i < 10) {
        i = i + 1;
        x = x * i;

        if (i == 5) {
            output(x);
        } else if (i > 6) {
            output(i);
        } else if (i >= 7) {
            output(10);
        } else if (i < 0) {
            output(1);
        } else if (i <= 0) {
            output(i);
        } else if (i != 2) {
            output(x);
        }
    }

    output(x);

    return 0;
}
