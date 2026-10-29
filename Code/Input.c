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
