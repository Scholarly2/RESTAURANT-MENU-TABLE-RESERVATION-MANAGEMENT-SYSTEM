#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <errno.h>

#include "RestaurantTypes.h"
#include "Input.h"
//phu

int generateProductID(const State *s)
{
    int id;

    if (s == NULL || 
    s->productCount >= MAX_PRODUCT ||
    s->productCount < 0 ||
    s->lastProductID < 0 || 
        s->lastProductID == INT_MAX)
    {
        return -1;
    }
   id = s->lastProductID;
    if (isDuplicateID(s, id + 1))
    {
        return -1;
    }
    return id;
    
}

State *copyState(const State *source)
{
    State *copy;
    if (source == NULL)
    {
        return NULL;
    }
    copy = malloc(sizeof(State));
    if(copy == NULL )
    {
        return NULL;
    }
    *copy = *source;
    return copy;
}

//add product to the menu

int addProduct(const char *name, int type, double price, int quantity   )
{
    char name[NAME_SIZE];
    int type, quantity, id;
    double price;
    State *c;
    MenuItem item = {0};

    if (S.productCount >= MAX_PRODUCT) {
        printf("Cannot add more products. Maximum limit reached.\n");
        return FALSE;
    }

    if(!readName("Name of product: ", FALSE, name)) {
        printf("Invalid product name.\n");
        return FALSE;
    }

    printf("Type of product:\n");
    printf("1. Starter\n");
    printf("2. Main Dish\n");
    printf("3. Side Dish\n");   
    printf("4. Dessert\n");
    printf("5. Drink\n");

    if(!readInt("Type of product (1-5): ", STARTER, DRINK, &type)) {
        printf("Invalid product type.\n");
        return FALSE;
    }

    if(!readPrice("Price of product: ", &price)) {
        printf("Invalid product price.\n");
        return FALSE;
    }

    if(!readInt("Quantity of product: ", 0, INT_MAX, &quantity)) {
        printf("Invalid product quantity.\n");
        return FALSE;
    }
    
    if(!isValidName(name) || type < STARTER || type > DRINK || price <= 0 || quantity < 0) {
        printf("Invalid product details.\n");
        return FALSE;
    }

    price = roundMoney(price);

    if (!isValidPrice(price)) {
        printf("Price must be between %.2f and %.2f.\n", 0.1, MAX_PRICE);
        return FALSE;
    }

    id = generateProductID(&S);

    if(id == -1)
    {
        printf("Flailed to create product ID. Please try again.\n");
        return FALSE;
    }

    c = copyState(&S);
    if (c == NULL)
    {
        printf("Failed to save! Full memory. Please try again.\n");
        return FALSE;
    }

    item.productID = id;
    strncpy(item.productName, name, NAME_SIZE - 1);
    item.productName[NAME_SIZE - 1] = '\0';
    item.productType = (enum ProductType)type;
    item.productPrice = price;
    item.quantity = quantity;
    item.productStatus = PRODUCT_AVAILABLE;

    c->products[c->productCount] = item;
    c->productCount++;          
    c ->lastProductID = id;

    if (!commitChange(c, "Add Product", id, "-", name)) {
        printf("Failed to save! Full memory. Please try again.\n");
        free(c);
        return FALSE;
    }

    printf("Product added successfully with ID: %d\n", id);
    return TRUE;
}



//update product

//update product price
//update product name
//update product type