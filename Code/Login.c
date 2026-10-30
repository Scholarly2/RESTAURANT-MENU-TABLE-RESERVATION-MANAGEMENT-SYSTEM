#include <stdio.h>
#include <string.h>
#include "../Include_type_struct/Login.h"
#include "../Include_type_struct/RestaurantTypes.h"
#include "../Include_type_struct/Input.h"
#include <conio.h>
#include <ctype.h>


void inputPassword(char *password, int maxSize) {
    int i = 0;
    char c;

    while (i < maxSize - 1) {
#ifdef _WIN32
        c = _getch();
#else
        c = getchar(); 
#endif
        if (c == '\n' || c == '\r') {
            password[i] = '\0';
            break;
        }
        else if (c == 127 || c == 8) {
            if (i > 0) {
                i--;
                printf("\b \b");
                fflush(stdout);
            }
        }
        else if (c >= 32 && c <= 126) {
            password[i++] = c;
            printf("*");
            fflush(stdout);
        }
    }
    printf("\n");
}

void initDefaultAdmin(Account accounts[], int *accountCount) {
    if (*accountCount == 0) {
        strcpy(accounts[0].username, "admin");
        strcpy(accounts[0].password, "Admin123");
        accounts[0].role = ROLE_MANAGER;
        *accountCount = 1;
    }
}
int login(Account accounts[], int accountCount) {

    char username[30];
    char password[30];
    printf( "================================== " "LOGIN" " ==================================" "\n\n");
    int index = -1;
    while (index == -1) {
        printf("Enter username: ");
		getString(username, sizeof(username));
        trim(username);
        index = findAccountByUsername(accounts, accountCount, username);
        if (index == -1) {
            printf( "Username '%s' not found. Please try again.\n", username);
        }
    }
    while (1) {
        printf("Enter password: ");
        inputPassword(password, sizeof(password));
        if (strcmp(password, accounts[index].password) == 0) {
            printf( "Login successfully!\n" );
            return index;
        } else {
            printf("Invalid password. Please try again.\n" );
        }
    }
}
void printMenuByRole(int role)
{
    if (role != ROLE_MANAGER && role != ROLE_STAFF) {
        printf("Invalid account role.\n");
        return;
    }

    printf("╔════════════════════════════════════════╗\n");
    printf("║         RESTAURANT MANAGEMENT          ║\n");
    printf("╚════════════════════════════════════════╝\n");

    printf("Role: %s\n",
           role == ROLE_MANAGER ? "Manager" : "Staff");

    printf("1. View and search products\n");
    printf("2. Table operations\n");
    printf("3. Reservation management\n");
    printf("4. Orders and bills\n");
    printf("5. Change my password\n");

    if (role == ROLE_MANAGER) {
        printf("6. Product management\n");
        printf("7. Charge settings\n");
        printf("8. Order history\n");
        printf("9. Account management\n");
    }

    printf("0. Exit\n");
    printf("------------------------------------------\n");
}
void updateOwnAccount(Account accounts[], int accountCount, int currentIndex) {
    char newPassword[30];
    printf( "\n============================== " "CHANGE PASSWORD" " ==============================" "\n");
    printf("Current account: %s\n", accounts[currentIndex].username);
    while (1) {
        printf("Enter new password: ");
        inputPassword(newPassword, sizeof(newPassword));
        if (isValidPassword(newPassword)) break;
    }
    strcpy(accounts[currentIndex].password, newPassword);
    printf( "Password updated successfully.\n");
}
int findAccountByUsername(Account accounts[], int accountCount, char username[]) {
    for (int i = 0; i < accountCount; i++) {
        if (strcmp(accounts[i].username, username) == 0) {
            return i;
        }
    }
    return -1;
}
void createStaffAccount(Account accounts[], int *accountCount) {
    if (*accountCount >= MAX_ACCOUNTS) {
        printf( "Account list is full.\n");
        return;
    }
    char username[30];
    char password[30];
    printf("\n============================ CREATE STAFF ACCOUNT ===========================\n" );
    while (1) {
        printf("Enter username: ");
		getString(username, sizeof(username));
        trim(username);
        if (isValidUsername(username)) break;
    }
    if (findAccountByUsername(accounts, *accountCount, username) != -1) {
        printf("Username already exists.\n");
        return;
    }
    while (1) {
        printf("Enter password: ");
        inputPassword(password, sizeof(password));
        if (isValidPassword(password)) break;
    }
    strcpy(accounts[*accountCount].username, username);
    strcpy(accounts[*accountCount].password, password);
    accounts[*accountCount].role = ROLE_STAFF;
    (*accountCount)++;
    printf("Staff account created successfully.\n");
}
void updateUserRole(Account accounts[], int accountCount) {
    char username[30];
    printf( "\n================================ " "CHANGE USER ROLE" " ===========================" "\n");
    printf("Enter username: ");
	getString(username, sizeof(username));
    trim(username);
    int index = findAccountByUsername(accounts, accountCount, username);
    if (index == -1) {
        printf( "Account not found!\n" );
        return;
    }
    if (accounts[index].role == ROLE_MANAGER) {
        accounts[index].role = ROLE_STAFF;
        printf( "Account has been demoted to Staff.\n" );
    }else {
      accounts[index].role = ROLE_MANAGER;
      printf("Account has been promoted to Admin.\n" );
    }
}
void listAccounts(Account accounts[], int accountCount) {
    printf( "\n=============================== ACCOUNT LIST ================================\n");
    for (int i = 0; i < accountCount; i++) {
        printf("Username: %s\n", accounts[i].username);
        if (accounts[i].role == ROLE_MANAGER) printf("Role: MANAGER\n");
        else printf("Role: STAFF\n");
        printf( "-----------------------------------------------------------------------------\n" );
    }
}
void listAccountsNPass(Account accounts[], int accountCount) {
    printf("\n=============================== ACCOUNT LIST ================================\n");
    for (int i = 0; i < accountCount; i++) {
        printf("Username: %s\n", accounts[i].username);
        printf("Password: %s\n", accounts[i].password);
        if (accounts[i].role == ROLE_MANAGER) printf( "Role: MANAGER\n" );
        else printf( "Role: STAFF\n" );
        printf("-----------------------------------------------------------------------------\n");
    }
}
void saveAccounts(Account accounts[], int accountCount) {
    FILE *file = fopen("data/Account.dat", "wb");
    if (file == NULL) {
        printf("Cannot open account file.\n");
        return;
    }
    fwrite(&accountCount, sizeof(int), 1, file);
    fwrite(accounts, sizeof(Account), accountCount, file);
    fclose(file);
}
void loadAccounts(Account accounts[], int *accountCount) {
    FILE *file = fopen("data/Account.dat", "rb");
    if (file == NULL) {
        *accountCount = 0;
        return;
    }
    fread(accountCount, sizeof(int), 1, file);
    fread(accounts, sizeof(Account), *accountCount, file);
    fclose(file);
}