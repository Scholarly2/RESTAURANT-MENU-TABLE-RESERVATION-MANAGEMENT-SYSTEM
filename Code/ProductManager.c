#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <limits.h>

#include "../Include_type_struct/RestaurantTypes.h"
#include "../Include_type_struct/Input.h"
#include "../Include_type_struct/FileHelper.h"
#include "../Include_type_struct/menu.h"
#include "../Include_type_struct/Login.h"

enum ProductUPDATE
{
    UP_Name = 1,
    UP_PRICE,
    UP_TYPE,
    UP_QUANTITY,
    UP_STATUS
};

//find product
int findProductByID(const State *s, int id)
{
    int i;
    if (s == NULL || id <= 0 || s->productCount < 0 || s-> productCount > MAX_PRODUCT)
    {
        return -1; //index
    }

    for (i =0; i< s->productCount; i++)
    {
        if (s->products[i].productID == id)
        {
            return i;
        }
    }
    return -1;
}
int generateProductID (const State *s)
{
    int i;
    // bo nho day
    if (s==NULL || s->productCount < 0 || s->productCount >= MAX_PRODUCT || 
        s->lastProductID < 0 || s->lastProductID == INT_MAX) 
    {
        return -1;
    }
    for (i =0; i<s->productCount; i++)
    {
        if( s->products[i].productID > s->lastProductID)
        {
            return -1;
        }
    }
    return s->lastProductID + 1;
}

int updatePrice(MenuItem *item, double value)
{
    if (item == NULL || !isValidPrice(value) )
    {
        return FALSE;
    }
    value = round(value*100.0)/100.0;
    if(!isValidPrice(value))
    {
        return FALSE;
    }
    item->productPrice = value;
    return TRUE;
}

int updateName(MenuItem *item, const char *value)
{
    if (item == NULL || !isValidName(value))
    {
        return FALSE;
    }
    memmove(item->productName, value, strlen(value) + 1);
    return TRUE;
}

int updateType(MenuItem *item, int value)
{
    if (item == NULL || value < STARTER || value > DRINK )
    {
        return FALSE;
    }

    item -> productType = (enum ProductType)value;
    return TRUE;
}
int updateQuantity(MenuItem *item, int value)
{
    if (item == NULL || value < 0 || value > MAX_QUANTITY)
    {
        return FALSE;
    }
    item->quantity = value;
    return TRUE;
}

int updateStatus(MenuItem *item, int value)
{
    if (item == NULL || (value != PRODUCT_AVAILABLE && value != PRODUCT_UNAVAILABLE))
    {
        return FALSE;
    }
    item ->productStatus = (enum ProductStatus)value;
    return TRUE;
}

int addProduct(void)
{
    char name[NAME_SIZE];
    int type, quantity, id;
    double price;

    MenuItem item = {0};

    MenuItem oldItem;
    int oldCount;
    int oldLastID;

    if (S.productCount < 0) {
        printf("Invalid product data.\n");
        return FALSE;
    }

    if (S.productCount >= MAX_PRODUCT) {
        printf("Cannot add more products. Maximum limit reached.\n");
        return FALSE;
    }

    printf("Name format: COM_RANG_DUA_BO, SUP_NAM_TUYET.\n");

    for (;;) {
        if (!readName("Name of product: ", FALSE, name)) {
            return FALSE;
        }

        if (isValidProductName(name)) {
            break;
        }

        printf("Use uppercase letters A-Z with single underscores "
               "between words.\n");
    }

    printf("Type of product:\n");
    printf("1. Starter\n");
    printf("2. Main Dish\n");
    printf("3. Side Dish\n");
    printf("4. Dessert\n");
    printf("5. Drink\n");

    if (!readInt("Type of product (1-5): ", STARTER, DRINK, &type)) {
        return FALSE;
    }

    if (!readPrice("Price of product: ", &price)) {
        return FALSE;
    }

    if (!readInt("Quantity of product: ", 0, MAX_QUANTITY, &quantity)) {
        return FALSE;
    }

    if (!isValidProductName(name) ||
        type < STARTER ||
        type > DRINK ||
        !isValidPrice(price) ||
        quantity < 0 ||
        quantity > MAX_QUANTITY) {
        printf("Invalid product details.\n");
        return FALSE;
    }

    price = round(price * 100.0) / 100.0;

    if (!isValidPrice(price)) {
        printf("Price must be between %.2f and %.2f.\n",
               0.01, MAX_PRICE);
        return FALSE;
    }

    id = generateProductID(&S);

    if (id == -1) {
        printf("Failed to create product ID. Please try again.\n");
        return FALSE;
    }

    item.productID = id;
    strncpy(item.productName, name, NAME_SIZE - 1);
    item.productName[NAME_SIZE - 1] = '\0';
    item.productType = (enum ProductType)type;
    item.productPrice = price;
    item.quantity = quantity;
    item.productStatus = PRODUCT_AVAILABLE;

    oldCount = S.productCount;
    oldLastID = S.lastProductID;
    oldItem = S.products[oldCount];

    S.products[oldCount] = item;
    S.productCount++;
    S.lastProductID = id;

    if (!saveProducts(&S, PRODUCT_FILE)) {
        S.products[oldCount] = oldItem;
        S.productCount = oldCount;
        S.lastProductID = oldLastID;

        printf("Failed to write product data to file.\n");
        return FALSE;
    }

    printf("Product added successfully with ID: %d\n", id);
    return TRUE;
}

