#ifndef INPUT_H
#define INPUT_H

#include <time.h>
#include "RestaurantTypes.h"

//max phone number
#define MIN_PHONE_DIGITS 9
#define MAX_PHONE_DIGITS 14


// Doc dong, chuyen chuoi thanh so, kiem tra khoang gia tri.
int readLine(char *text, int size);
int parseInt(const char *text, int min, int max, int *value);
int parseDouble(const char *text, double min, double max, double *value);
int readInt(const char *prompt, int min, int max, int *value);
int readDouble(const char *prompt, double min, double max, double *value);

// check input price and get.
int isValidPrice(double price);
int parsePrice(const char *text, double *price);
int readPrice(const char *prompt, double *price);

// check name; customer = TRUE 
int isValidName(const char *name);
int isValidCustomerName(const char *name);
int readName(const char *prompt, int customer, char *name);

// check phone number
int isValidPhone(const char *phoneNumber);
int readPhone(char *phone);

// check real time, check time future (booking)
int toTimestamp(DateTime value, time_t *result);
int getCurrentDateTime(DateTime *result);
int compareDateTime(DateTime a, DateTime b);
int isValidBookingTime(DateTime value);
int readBookingTime(DateTime *value);

// check duplicata - available (tồn tại)
int isDuplicateID(const State *s, int id);

// check bill, check tham so
int readOrderType(enum OrderType *type);
int readBillInput(int *orderID, double *discount, double *tax);

#endif

