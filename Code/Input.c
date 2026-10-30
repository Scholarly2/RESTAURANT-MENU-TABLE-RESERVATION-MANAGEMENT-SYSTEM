#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <math.h>
#include "../Include_type_struct/RestaurantTypes.h"
#include "../Include_type_struct/Input.h"

int parseDouble(const char *text, double min, double max, double *value)
{
    char *endptr;
    double number;

    if (text == NULL || value == NULL)
        return FALSE;

    number = strtod(text, &endptr);

    if (text == endptr || *endptr != '\0')
        return FALSE;

    if (!isfinite(number))
        return FALSE;

    if (number < min || number > max)
        return FALSE;

    *value = number;
    return TRUE;
}

int readDouble(const char *prompt, double min, double max, double *value)
{
    char text[100];
    double number;

    if (prompt == NULL || value == NULL)
        return FALSE;

    printf("%s", prompt);

    if (fgets(text, sizeof(text), stdin) == NULL)
        return FALSE;

    text[strcspn(text, "\n")] = '\0';

    if (!parseDouble(text, min, max, &number))
        return FALSE;

    *value = number;
    return TRUE;
}

int readBookingTime(DateTime *value)
{
    DateTime temp;

    if (value == NULL)
        return FALSE;

    printf("Enter booking date and time (YYYY MM DD HH MM): ");

    if (scanf("%d %d %d %d %d",
              &temp.year,
              &temp.month,
              &temp.day,
              &temp.hour,
              &temp.minute) != 5)
    {
        return FALSE;
    }

    if (!isValidBookingTime(temp))
        return FALSE;

    *value = temp;
    return TRUE;
}

int nextID(int lastID)
{
    return lastID + 1;
}

int readPhone(char *phone)
{
    char text[PHONE_SIZE];

    if (phone == NULL)
        return FALSE;

    printf("Enter phone number: ");

    if (fgets(text, sizeof(text), stdin) == NULL)
        return FALSE;

    text[strcspn(text, "\n")] = '\0';
    trim(text);

    if (!isValidPhone(text))
        return FALSE;

    strcpy(phone, text);

    return TRUE;
}

int isValidBookingTime(DateTime value)
{
    time_t bookingTime;
    time_t currentTime;

    if (!toTimestamp(value, &bookingTime))
        return FALSE;

    time(&currentTime);

    if (bookingTime <= currentTime)
        return FALSE;

    return TRUE;
}

int isValidProductName(const char *name)
{
    int i;
    int hasCharacter = FALSE;

    if (name == NULL || name[0] == '\0')
        return FALSE;

    for (i = 0; name[i] != '\0'; i++)
    {
        if (!isspace((unsigned char)name[i]))
        {
            hasCharacter = TRUE;

            if (!isalpha((unsigned char)name[i]) &&
                !isdigit((unsigned char)name[i]) &&
                name[i] != ' ' &&
                name[i] != '-')
            {
                return FALSE;
            }
        }
    }

    return hasCharacter;
}

int isValidCustomerName(const char *name)
{
    int i;

    if (name == NULL || name[0] == '\0')
        return FALSE;

    for (i = 0; name[i] != '\0'; i++)
    {
        if (!isalpha((unsigned char)name[i]) && name[i] != ' ')
            return FALSE;
    }

    return TRUE;
}

int readPrice(const char *prompt, double *price)
{
    char text[100];

    if (prompt == NULL || price == NULL)
        return FALSE;

    printf("%s", prompt);

    if (fgets(text, sizeof(text), stdin) == NULL)
        return FALSE;

    text[strcspn(text, "\n")] = '\0';

    return parsePrice(text, price);
}

void trim(char *str)
{
    int start = 0;
    int end;
    int i;

    if (str == NULL) return;

    while (isspace((unsigned char)str[start]))
    {
        start++;
    }

    end = (int)strlen(str) - 1;

    while (end >= start && isspace((unsigned char)str[end]))
    {
        end--;
    }

    for (i = 0; i <= end - start; i++)
    {
        str[i] = str[start + i];
    }

    str[i] = '\0';
}

int readName(const char *prompt, int customer, char *name)
{
    if (prompt == NULL || name == NULL)
        return FALSE;

    printf("%s", prompt);

    if (fgets(name, NAME_SIZE, stdin) == NULL)
        return FALSE;

    name[strcspn(name, "\n")] = '\0';
    trim(name);

    if (isEmpty(name))
        return FALSE;

    if (customer == TRUE)
        return isValidCustomerName(name);

    return isValidProductName(name);
}

int isEmpty(const char *text)
{
    int i;

    if (text == NULL) return TRUE;

    for (i = 0; text[i] != '\0'; i++)
    {
        if (!isspace((unsigned char)text[i]))
        {
            return FALSE;
        }
    }

    return TRUE;
}

void getString(char *buffer, int size)
{
    if (buffer == NULL || size <= 0) return;

    if (fgets(buffer, size, stdin) != NULL)
    {
        buffer[strcspn(buffer, "\n")] = '\0';
    }
}

