#include "Input.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <errno.h>

#define INPUT_BUFFER_SIZE 1256

//check input product item - UYEN

//doc dong
int readLine(char *text, int size)
{
    char *newline;
    size_t length;
    int ch;
    
    if (text == NULL || size < 2) 
    {
        return FALSE;
    }

    for (;;)
    {
        if (fgets(text, size, stdin) == NULL)
        {
            return FALSE;
        }

        newline = strchr(text, '\n');
        if (newline != NULL)
        {
            *newline = '\0';
        }
        else 
        {
            ch = getchar();
            if (ch == '\r') 
            {
                ch = getchar();
            }
            if(ch != '\n' && ch != EOF)
            {
                while (ch != '\n' && ch != EOF)
                {
                    ch = getchar();
                }
                if (ferror(stdin)) 
                {
                    return FALSE;
                }
                printf("Input too long. Please enter a shorter input.\n");
                fflush(stdout);
                continue;
            }
            if (ferror(stdin)) 
            {
                return FALSE;
            }

            length = strlen(text);
            if (length > 0 && text[length -1] == '\0')
            {
                text[length -1] = '\0';
            }
            return TRUE;
        }

    }

}

//chuyen chuoi thanh so nguyen
int praseInt(const char *text, int min, int max, int *value)
{
    char *end;
    long number;
    
    if (text == NULL || value == NULL || min > max )
    {
        return FALSE;
    }
    errno = 0;
    number = strtol(text, &end, 10);
    if (end == text || errno == ERANGE)
    {
        return FALSE;
    }

    while (isspace((unsigned char)*end))
    {
        end++;
    }
    if (*end != '\0' || number < min || number > max)
    {
        return FALSE;
    }
    *value = (int)number;
    return TRUE;
}

//chuyen chuoi thanh so thuc
int parseDouble(const char *text, double min, double max, double *value)
{
    char *end;
    double number;

    if(text == NULL || value == NULL || !isfinite(min) ||
       !isfinite(max) || min > max)
    {
        return FALSE;
    }

    errno = 0;
    number = strtod(text, &end);
    if (end == text || errno == ERANGE || !isfinite(number))
    {
        return FALSE;
    }

    while (isspace((unsigned char)*end))
    {
        end++;
    }

    if (*end != '\0' || number < min || number > max)
    {
        return FALSE;
    }

    *value = number;
    return TRUE;
}

int readInt(const char *prompt, int min, int max, int *value)
{
   char text[INPUT_BUFFER_SIZE];
   if (prompt == NULL || value == NULL || min > max)
   {
       return FALSE;
   }
   for (;;)
   {
     printf("%s", prompt);
     fflush(stdout);
     if (!readLine(text, sizeof(text)))
     {
        printf("Error reading input. Please try again.\n");
        return FALSE;
     }
     if(parseInt(text, min, max, value))
     {
        return TRUE;
     }
     else
     {
        printf("Invalid input. Please enter an integer between %d and %d.\n", min, max);
     }
   }
}

//nhap so thuc trong khoang min max
int readDouble(const char *prompt, double min, double max, double *value)
{
    char text[INPUT_BUFFER_SIZE];
    if (prompt == NULL || value == NULL || !isfinite(min) || !isfinite(max) || min > max)
    {
        return FALSE;
    }
    for (;;)
    {
        printf("%s", prompt);
        fflush(stdout);
        if (!readLine(text, sizeof(text)))
        {
            printf("Error reading input. Please try again.\n");
            return FALSE;
        }
        if (parseDouble(text, min, max, value))
        {
            return TRUE;
        }
        else
        {
            printf("Invalid input. Please enter a number between %.2f and %.2f.\n", min, max);
        }
    }
}

//check gia va gioi han cua gia
int isValidPrice(double price)
{
    return isfinite(price) && price >= 0.1 && price <= MAX_PRICE;
}

//check gia va lam tron 2 chu so thap phan
int parsePrice(const char *text, double *price)
{
    double value;
    if (price == NULL || !praseDouble(text, 0.1, MAX_PRICE, &value))
    {
        return FALSE;
    }

    value = round(value * 100) / 100;
    if (!isValidPrice(value))
    {
        return FALSE;
    }

    *price = value;
    return TRUE;
}

int readPrice(const char *prompt, double *price)
{
    char text[INPUT_BUFFER_SIZE];
    if (prompt == NULL || price == NULL)
    {
        return FALSE;
    }
    for (;;)
    {
        printf("%s", prompt);
        fflush(stdout);
        if (!readLine(text, sizeof(text)))
        {
            printf("Error reading input. Please try again.\n");
            return FALSE;
        }
        if (parsePrice(text, price))
        {
            return TRUE;
        }
        else
        {
            printf("Invalid input. Please enter a price between %.2f and %.2f.\n", 0.1, MAX_PRICE);
        }
    }
}

//check ten san pham
int isValidName(const char *name)
{
    size_t i, length;
    int hasContent = FALSE;
    if (name == NULL )
    {
        return FALSE;
    }
    length = strlen(name);
    if (length == 0 || length > NAME_SIZE - 1)
    {
        return FALSE;
    }
    for (i =0; i< length; i++)
    {
        unsigned char ch = (unsigned char)name[i];
        if (ch =='|' || ch < 32 || ch == 127)
        {
            return FALSE;
        }
        if (!isspace(ch))
        {
            hasContent = TRUE;
        }
    }
    return hasContent;
}

//customer function

