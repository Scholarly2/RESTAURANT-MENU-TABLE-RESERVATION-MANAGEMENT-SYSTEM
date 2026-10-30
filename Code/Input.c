#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <limits.h>

#include "RestaurantTypes.h"
#include "Input.h"

int isValidNumber(const char *str)
{
    int i = 0;

    if (str == NULL || str[0] == '\0') return FALSE;
    if (str[0] == '-' || str[0] == '+') i++;
    if (str[i] == '\0') return FALSE;

    for (; str[i] != '\0'; i++) {
        if (!isdigit((unsigned char)str[i])) return FALSE;
    }
    return TRUE;
}

int readInt(const char *prompt, int *value)
{
    char buf[LINE_SIZE];
    long v;

    printf("%s", prompt);
    if (fgets(buf, sizeof(buf), stdin) == NULL) return -1;

    if (strchr(buf, '\n') == NULL) {
        int ch;
        while ((ch = getchar()) != '\n' && ch != EOF) { }
    }
    buf[strcspn(buf, "\r\n")] = '\0';

    if (!isValidNumber(buf)) return FALSE;

    errno = 0;
    v = strtol(buf, NULL, 10);
    if (errno == ERANGE || v > INT_MAX || v < INT_MIN) return FALSE;

    *value = (int)v;
    return TRUE;
}