int deleteProduct(void)
{
    int id;
    int index;
    int confirm;
    enum ProductStatus oldStatus;

    if (S.productCount == 0) {
        printf("The product list is empty.\n");
        return FALSE;
    }

    if (S.productCount < 0 || S.productCount > MAX_PRODUCT) {
        printf("Invalid product data.\n");
        return FALSE;
    }

    do {
        if (!readInt("Product ID to delete (0 Back): ",
                     0, INT_MAX, &id)) {
            return FALSE;
        }

        if (id == 0) {
            return TRUE;
        }

        index = findProductByID(&S, id);

        if (index == -1) {
            printf("Product not found. Please enter another ID.\n");
        }

    } while (index == -1);

    printf("\n=== SELECTED PRODUCT ===\n");
    printListProduct(&S.products[index], 1);

    if (S.products[index].productStatus == PRODUCT_UNAVAILABLE) {
        printf("This product is already unavailable.\n");
        return TRUE;
    }

    if (!readInt("Are you sure? (1 Yes, 0 Back): ",
                 0, 1, &confirm)) {
        return FALSE;
    }

    if (confirm == 0) {
        return TRUE;
    }

    oldStatus = S.products[index].productStatus;
    S.products[index].productStatus = PRODUCT_UNAVAILABLE;

    if (!saveProducts(&S, PRODUCT_FILE)) {
        S.products[index].productStatus = oldStatus;
        printf("Failed to save product data.\n");
        return FALSE;
    }
    printf("Product marked as unavailable successfully.\n");
    return TRUE;
}


int updateProduct(void)
{
    int id;
    int index;
    int choice;
    int up;
    int value;

    double number = 0.0;
    char name[NAME_SIZE];
    const char *text = NULL;

    MenuItem oldItem;
    MenuItem item;

    if (S.productCount == 0) {
        printf("The product list is empty.\n");
        return TRUE;
    }

    if (S.productCount < 0 || S.productCount > MAX_PRODUCT) {
        printf("Invalid product data.\n");
        return FALSE;
    }

    do 
    {

        if (!readInt("Product ID (0 Back): ", 0, INT_MAX, &id)) 
        {
             return FALSE;
        }
    if (id == 0) 
        {
            return TRUE;
        }
    index = findProductByID(&S, id);
    if (index == -1) 
    {
        printf("Product not found. Please enter another ID.\n");
    }
    } while (index == -1);

    printf("\n=== SELECTED PRODUCT ===\n");
    printListProduct(&S.products[index], 1);

    printf("\n=== UPDATE PRODUCT ===\n");
    printf("1. Update price\n");
    printf("2. Update name\n");
    printf("3. Update status\n");
    printf("4. Update quantity\n");
    printf("5. Update type\n");
    printf("0. Back to product menu\n");

    if (!readInt("Choose: ", 0, 5, &choice)) {
        return FALSE;
    }

    if (choice == 0) {
        return TRUE;
    }

    
    switch (choice) {
    case 1:
        up = UP_PRICE;

        if (!readPrice("New price: ", &number)) {
            return FALSE;
        }
        break;

    case 2:
        up = UP_Name;

        if (!readName("New product name: ", FALSE, name)) {
            return FALSE;
        }

        text = name;
        break;

    case 3:
        up = UP_STATUS;

        printf("0. Available\n");
        printf("1. Unavailable\n");

        if (!readInt("New status: ",
                     PRODUCT_AVAILABLE,
                     PRODUCT_UNAVAILABLE,
                     &value)) {
            return FALSE;
        }

        number = value;
        break;

    case 4:
        up = UP_QUANTITY;

        if (!readInt("New quantity: ", 0, MAX_QUANTITY, &value)) {
            return FALSE;
        }

        number = value;
        break;

    case 5:
        up = UP_TYPE;

        printf("1. Starter\n");
        printf("2. Main dish\n");
        printf("3. Side dish\n");
        printf("4. Dessert\n");
        printf("5. Drink\n");

        if (!readInt("New type: ", STARTER, DRINK, &value)) {
            return FALSE;
        }

        number = value;
        break;

    default:
        return FALSE;
    }

    oldItem = S.products[index];
    item = oldItem;

    switch (up) {
    case UP_PRICE:
        if (!updatePrice(&item, number)) return FALSE;
        break;
    case UP_Name:
        if (!updateName(&item, text)) return FALSE;
        break;
    case UP_STATUS:
        if (!updateStatus(&item, (int)number)) return FALSE;
        break;
    case UP_QUANTITY:
        if (!updateQuantity(&item, (int)number)) return FALSE;
        break;
    case UP_TYPE:
        if (!updateType(&item, (int)number)) return FALSE;
        break;
    default:
        return FALSE;
    }

    S.products[index] = item;
    if (!saveProducts(&S, PRODUCT_FILE)) {
        S.products[index] = oldItem;
        printf("Failed to update or save the product.\n");
        return FALSE;
    }

    printf("Product updated successfully.\n");
    return TRUE;
}


