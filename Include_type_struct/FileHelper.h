#ifndef FILE_HELPER_H
#define FILE_HELPER_H

#include "../Include_type_struct/RestaurantTypes.h"
#define TABLE_STATUS_FILE "Data/OrderHistoryTables.txt"
#define ORDER_HISTORY_FILE "Data/OrderHistory.txt"

int saveAllData(const State *s);
#define PRODUCT_FILE "Data/products.txt"


//load and save data product
int loadProducts(State *changing, const char *path);
int saveProducts(const State *s, const char *path);

//load voi save data bill
int loadBillReport(State *changing, const char *path);
int saveBillReport(const State *changing, const char *path);

//load va save data transaction
int saveTransaction(const State *s, const char *path);
int loadTransaction(State *changing, const char *path);

//load va save table
int saveAllData(const State *s);
int saveTableStatus(const State *s, const char *path);
int loadTableStatus(State *s, const char *path);
#endif
