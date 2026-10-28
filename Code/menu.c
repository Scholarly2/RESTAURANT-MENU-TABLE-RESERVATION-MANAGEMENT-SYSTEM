#include <stdio.h>
#include <string.h>
#include "common.h"
#include "Input.h"
#include "FileHelper.h"
#include "TableManager.h"
#include "ProductManager.h"
#include "ReservationManager.h"
#include "OrderManager.h"
#include "BillManager.h"
#include "menu.h"
#define MENU_WIDTH    52  

static const char *const MAIN_ITEMS[] = {
    "Menu Item Management",
    "Table Management",
    "Table Reservation",
    "Ordering & Billing"
};

static const char *const PRODUCT_ITEMS[] = {
    "Add new item",
    "Update item",
    "Delete item",
    "Search by name",
    "Search by type",
    "Sort by price",
    "Sort by name",
    "Print list"
};

static const char *const TABLE_ITEMS[] = {
    "Show tables",
    "Update table status",
    "Find available table",
    "Sort tables by status"
};

static const char *const RESERVATION_ITEMS[] = {
    "Create reservation",
    "Cancel reservation",
    "Sort booking by time"
};

static const char *const ORDER_ITEMS[] = {
    "Create order",
    "Add food",
    "Update food",
    "Delete food",
    "List pre-ordered dishes",
    "Print bill"
};

/* ===================== DRAWING HELPERS ===================== */

void printBorder(char fill)
{
    int i;

    putchar('+');
    for (i = 0; i < MENU_WIDTH - 2; i++) putchar(fill);
    printf("+\n");
}

void printCentered(const char *text)
{
    int inner = MENU_WIDTH - 2;
    int len   = (int)strlen(text);
    int left, right;

    if (len > inner) len = inner;
    left  = (inner - len) / 2;
    right = inner - len - left;

    printf("|%*s%.*s%*s|\n", left, "", len, text, right, "");
}

void printMenuItem(int number, const char *label)
{
    int labelWidth = MENU_WIDTH - 2 - 6;     

    printf("|  %d.  %-*.*s|\n", number, labelWidth, labelWidth, label);
}

void printMenu(const char *title, const char *const items[], int itemCount,
               const char *zeroLabel)
{
    int i;

    printf("\n");
    printBorder('=');
    printCentered(title);
    printBorder('=');
    for (i = 0; i < itemCount; i++) {
        printMenuItem(i + 1, items[i]);
    }
    printBorder('-');
    printMenuItem(0, zeroLabel);
    printBorder('=');
}

/* ===================== NAVIGATION ===================== */

void showMainMenu(Table *tables, int tableCount, Product *products, int *productCount)
{
    int choice = -1;
    int r;

    do {
        printMenu("RESTAURANT MANAGEMENT SYSTEM",
                  MAIN_ITEMS, ARRAY_SIZE(MAIN_ITEMS), "Save & Exit");

        r = readInt(" >> Enter your choice: ", &choice);
        if (r == -1) {                       
            choice = 0;
            printf("\n");
        } else if (r == 0 || choice < 0 || choice > 4) {
            printf("Error, please enter a number from 0 to 4\n");
            choice = -1;
            continue;
        }

        if (choice != 0) {
            showModuleMenu(choice, tables, tableCount, products, productCount);
        }
    } while (choice != 0);

    saveProduct(products, *productCount);
    saveTransaction();
    saveBillReport();
    printf("\n");
    printBorder('=');
    printCentered("Thank you & goodbye!");
    printBorder('=');
}

void showModuleMenu(int module, Table *tables, int tableCount,
                    Product *products, int *productCount)
{
    int choice = -1;
    int r;

    do {
        switch (module) {
            case 1:  
                printMenu("MENU ITEM MANAGEMENT",
                          PRODUCT_ITEMS, ARRAY_SIZE(PRODUCT_ITEMS), "Back");
                break;
            case 2:  
                printMenu("TABLE MANAGEMENT",
                          TABLE_ITEMS, ARRAY_SIZE(TABLE_ITEMS), "Back");
                break;
            case 3:  
                printMenu("TABLE RESERVATION",
                          RESERVATION_ITEMS, ARRAY_SIZE(RESERVATION_ITEMS), "Back");
                break;
            case 4:  
                printMenu("ORDERING & BILLING",
                          ORDER_ITEMS, ARRAY_SIZE(ORDER_ITEMS), "Back");
                break;
            default:
                break;
        }

        r = readInt(" >> Enter your choice: ", &choice);
        if (r == -1) {                       
            choice = 0;
            continue;
        }
        if (r == 0) {
            printf("Error: please enter a valid number!\n");
            choice = -1;
            continue;
        }

        switch (module) {
            case 1:
                switch (choice) {
                    case 1: addProduct(products, productCount);            break;
                    case 2: updateProduct(products, *productCount);        break;
                    case 3: deleteProduct(products, productCount);         break;
                    case 4: searchProductByName(products, *productCount);  break;
                    case 5: searchProductByType(products, *productCount);  break;
                    case 6: sortByPrice(products, *productCount);          break;
                    case 7: sortByName(products, *productCount);           break;
                    case 8: printListProduct(products, *productCount);     break;
                    case 0: printf("Back to main menu...\n");              break;
                    default: printf("Invalid choice!\n");
                }
                break;

            case 2:
                switch (choice) {
                    case 1:
                        sortTableByStatus(tables, tableCount);
                        printTables(tables, tableCount);
                        break;
                    case 2:
                        updateStatusTable(tables, tableCount);
                        break;
                    case 3: {
                        int numberCustomer = 0, tableID, idx;

                        r = readInt("Enter number of customers: ", &numberCustomer);
                        if (r != 1) {
                            printf("Invalid number!\n");
                            break;
                        }
                        tableID = findAvailableTable(tables, tableCount, numberCustomer);
                        if (tableID == -1) {
                            printf("No suitable table available\n");
                            break;
                        }
                        idx = findTableIndexByID(tables, tableCount, tableID);
                        printf("Suggested table: number %d (seats up to %d)\n",
                               tables[idx].tableNumber, tables[idx].tableType);
                        break;
                    }
                    case 4:
                        sortTableByStatus(tables, tableCount);
                        printf("Tables sorted by status.\n");
                        break;
                    case 0: printf("Back to main menu...\n"); break;
                    default: printf("Invalid choice!\n");
                }
                break;

            case 3:
                switch (choice) {
                    case 1: createReservation(tables, tableCount); break;
                    case 2: cancelReservation(tables, tableCount); break;
                    case 3: sortBookingByTime();                   break;
                    case 0: printf("Back to main menu...\n");      break;
                    default: printf("Invalid choice!\n");
                }
                break;

            case 4:
                switch (choice) {
                    case 1: createOrderTable(tables, tableCount);  break;
                    case 2: addOrderFood();                        break;
                    case 3: updateOrderFood();                     break;
                    case 4: deleteOrderFood();                     break;
                    case 5: listPreOrderedDishes();                break;
                    case 6: printBillReport();                     break;
                    case 0: printf("Back to main menu...\n");      break;
                    default: printf("Invalid choice!\n");
                }
                break;

            default:
                break;
        }
    } while (choice != 0);
}
