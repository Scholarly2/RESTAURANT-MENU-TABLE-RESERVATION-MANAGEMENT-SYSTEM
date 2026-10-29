#include "Input.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <time.h>
#include <errno.h>
#include <limits.h>
#define TRUE 1
#define FALSE 0
#define MAX_PRODUCT 100
#define MAX_TABLES 100
#define MAX_RESERVATIONS 1000
#define MAX_ORDERS 1000
#define MAX_BILLS 1000
#define MAX_ITEMS 100
#define NAME_SIZE 100     
#define CUSTOMER_NAME_SIZE 50 
#define PHONE_SIZE 15       
#define TEXT_SIZE 256
#define LINE_SIZE 2048
#define ITEM_PER_PAGE 10
#define MAX_QUANTITY 1000
#define MAX_PRICE 1000000.0
#define MAX_MONEY 1000000000000.0
#define MAX_DISTANCE 1000.0
#define MIN_PHONE_DIGITS 9
#define MAX_PHONE_DIGITS 14
#define CHECKIN_EARLY_MIN 30
#define SCHEMA_VERSION 2
#define CURRENT_FILE "CurrentState.txt"
#define LOG_FILE "AuditLog.txt"
enum ProductType {
	STARTER = 1, MAINDISH, SIDEDISH, DESSERT, DRINK
};
enum ProductStatus { PRODUCT_AVAILABLE = 0, PRODUCT_UNAVAILABLE };
enum TableStatus { TABLE_AVAILABLE = 0, TABLE_BOOKED, TABLE_SERVING };
enum OrderType { DINE_IN = 1, TAKEAWAY, DELIVERY };
enum ReservationStatus { WAITING = 0, COMPLETED, CANCELLED };
enum OrderStatus { ORDER_OPEN = 0, ORDER_PAID, ORDER_CANCELLED };
enum TableEvent { BOOK = 1, CANCEL, CHECK_IN, PAY_BILL, CANCEL_ORDER };
 
typedef struct {
	int year, month, day, hour, minute;
} DateTime;
typedef struct {
	int productID;
	char productName[NAME_SIZE];
	double productPrice;
	enum ProductType productType;
	enum ProductStatus productStatus;
	int quantity;
} MenuItem;
typedef struct {
	int tableID;
	int tableNumber;
	int tableType;        
	int tableTypeID;     	
	enum TableStatus tableStatus;
} Table;
typedef struct {
	int reservationID;
	char nameCustomer[CUSTOMER_NAME_SIZE];
	char phoneNumber[PHONE_SIZE];
	int NumberCustomer;
	DateTime timeBooking;
	int tableType;
	int tableID;  
	enum ReservationStatus reservationStatus;
} Reservation;
typedef struct {
	int productID;
	char productName[NAME_SIZE];
	enum ProductType productType;
	double productPrice;
	int quantity;
} OrderItem;
 
typedef struct {
	int orderID, tableID, reservationID;
	enum OrderType orderType;
	int isPreOrder;
	int NumberCustomer;
	double deliveryDistance;
	enum OrderStatus orderStatus;
	int itemCount;
	OrderItem items[MAX_ITEMS];
} Order;
typedef struct {
	DateTime timestamp;
	int staffID, targetID;
	char actionType[30];
	char oldValue[TEXT_SIZE], newValue[TEXT_SIZE];
} AuditLog;
typedef struct {
	double dineInRate, takeawayRate;
	double deliveryBase, deliveryPerKm;
} ChargePolicy;
typedef struct {
	int billID, orderID, tableID, staffID;
	enum OrderType orderType;
	DateTime timeBill;
	int itemCount;
	OrderItem items[MAX_ITEMS];
	double totalMoney;
	double discountRate, discount;
	double taxRate, tax;      
	double surcharge;           
	double amountDue;         	
	ChargePolicy policySnapshot; 
	double deliveryDistance;    
} Bill;
typedef struct {
	MenuItem products[MAX_PRODUCT];
	Table tables[MAX_TABLES];
	Reservation reservations[MAX_RESERVATIONS];
	Order orders[MAX_ORDERS];
	Bill bills[MAX_BILLS];
	int productCount, tableCount, resCount;
	int orderCount, billCount;
	int lastProductID, lastResID, lastOrderID, lastBillID;
	ChargePolicy policy;
} State;
State S;
int currentStaff;
int currentGeneration = 0;
int isValidPhone(const char*phoneNumber){
    const char*p=phoneNumber;
	if(phoneNumber==NULL){
		return FALSE;
	}
	if (*p=='+'){
		p=p+1;
        printf("Phone number starts with '+', skipping to: %s\n", p);
	}
	else if (*p!='0'){
        printf("Phone number does not start with '0' or '+'.\n");
        return FALSE;
	}
	int digits=strlen(p);
    printf("Phone number after processing: %s, digits count: %d\n", p, digits);
	if (digits<MIN_PHONE_DIGITS||digits>MAX_PHONE_DIGITS){
		printf("Phone number has invalid number of digits: %d\n", digits);
        return FALSE;
	}
    printf("Validating each character in phone number: %s\n", p);
	while (*p!='\0'){
		if (!isdigit((unsigned char)*p)){
		printf("Invalid character in phone number: %c\n", *p);
        return FALSE;
		}
		p=p+1;
        printf("Next character to validate: %c\n", *p);
	}
	    printf("Phone number is valid: %s\n", phoneNumber);
    return TRUE;
}
int toTimestamp(DateTime t,time_t*result){
    if (result==NULL){
        return FALSE;
    }
    if (t.year < 2000 || t.year > 2100 ||
        t.month < 1 || t.month > 12 ||
        t.day < 1 || t.day > 31 ||
        t.hour < 0 || t.hour > 23 ||
        t.minute < 0 || t.minute > 59) {
            return FALSE;
    }
    struct tm x = {0};
    x.tm_year = t.year - 1900;
    x.tm_mon = t.month - 1;
    x.tm_mday = t.day;
    x.tm_hour = t.hour;
    x.tm_min = t.minute;
    time_t when = mktime(&x);
    if (when == -1) {
        return FALSE;
    }
    if (x.tm_year != t.year - 1900 || x.tm_mon != t.month - 1 ||
        x.tm_mday != t.day || x.tm_hour != t.hour || x.tm_min != t.minute) {
        return FALSE;
    }
    *result = when;
    return TRUE;
}
#include <errno.h>
#include <limits.h>
#include "common.h"


