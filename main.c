#include <stdio.h>
#include <stdlib.h>
#include "Include_type_struct/RestaurantTypes.h"
#include "Include_type_struct/Input.h"
#include "Include_type_struct/FileHelper.h"
#include "Include_type_struct/menu.h"
#include "Include_type_struct/Login.h"



State S = {0};

static void printMainMenu(int role)
{
    printf("\n=== RESTAURANT MANAGEMENT ===\n");
    printf("Role: %s\n", role == ROLE_MANAGER ? "Manager" : "Staff");
    printf("1. Product management\n");
    printf("2. Table management\n");
    printf("3. Reservation management\n");
    printf("4. Orders and bills\n");
    printf("5. Change my password\n");
    if (role == ROLE_MANAGER) {
        printf("6. Account management\n");
    }
    printf("0. Exit\n");
}

static void manageAccounts(Account accounts[], int *accountCount)
{
    int choice;

    do {
        printf("\n=== ACCOUNT MANAGEMENT ===\n");
        printf("1. List accounts\n");
        printf("2. Create staff account\n");
        printf("3. Change account role\n");
        printf("0. Back\n");

        if (!readInt("Choose: ", 0, 3, &choice)) {
            return;
        }

        switch (choice) {
        case 1:
            listAccounts(accounts, *accountCount);
            break;
        case 2:
            createStaffAccount(accounts, accountCount);
            saveAccounts(accounts, *accountCount);
            break;
        case 3:
            updateUserRole(accounts, *accountCount);
            saveAccounts(accounts, *accountCount);
            break;
        case 0:
            break;
        }
    } while (choice != 0 && !feof(stdin) && !ferror(stdin));
}

int main(void)
{
    Account accounts[MAX_ACCOUNTS] = {0};
    int accountCount = 0;
    int currentAccount;
    int choice;

    loadAccounts(accounts, &accountCount);
    if (accountCount < 0 || accountCount > MAX_ACCOUNTS) {
        fprintf(stderr, "Account data is invalid.\n");
        return EXIT_FAILURE;
    }

    if (accountCount == 0) {
        initDefaultAdmin(accounts, &accountCount);
        saveAccounts(accounts, accountCount);
    }

    currentAccount = login(accounts, accountCount);
    if (currentAccount < 0 || currentAccount >= accountCount) {
        fprintf(stderr, "Login failed.\n");
        return EXIT_FAILURE;
    }

    if (!loadProducts(&S, PRODUCT_FILE))
{
    printf("Failed to load product data!\n");
}
else
{
    printf("Total products: %d\n", S.productCount);
}

    do {
        int role = accounts[currentAccount].role;
        int maxChoice = role == ROLE_MANAGER ? 6 : 5;

        printMainMenu(role);
        if (!readInt("Choose: ", 0, maxChoice, &choice)) {
            break;
        }

        switch (choice) {
        case 1:
            ProductMenu(role);
            break;
        case 2:
            TableMenu();
            break;
        case 3:
            ReservationMenu(role);
            break;
        case 4:
            OrderMenu(role);
            break;
        case 5:
            updateOwnAccount(accounts, accountCount, currentAccount);
            saveAccounts(accounts, accountCount);
            break;
        case 6:
            if (role == ROLE_MANAGER) {
                manageAccounts(accounts, &accountCount);
            }
            break;
        case 0:
            printf("Exit the program.\n");
            break;
        }
    } while (choice != 0 && !feof(stdin) && !ferror(stdin));

    saveAccounts(accounts, accountCount);
    return ferror(stdin) ? EXIT_FAILURE : EXIT_SUCCESS;
}