void printListProduct(const MenuItem *list, int count)
{
    int i;
    const char *typename;
    const char *statusname;

    if(count < 0 || count > MAX_PRODUCT || (list == NULL && count > 0) )
    {
        printf("Invalid product list");
        return;
    }

    if (count == 0)
    {
        printf("The product list is empty");
        return;
    }
    printf("\n%-6s %-25s %12s %-12s %8s %-12s\n",
           "ID", "Name", "Price", "Type", "Quantity", "Status");
    for (i = 0; i < count; i++) {
        switch (list[i].productType) {
        case STARTER:  typename = "Starter"; break;
        case MAINDISH: typename = "Main dish"; break;
        case SIDEDISH: typename = "Side dish"; break;
        case DESSERT:  typename = "Dessert"; break;
        case DRINK:    typename = "Drink"; break;
        default:      typename = "Unknown"; break;
        }
        switch (list[i].productStatus) {
        case PRODUCT_AVAILABLE:   statusname = "Available"; break;
        case PRODUCT_UNAVAILABLE: statusname = "Unavailable"; break;
        default:                  statusname = "Unknown"; break;
        }
        printf("%-6d %-25s %12.2f %-12s %8d %-12s\n",
               list[i].productID, list[i].productName, list[i].productPrice,
               typename, list[i].quantity, statusname);
    }
}
//check uper
int compareProductNames(const char *a, const char *b)
{
    while (*a && *b) {
        int x = tolower((unsigned char)*a);
        int y = tolower((unsigned char)*b);
        if (x != y) return (x > y) ? 1 : -1;
        a++;
        b++;
    }
    return (*a != '\0') - (*b != '\0');
}
//check keyword
void printSortedProducts(int criterion)
{
    MenuItem list[MAX_PRODUCT];
    int i, j;
    if (S.productCount < 0 || S.productCount > MAX_PRODUCT) {
        puts("Invalid product list.");
        return;
    }
    memcpy(list, S.products, (size_t)S.productCount * sizeof(list[0]));
    for (i = 1; i < S.productCount; i++) {
        MenuItem key = list[i];
        j = i - 1;
        while (j >= 0) {
            int cmp;
            if (criterion == 2) {
                cmp = (list[j].productPrice > key.productPrice) -
                      (list[j].productPrice < key.productPrice);
            } else if (criterion == 3) {
                cmp = compareProductNames(list[j].productName, key.productName);
            } else {
                cmp = 0;
            }
            if (cmp == 0) cmp = (list[j].productID > key.productID) -
                               (list[j].productID < key.productID);
            if (cmp <= 0) break;
            list[j + 1] = list[j];
            j--;
        }
        list[j + 1] = key;
    }
    printListProduct(list, S.productCount);
}

void sortByID(void)    { printSortedProducts(1); }
void sortByPrice(void) { printSortedProducts(2); }
void sortByName(void)  { printSortedProducts(3); }

