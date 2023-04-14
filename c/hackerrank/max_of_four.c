#include <stdio.h>
/*
Add `int max_of_four(int a, int b, int c, int d)` here.
*/

int max_of_two(int a, int b) {
    if (a > b) {
        return a;
    } else {
        return b;
    }
}

int max_of_four(int a, int b, int c, int d) {
    if (max_of_two(a, b) > max_of_two(c, d)) {
        return max_of_two(a, b);
    } else {
        return max_of_two(c, d);
    }
}

int main() {
    int a, b, c, d;
    scanf("%d %d %d %d", &a, &b, &c, &d);
    int ans = max_of_four(a, b, c, d);
    printf("%d", ans);
    
    return 0;
}

