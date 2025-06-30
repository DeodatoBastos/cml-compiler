int dummy(int a) {
    if (a == 0) {
        return 0;
    } else if (a < 0) {
        return -1;
    }

    return 1;
}

int main(void) {
    int i;
    i = 10;
    while (i >= 0) {
        output(dummy(i - 5));
    }
    return 0;
}