void searchProductByID(int id)
{
    int index = findProductByID(&S,id);
    if (index == -1) 
    {
        printf("Product not found.");
    }
    else printListProduct(&S.products[index], 1);
}
void searchProductByName(const char *keyword)
{
    MenuItem matches[MAX_PRODUCT];
    char lowerKeyword[NAME_SIZE];
    char lowerName[NAME_SIZE];

    int i;
    int j;
    int count = 0;

    if (!isValidName(keyword) ||
        S.productCount < 0 ||
        S.productCount > MAX_PRODUCT) {
        printf("Invalid search data.\n");
        return;
    }

    if (S.productCount == 0) {
        printf("The product list is empty.\n");
        return;
    }

    for (j = 0; keyword[j] != '\0'; j++) {
        lowerKeyword[j] = (char)tolower((unsigned char)keyword[j]);
    }
    lowerKeyword[j] = '\0';

    for (i = 0; i < S.productCount; i++) {

        for (j = 0;
             j < NAME_SIZE - 1 && S.products[i].productName[j] != '\0';
             j++) {
            lowerName[j] =
                (char)tolower((unsigned char)S.products[i].productName[j]);
        }
        lowerName[j] = '\0';

        if (strstr(lowerName, lowerKeyword) != NULL) {
            matches[count] = S.products[i];
            count++;
        }
    }

    if (count == 0) {
        printf("No products found for \"%s\".\n", keyword);
        return;
    }

    printf("\n=== SUGGESTED PRODUCTS ===\n");
    printf("Keyword: %s\n", keyword);
    printf("Found %d matching product(s).\n", count);

    printListProduct(matches, count);
}
void searchProductByType(int type)
{
    MenuItem matches[MAX_PRODUCT];
    int i, count = 0;
    if (type < STARTER || type > DRINK || S.productCount < 0 || S.productCount > MAX_PRODUCT) {
        puts("Invalid search data.");
        return;
    }
    for (i = 0; i < S.productCount; i++) {
        if ((int)S.products[i].productType == type) matches[count++] = S.products[i];
    }
    if (count == 0) puts("No matching products.");
    else printListProduct(matches, count);
}

int searchProductMenu(void)
{
    int choice, id;
    char keyword[NAME_SIZE];
    do {
        puts("\n=== SEARCH PRODUCT ===\n1. Search by ID\n2. Search by name\n0. Back");
        if (!readInt("Choose: ", 0, 2, &choice)) return FALSE;
        switch (choice) {
        case 1:
            if (S.productCount == 0) { puts("The product list is empty."); break; }
            do 
            {
                if (!readInt("Product ID (0 Back): ", 0, INT_MAX, &id)) return FALSE;
                if (id == 0) break;
                if (findProductByID(&S, id) != -1) break;
                printf("Product not found. Please enter another ID.\n");

            } while (TRUE);

            if (id != 0) searchProductByID(id);
            break;

        case 2:
            if (!readName("Name or keyword: ", FALSE, keyword)) return FALSE;
            searchProductByName(keyword);
            break;

        case 0: break;

        }
    } while (choice != 0);
    return TRUE;
}

int showProductListMenu(void)
{
    int choice;
    do {
        puts("\n=== SHOW PRODUCT LIST ===");
        puts("1. Sort by ID\n2. Sort by price (low to high)\n3. Sort by name (A-Z)\n0. Back");
        if (!readInt("Choose: ", 0, 3, &choice)) return FALSE;
        switch (choice) {
        case 1: sortByID(); break;
        case 2: sortByPrice(); break;
        case 3: sortByName(); break;
        case 0: break;
        }
    } while (choice != 0);
    return TRUE;
}

void ProductMenu(int role)
{
    int choice;
    
    if (S.productCount < 0 || S.productCount > MAX_PRODUCT) {
        puts("Invalid product data. Load a valid product file first.");
        return;
    }
    if (role == ROLE_MANAGER)
    {
        do {
            printf("\n=== PRODUCT MANAGEMENT ===");
            printf("1. Add product");
            printf("2. Update product menu");
            printf("3. Delete product by name");
            printf("4. Search products menu");
            printf("5. Show menu items");
            printf("0. Go back");
        if (!readInt("Choose: ", 0, 5, &choice)) return;
        switch (choice) {
        case 1: addProduct(); break;
        case 2: updateProduct(); break;
        case 3: deleteProduct(); break;
        case 4: searchProductMenu(); break;
        case 5: if (!showProductListMenu()) return; break;
        case 0: break;
        } 
        if (feof(stdin) || ferror(stdin)) return;
        } while (choice != 0);
    }
    else 
    {
            do {
            printf("\n=== PRODUCT MANAGEMENT ===");
            printf("1. Search products menu");
            printf("2. Show menu items");
            printf("0. Go back");
        if (!readInt("Choose: ", 0, 2, &choice)) return;
        switch (choice) {
        case 1: searchProductMenu(); break;
        case 2: if (!showProductListMenu()) return; break;
        case 0: break;
        } 
        if (feof(stdin) || ferror(stdin)) return;
        } while (choice != 0);
    }
    
}
