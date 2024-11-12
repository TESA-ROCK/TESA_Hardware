#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    FILE *fp;
    char buffer[100];
    fp = popen("./hello.py", "r");
    if (fp == NULL) {
        printf("Error running Python code");
        exit(1);
    }
    if (fgets(buffer, 100, fp) != NULL) {
        printf("Got %s\n", buffer);
    }
}