int isValidUsername(const char *username) {
    if (isEmpty(username)) {
        printf("Error: Username cannot be empty.\n");
        return 0;
    }

    int len = strlen(username);
    if (len < 2 || len > 25) {
        printf("Error: Username length must be between 2 and 25 characters.\n");
        return 0;
    }

    for (int i = 0; i < len; i++) {
        if (!isalnum((unsigned char)username[i])) {
            printf("Error: Username can only contain letters and numbers.\n");
            return 0;
        }
    }

    return 1;
}

int isValidPassword(const char *password) {
    if (isEmpty(password)) {
        printf("Error: Password cannot be empty.\n");
        return 0;
    }

    int len = strlen(password);
    if (len < 6 || len > 20) {
        printf("Error: Password length must be between 6 and 20 characters.\n");
        return 0;
    }

    int hasUpper = 0, hasLower = 0, hasDigit = 0;
    for (int i = 0; i < len; i++) {
        unsigned char c = (unsigned char)password[i];
        if (isupper(c)) hasUpper = 1;
        if (islower(c)) hasLower = 1;
        if (isdigit(c)) hasDigit = 1;
    }

    int valid = 1;
    if (!hasUpper || !hasLower || !hasDigit ) {
        printf("Invalid! Password must contain at least one uppercase letter, one lowercase letter and one digit.\n");
        valid = 0;
    }

    return valid;
}

int toTimestamp(DateTime value, time_t *result)
{
    struct tm timeInfo;
    time_t timestamp;

    if (result == NULL) {
        return FALSE;
    }

    if (value.year < 1900 ||
        value.month < 1 || value.month > 12 ||
        value.day < 1 || value.day > 31 ||
        value.hour < 0 || value.hour > 23 ||
        value.minute < 0 || value.minute > 59) {
        return FALSE;
    }

    timeInfo.tm_year = value.year - 1900;
    timeInfo.tm_mon = value.month - 1;
    timeInfo.tm_mday = value.day;
    timeInfo.tm_hour = value.hour;
    timeInfo.tm_min = value.minute;
    timeInfo.tm_sec = 0;
    timeInfo.tm_isdst = -1;

    timestamp = mktime(&timeInfo);

    if (timestamp == (time_t)-1) {
        return FALSE;
    }

    /* mktime có thể tự điều chỉnh ngày không tồn tại,
       nên cần kiểm tra lại các thành phần ngày giờ. */
    if (timeInfo.tm_year != value.year - 1900 ||
        timeInfo.tm_mon != value.month - 1 ||
        timeInfo.tm_mday != value.day ||
        timeInfo.tm_hour != value.hour ||
        timeInfo.tm_min != value.minute) {
        return FALSE;
    }

    *result = timestamp;
    return TRUE;
}
int isValidPrice(double price)
{
    if (!isfinite(price)) {
        return FALSE;
    }

    if (price < 0) {
        return FALSE;
    }

    return TRUE;
}

int isValidPhone(const char *phoneNumber)
{
    int i;
    int length;

    if (phoneNumber == NULL) {
        return FALSE;
    }

    length = strlen(phoneNumber);

    if (length < 10 || length > 11) {
        return FALSE;
    }

    for (i = 0; i < length; i++) {
        if (!isdigit((unsigned char)phoneNumber[i])) {
            return FALSE;
        }
    }

    return TRUE;
}

int parsePrice(const char *text, double *price)
{
    char *endptr;
    double value;

    if (text == NULL || price == NULL || text[0] == '\0') {
        return FALSE;
    }

    value = strtod(text, &endptr);

    // Không chuyển đổi được thành số
    if (text == endptr) {
        return FALSE;
    }

    // Không cho phép ký tự thừa
    while (isspace((unsigned char)*endptr)) {
        endptr++;
    }

    if (*endptr != '\0') {
        return FALSE;
    }

    // Giá không được âm hoặc vượt quá giới hạn số
    if (!isfinite(value) || value < 0) {
        return FALSE;
    }

    *price = value;
    return TRUE;
}

int parseInt(const char *text, int min, int max, int *value)
{
    char *endptr;
    long number;

    if (text == NULL || value == NULL) {
        return FALSE;
    }

    number = strtol(text, &endptr, 10);

    // Kiểm tra chuỗi rỗng hoặc không phải số
    if (text == endptr || *endptr != '\0') {
        return FALSE;
    }

    // Kiểm tra giới hạn
    if (number < min || number > max) {
        return FALSE;
    }

    *value = (int)number;
    return TRUE;
}

int isValidName(const char *name)
{
    int i;
    int hasLetter = FALSE;

    if (name == NULL || name[0] == '\0') {
        return FALSE;
    }

    for (i = 0; name[i] != '\0'; i++) {
        if (isalpha((unsigned char)name[i])) {
            hasLetter = TRUE;
        }
        else if (name[i] != ' ') {
            return FALSE;
        }
    }

    return hasLetter;
}

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

int readInt(const char *prompt,int min, int max, int *value)
{
    char buf[NAME_SIZE];
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