#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <limits.h>
#include "FileHelper.h"
#include "RestaurantTypes.h"
#include "Input.h"
#include "common.h"

#define dataFile "products.txt"

int loadProducts(State *changing, const char *path)
{
    FILE *file;
    MenuItem products[MAX_PRODUCT] = {0};

    char line[512];
    char versionText[16], countText[16], lastIDText[16];
    char idText[32], name[NAME_SIZE], typeText[16], priceText[64], quantityText[32], statusText[32];

    int version, count, lastID;
    int id, type, status, quantity;
    int i,j, used;
    double price;

    if (changing ==NULL || path == NULL)
    {
        return FALSE;
    }

    file = fopen(path, "r");
    if (file == NULL)
    {
        printf("Failed to open file: %s\n", path);
        return FALSE;
    }

    if (fgets(line, sizeof(line), file) == NULL)
    {
        fclose(file);
        return FALSE;
    }
    
    used = 0;
    
    if (sscanf(line, "PRODUCT|%15[^|]|%15[^\r\n]%n",
        versionText, countText, &used) != 3)
        {
            fclose(file);
            return FALSE;
        }
    
    if (used == 0 || strspn(line + used, "\r\n") != strlen(line + used))
    {
        fclose(file);
        return FALSE;
    }

    if (!parseInt(versionText, 2, 2, &version) || !parseInt(countText, 0, MAX_PRODUCT, &count) || !parseInt(lastIDText, 0, INT_MAX, &lastID))
    {
        fclose(file);
        return FALSE;
    }
    
    for(i = 0; i< count; i++)
    {
        if (fgets(line, sizeof(line), file) == NULL)
        {
            fclose(file);
            return FALSE;
        }

        used = 0;
        if (sscanf(line, "%31[^|]|%99[^|]|%63[^|]|%31[^|]|%31[^|]|%31[^\r\n]%n",
            idText, name, typeText, priceText, quantityText, statusText, &used) != 6)
        {
            fclose(file);
            return FALSE;
        }

        if (used == 0 || strspn(line + used, "\r\n") != strlen(line + used))
        {
            fclose(file);
            return FALSE;
        }
        //check data 
        if (!parseInt(idText, 1, INT_MAX, &id) ||
            !isValidName(name) ||
            !parseInt(typeText, STARTER, DRINK, &type) ||
            !parsePrice(priceText, &price) ||
            !parseInt(quantityText, 0, MAX_QUANTITY, &quantity) ||
            !parseInt(statusText, PRODUCT_AVAILABLE, PRODUCT_UNAVAILABLE, &status))
        {
            fclose(file);
            return FALSE;
        }

        if(id > lastID)
        {
            fclose(file);
            return FALSE;
        }

        if(fabs(price) - round(price * 100) / 100 > 0.0001)
        {
            fclose(file);
            return FALSE;
        }

        for(j=0; i<i; j++)
        {
            if(products[j].productID == id) //checkID
            {
                fclose(file);
                return FALSE;
            }
        }

        products[i].productID = id;
        strcpy(products[i].productName, name);
        products[i].productPrice = price;
        products[i].productType = (enum ProductType)type;
        products[i].quantity = quantity;
        products[i].productStatus = (enum ProductStatus)status;

    }

    if (fgetc(file) != EOF || ferror(file))
    {
        fclose(file);
        return FALSE;
    }

    if(fclose(file) != 0)
    {
        return FALSE;
    }

    //update product if read successfully
    memcpy(changing->products, products, sizeof(products));
    changing->productCount = count;
    changing->lastProductID = lastID;
    return TRUE;
}

int saveProducts(const State *s, const char *path)
{
    FILE *file;
    int i,j;
    int success = TRUE;

    if (s==NULL || path == NULL || s->productCount < 0 || s->productCount > MAX_PRODUCT || s->lastProductID < 0)
    {
        return FALSE;
    }

    for (i=0; i< s->productCount; i++)
    {
        const MenuItem *item = &s->products[i];

        if (memchr(item, 0, sizeof(MenuItem)) == NULL)
        {
            return FALSE;
        }

        if (item->productID <=0||
            item->productID > s->lastProductID||
            !isValidName(item->productName)||
            !isValidPrice(item->productPrice)||
            item->productType < STARTER || item->productType > DRINK||
            (item->productStatus != PRODUCT_AVAILABLE && item->productStatus != PRODUCT_UNAVAILABLE)
            || item->quantity < 0 ||
            item->quantity > MAX_QUANTITY )
        {
            return FALSE;
        }

        if(fabs(item->productPrice - round(item->productPrice*100.0)/100.0) > 0.000001)
        {
            return FALSE;
        }
        
        for (j=0; i<i; j++)
        {
            if(s->products[j].productID == item->productID)
            {
                return FALSE;
            }
        }
    }

    file = fopen(path, "w");

    if (file == NULL)
    {
        return FALSE;
    }

    if(fprintf(file, "PRODUCT|2|%d|%d\n", s->productCount, s->lastProductID) < 0)
    {
        success = FALSE;
    }

    //in san pham
    for (i=0; i< s->productCount && success; i++)
    {
        const MenuItem *item = &s -> products[i];
        if(fprinf(file, "p|%d|%s|%.2f|%d|%d|%d\n", 
                    item->productID,
                    item->productName,
                    item->productPrice,
                    (int)item -> productType,
                    (int)item -> productStatus,
                    item->quantity) < 0 )
        {
                success = FALSE;
        }
    }
    if (fclose(file) != 0)
    {
        success = FALSE;
    }
    return success;
}
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
        return FALSE;
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

        isDuplicate = FALSE;
        for (i = 0; i < *count; i++) {
            if (strcmp(list[i].productID, item.productID) == 0) {
                isDuplicate = TRUE;
                break;
            }
        }
        if (isDuplicate) continue;

        list[*count] = item;
        (*count)++;
    }

    fclose(f);
    printf("Loaded %d products\n", *count);
    return TRUE;
}

int saveProduct(const Product *list, int count)
{
    FILE *f = fopen(PRODUCT_FILE, "w");
    int i;

    if (f == NULL) {
        printf("Error: cannot open file to save\n");
        return FALSE;
    }

    for (i = 0; i < count; i++) {
        fprintf(f, "%s,%s,%d,%.2f,%d,%d\n",
                list[i].productID, list[i].productName, list[i].productType,
                list[i].price, list[i].status, list[i].quantity);
    }

    fclose(f);
    return TRUE;
}

int saveTransaction(void) { return TRUE; }  
int saveBillReport(void)  { return TRUE; }   