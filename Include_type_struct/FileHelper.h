#ifndef FILE_HELPER_H
#define FILE_HELPER_H

#include "RestaurantTypes.h"

int saveAllData(const State *s);
#define PRODUCT_FILE "products.txt"
#define TRANSACTION_FILE "transaction.txt"

//load and save data product
int loadProducts(State *changing, const char *path);
int saveProducts(const State *s, const char *path);

//load voi save data bill
int loadBillReport(State *chaning, const char *path);
int saveBillReport(State *chaning, const char *path);
// load voi save transaction
int loadTransaction(State *changing,const char *path);
int saveTransaction(State *changing,const char *path);
#endif