#define INPUT_BUFFER_SIZE 1256

//check input product item - UYEN
//check dinh dang name product
int isValidProductName(const char *name)
{
    int i;

    if (!isValidName(name)) {
        return FALSE;
    }

    for (i = 0; name[i] != '\0'; i++) {
        if (name[i] == '_') {
            if (i == 0 ||
                name[i + 1] == '\0' ||
                name[i + 1] == '_') {
                return FALSE;
            }
        } else if (name[i] < 'A' || name[i] > 'Z') {
            return FALSE;
        }
    }

    return TRUE;
}
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

void getCurrentDateTime(DateTime*reasult){
    time_t now = time(NULL);
	struct tm*t=localtime(&now);
	if (reasult==NULL|| t== NULL){
		printf("Failed to get current time or result pointer is NULL.\n");
        return;
	}
	(*reasult).year=t->tm_year+1900;
	(*reasult).month=t->tm_mon +1;
	(*reasult).day=t->tm_mday;
	(*reasult).hour=t->tm_hour;
	(*reasult).minute=t->tm_min;
    printf("Current DateTime set to: %04d-%02d-%02d %02d:%02d\n", reasult->year, reasult->month, reasult->day, reasult->hour, reasult->minute);	
}
<<<<<<< HEAD
int compareDateTime(DateTime a, DateTime b) {
long long ka = (((a.year * 100LL + a.month)
                * 100 + a.day)
                * 100 + a.hour)
                * 100 + a.minute;
printf("Comparing DateTime A: %04d-%02d-%02d %02d:%02d (ka=%lld) with DateTime B: %04d-%02d-%02d %02d:%02d\n",
           a.year, a.month, a.day, a.hour, a.minute, ka,
           b.year, b.month, b.day, b.hour, b.minute);
    long long kb = (((b.year * 100LL + b.month) * 100 + b.day)
                    * 100 + b.hour) * 100 + b.minute;
                    printf("Computed kb=%lld for DateTime B\n", kb);
    if (ka < kb) {
        printf("DateTime A is earlier than DateTime B\n");
        return -1;
    }
    if (ka > kb) {
        printf("DateTime A is later than DateTime B\n");
        return TRUE;
    }
    printf("DateTime A is equal to DateTime B\n");
    return FALSE;
}
int isValidBookingTime(DateTime t){
    DateTime now;
    getCurrentDateTime(&now);
    if (compareDateTime(t, now) < 0) {
        return FALSE;
    }
    time_t bookingTimestamp, nowTimestamp;
    if (!toTimestamp(t, &bookingTimestamp) || !toTimestamp(now, &nowTimestamp)) {
        return FALSE;
    }
    double diffMinutes = difftime(bookingTimestamp, nowTimestamp) / 60.0;
    if (diffMinutes < CHECKIN_EARLY_MIN) {
        return FALSE;
    }
    return TRUE;
}
void sortBookingByTime(void){
    if (S.resCount <= 0) {
        return;
    }
    Reservation *view = malloc((size_t)S.resCount * sizeof(Reservation));
    if (view == NULL) {
        return;
    }
    for (int i = 0; i < S.resCount; i++) {
        view[i] = S.reservations[i];
    }
    for (int i = 0; i < S.resCount - 1; i++) {
        int minIndex;
        minIndex = i;
        for (int j = i + 1; j < S.resCount; j++) {
            int d = compareDateTime(view[j].timeBooking, view[minIndex].timeBooking);
            if (d < 0 || (d == 0 && view[j].reservationID < view[minIndex].reservationID)) {
                minIndex = j;
            }
        }
        if (minIndex != i) {
            Reservation temp = view[i];
            view[i] = view[minIndex];
            view[minIndex] = temp;
        }
    }
    for (int i = 0; i < S.resCount; i++) {
        S.reservations[i] = view[i];
    }
    free(view);
}
=======
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
>>>>>>> origin/BinhGold