int readName(const char *prompt, int customer, char *name)
{
    int valid;
    int size = customer ? CUSTOMER_NAME_SIZE : NAME_SIZE;
    if (prompt == NULL || name == NULL) return FALSE;
    for (;;) {
        printf("%s", prompt);
        fflush(stdout);
        if (!readLine(name, size)) return FALSE;
        valid = customer ? isValidCustomerName(name) : isValidName(name);
        if (valid) return TRUE;
        printf("Ten khong hop le. Vui long nhap lai.\n");
    }
}

int isValidPhone(const char *phoneNumber)
{
    const char *p;
    size_t digits;
    if (phoneNumber == NULL || strlen(phoneNumber) >= PHONE_SIZE) return FALSE;
    p = phoneNumber;
    if (*p == '+') p++;
    else if (*p != '0') return FALSE;
    digits = strlen(p);
    if (digits < MIN_PHONE_DIGITS || digits > MAX_PHONE_DIGITS) return FALSE;
    while (*p != '\0') {
        if (!isdigit((unsigned char)*p)) return FALSE;
        p++;
    }
    return TRUE;
}

int readPhone(char *phone)
{
    if (phone == NULL) return FALSE;
    for (;;) {
        printf("So dien thoai: ");
        fflush(stdout);
        if (!readLine(phone, PHONE_SIZE)) return FALSE;
        if (isValidPhone(phone)) return TRUE;
        printf("Bat dau bang 0 hoac +; 9-14 chu so; toi da %d ky tu ca dau +.\n",
               PHONE_SIZE - 1);
    }
}

//doi ngay gio sang timestamp
int toTimestamp(DateTime value, time_t *result)
{
    struct tm calendar = {0};
    time_t when;
    if (result == NULL || value.year < 2000 || value.year > 2099 ||
        value.month < 1 || value.month > 12 || value.day < 1 || value.day > 31 ||
        value.hour < 0 || value.hour > 23 || value.minute < 0 || value.minute > 59) {
        return FALSE;
    }
    calendar.tm_year = value.year - 1900;
    calendar.tm_mon = value.month - 1;
    calendar.tm_mday = value.day;
    calendar.tm_hour = value.hour;
    calendar.tm_min = value.minute;
    calendar.tm_isdst = -1;
    when = mktime(&calendar);
    if (when == (time_t)-1) return FALSE;
    if (calendar.tm_year != value.year - 1900 || calendar.tm_mon != value.month - 1 ||
        calendar.tm_mday != value.day || calendar.tm_hour != value.hour ||
        calendar.tm_min != value.minute) return FALSE;
    *result = when;
    return TRUE;
}

int getCurrentDateTime(DateTime *result)
{
    time_t now;
    struct tm *calendar;
    if (result == NULL) return FALSE;
    now = time(NULL);
    if (now == (time_t)-1) return FALSE;
    calendar = localtime(&now);
    if (calendar == NULL) return FALSE;
    result->year = calendar->tm_year + 1900;
    result->month = calendar->tm_mon + 1;
    result->day = calendar->tm_mday;
    result->hour = calendar->tm_hour;
    result->minute = calendar->tm_min;
    return TRUE;
}

//so sanh ngay gio
int compareDateTime(DateTime a, DateTime b)
{
    if (a.year != b.year) return a.year < b.year ? -1 : 1;
    if (a.month != b.month) return a.month < b.month ? -1 : 1;
    if (a.day != b.day) return a.day < b.day ? -1 : 1;
    if (a.hour != b.hour) return a.hour < b.hour ? -1 : 1;
    if (a.minute != b.minute) return a.minute < b.minute ? -1 : 1;
    return 0;
}

int isValidBookingTime(DateTime value)
{
    time_t when, now;
    if (!toTimestamp(value, &when)) return FALSE;
    now = time(NULL);
    return now != (time_t)-1 && difftime(when, now) > 0;
}

int readBookingTime(DateTime *value)
{
    DateTime candidate;
    if (value == NULL) return FALSE;
    for (;;) {
        if (!readInt("Nam (2000-2099): ", 2000, 2099, &candidate.year) ||
            !readInt("Thang: ", 1, 12, &candidate.month) ||
            !readInt("Ngay: ", 1, 31, &candidate.day) ||
            !readInt("Gio: ", 0, 23, &candidate.hour) ||
            !readInt("Phut: ", 0, 59, &candidate.minute)) return FALSE;
        if (isValidBookingTime(candidate)) {
            *value = candidate;
            return TRUE;
        }
        printf("Thoi gian phai ton tai va sau hien tai. Nhap lai.\n");
    }
}

//check trung id
int isDuplicateID(const State *s, int id)
{
    int i;
    if (s == NULL || id <= 0 || s->productCount < 0 ||
        s->productCount > MAX_PRODUCT) return FALSE;
    for (i = 0; i < s->productCount; i++) {
        if (s->products[i].productID == id) return TRUE;
    }
    return FALSE;
}

int readOrderType(enum OrderType *type)
{
    int value;
    if (type == NULL) return FALSE;
    if (!readInt("Loai don (1 Tai cho, 2 Mang ve, 3 Giao hang): ",
                 DINE_IN, DELIVERY, &value)) return FALSE;
    *type = (enum OrderType)value;
    return TRUE;
}

int readBillInput(int *orderID, double *discount, double *tax)
{
    int id;
    double discountRate, taxRate;
    if (orderID == NULL || discount == NULL || tax == NULL) return FALSE;
    if (!readInt("Ma don hang: ", 1, INT_MAX, &id) ||
        !readDouble("Giam gia (%): ", 0, 100, &discountRate) ||
        !readDouble("Thue theo du an (%): ", 0, 100, &taxRate)) return FALSE;
    *orderID = id;
    *discount = discountRate;
    *tax = taxRate;
    return TRUE;
}
