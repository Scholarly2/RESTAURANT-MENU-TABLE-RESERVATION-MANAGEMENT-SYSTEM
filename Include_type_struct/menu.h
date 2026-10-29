#ifndef RESTAURANT_MENU_H
#define RESTAURANT_MENU_H

#include <stdio.h>
#include "Input.h"

//thieu ham nao bao voi tao de bo sung
//dung cac ham trong nay de code, dung ten tranh conflict
int runAction(int module, int choice);
int runProductAction(int choice);
int runTableAction(int choice);
int runReservationAction(int choice);
int runOrderAction(int choice);

static inline void showModuleMenu(int module)
{
    int choice;
    int maxChoice;

    if (module < 1 || module > 4) {
        return;
    }

    for (;;) {
        switch (module) {
        case 1:
            printf("\n=== PRODUCT MANAGEMENT ===");
            printf("1. Add product");
            printf("2. Update product menu");
            printf("3. Delete product by name");
            printf("4. Search products and sort products menu");
            printf("5. Show menu items");
            printf("0. Go back");
            maxChoice = 5;
            break;
        case 2:
            puts("\n=== TABLE MANAGEMENT ===");
            puts("1. Display table list");
            puts("2. Change table status");
            puts("3. Find an available table by number of guests");
            puts("4. Sort tables by status");
            maxChoice = 4;
            break;
        case 3:
            puts("\n=== RESERVATION MANAGEMENT ===");
            puts("1. Create reservation");
            puts("2. Cancel reservation");
            puts("3. Display reservations sorted by time");
            puts("4. Confirm guest check-in");
            maxChoice = 4;
            break;
        default: // module == 4
            puts("\n=== ORDERS AND BILLS ===");
            puts("1. Create or select an order");
            puts("2. Add an item to the order");
            puts("3. Update item quantity in the order");
            puts("4. Remove an item from the order");
            puts("5. View pre-ordered items by table");
            puts("6. View draft bill");
            puts("7. Process payment and print bill");
            puts("8. Cancel order");
            puts("9. View order list/history");
            puts("10. Configure surcharges");
            puts("11. View paid bills");
            maxChoice = 11;
            break;
        }

        puts("0. Back");
        if (!readInt("Choice: ", 0, maxChoice, &choice)) {
            return;
        }
        if (choice == 0) {
            return;
        }
        if (!runAction(module, choice)) {
            return;
        }
    }
}

static inline void showMainMenu(void)
{
    int choice;

    for (;;) {
        puts("\n=== RESTAURANT MANAGEMENT ===");
        puts("1. Product management");
        puts("2. Table management");
        puts("3. Reservation management");
        puts("4. Orders and bills");
        puts("0. Exit");

        if (!readInt("Choice: ", 0, 4, &choice)) {
            return;
        }
        if (choice == 0) {
            puts("Goodbye!");
            return;
        }
        showModuleMenu(choice);
    }
}

#endif