#ifndef FILE_HELPER_H
#define FILE_HELPER_H

#include "RestaurantTypes.h"

int saveAllData(const State *s);
#define PRODUCT_FILE "products.txt"


//load and save data product
int loadProducts(State *changing, const char *path);
int saveProducts(const State *s, const char *path);

#endif
