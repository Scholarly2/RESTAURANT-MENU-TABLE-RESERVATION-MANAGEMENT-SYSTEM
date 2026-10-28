#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

#include "RestaurantTypes.h"
#include "Input.h"
#include "FileHelper.h"
#include "menu.c"
#include "OrderManager.c"
#include "ProductManager.c"
#include "ReservationManager.c"
#include "TableManager.c"

/* Chỉ định nghĩa một lần trong chương trình. */
State S = {0};

void productMenu(void);
void tableMenu(void);
void reservationMenu(void);
void orderMenu(void);

int main(void)
{
    int choice;

    if (!loadData(&S)) {
        printf("Cannot load restaurant data.\n");
        return EXIT_FAILURE;
    }

    do {
        printf("\n=== RESTAURANT MANAGEMENT ===\n");
        printf("1. Product management\n");
        printf("2. Table management\n");
        printf("3. Reservation management\n");
        printf("4. Orders and bills\n");
        printf("0. Exit\n");

        if (!readInt("Choose: ", 0, 4, &choice)) {
            return ferror(stdin) ? EXIT_FAILURE : EXIT_SUCCESS;
        }

        switch (choice) {
        case 1:
            productMenu();
            break;

        case 2:
            tableMenu();
            break;

        case 3:
            reservationMenu();
            break;

        case 4:
            orderMenu();
            break;

        case 0:
            printf("Exit the program.\n");
            break;
        }

        if (feof(stdin) || ferror(stdin)) {
            return ferror(stdin) ? EXIT_FAILURE : EXIT_SUCCESS;
        }
    } while (choice != 0);

    return EXIT_SUCCESS;
}
