#include <stdio.h>
#include <string.h>
#include "common.h"
#include "FileHelper.h"

int loadProduct(Product *list, int *count)
{
    Product item;
    char    line[LINE_SIZE];
    FILE   *f;
    int     n, i, isDuplicate;

    *count = 0;

    f = fopen(PRODUCT_FILE, "r");
    if (f == NULL) {
        printf("File not found, start with an empty menu\n");
        return 0;
    }

    while (*count < MAX_PRODUCT && fgets(line, sizeof(line), f) != NULL) {
        n = sscanf(line, "%9[^,],%49[^,],%d,%f,%d,%d",
                   item.productID, item.productName, &item.productType,
                   &item.price, &item.status, &item.quantity);

        if (n != 6) continue;
        if (item.productType < 1 || item.productType > 5) continue;
        if (item.price < 0) continue;
        if (item.status != 1 && item.status != 2) continue;
        if (item.quantity < 0) continue;

        isDuplicate = 0;
        for (i = 0; i < *count; i++) {
            if (strcmp(list[i].productID, item.productID) == 0) {
                isDuplicate = 1;
                break;
            }
        }
        if (isDuplicate) continue;

        list[*count] = item;
        (*count)++;
    }

    fclose(f);
    printf("Loaded %d products\n", *count);
    return 1;
}

int saveProduct(const Product *list, int count)
{
    FILE *f = fopen(PRODUCT_FILE, "w");
    int i;

    if (f == NULL) {
        printf("Error: cannot open file to save\n");
        return 0;
    }

    for (i = 0; i < count; i++) {
        fprintf(f, "%s,%s,%d,%.2f,%d,%d\n",
                list[i].productID, list[i].productName, list[i].productType,
                list[i].price, list[i].status, list[i].quantity);
    }

    fclose(f);
    return 1;
}

int saveTransaction(void) { return 1; }  
int saveBillReport(void)  { return 1; }   

