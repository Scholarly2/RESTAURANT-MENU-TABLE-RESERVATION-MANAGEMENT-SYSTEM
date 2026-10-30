#include <stdio.h>
#include <string.h>
#include <limits.h>
#include "FileHelper.h"
#include "Input.h"
#include "Login.h"
#include "menu.h"
#include "RestaurantTypes.h"

Account *menuAccount = NULL;

#define MENU_WIDTH 52

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(a) ((int)(sizeof(a) / sizeof((a)[0])))
#endif

static const char *const MAIN_ITEMS[] = {
    "Menu Item Management", "Table Management", "Table Reservation",
    "Ordering & Billing", "Accounts & Passwords"
};
static const char *const PRODUCT_ITEMS[] = {
    "Add new item", "Update item", "Delete item (set unavailable)",
    "Search products", "View and sort products"
};
static const char *const TABLE_ITEMS[] = {
    "Show tables", "Change table status", "Find available table", "Sort tables by status"
};
static const char *const RESERVATION_ITEMS[] = {
    "Create reservation", "Cancel reservation", "Sort booking by time", "Check in"
};
static const char *const ORDER_ITEMS[] = {
    "Create order", "Add food", "Update food", "Delete food",
    "View order items", "Pre-orders by table", "Pay", "Cancel order",
    "Fee settings", "Order list / history", "Paid bills"
};
static const char *const ACCOUNT_ITEMS[] = {
    "Create staff account", "Change account role", "View account list",
    "Change my password", "View account list and passwords"
};

// ===================== DRAWING HELPERS =====================

static void printBorder(char fill)
{
    int i;
    putchar('+');
    for (i = 0; i < MENU_WIDTH - 2; i++) putchar(fill);
    printf("+\n");
}

static void printCentered(const char *text)
{
    int inner = MENU_WIDTH - 2;
    int len = (int)strlen(text);
    int left, right;
    if (len > inner) len = inner;
    left = (inner - len) / 2;
    right = inner - len - left;
    printf("|%*s%.*s%*s|\n", left, "", len, text, right, "");
}

static void printMenuItem(int number, const char *label)
{
    char num[8];
    int labelWidth = MENU_WIDTH - 2 - 6;
    snprintf(num, sizeof(num), "%d.", number);
    printf("|  %-4s%-*.*s|\n", num, labelWidth, labelWidth, label);
}

// SỬA: Kiểm tra theo cả module và lựa chọn; cùng số 1 nhưng mỗi menu có quyền khác nhau.
// checkPermission(role, choice) in Login.c checks main-menu choices only.
// module 0: menu chính; 1..4: nghiệp vụ; 5: tài khoản.
static int isMenuChoiceAllowed(int module, int choice, int role)
{
    int maxChoice;

    if (role != ROLE_MANAGER && role != ROLE_STAFF) return FALSE;
    switch (module) {
    case 0: maxChoice = ARRAY_SIZE(MAIN_ITEMS); break;
    case 1: maxChoice = ARRAY_SIZE(PRODUCT_ITEMS); break;
    case 2: maxChoice = ARRAY_SIZE(TABLE_ITEMS); break;
    case 3: maxChoice = ARRAY_SIZE(RESERVATION_ITEMS); break;
    case 4: maxChoice = ARRAY_SIZE(ORDER_ITEMS); break;
    case 5: maxChoice = ARRAY_SIZE(ACCOUNT_ITEMS); break;
    default: return FALSE;
    }
    if (choice < 0 || choice > maxChoice) return FALSE;
    if (choice == 0) return TRUE;
    if (role == ROLE_MANAGER) return TRUE;

    switch (module) {
    case 0: return choice >= 1 && choice <= 4;
    case 1: return choice >= 4 && choice <= ARRAY_SIZE(PRODUCT_ITEMS);
    case 2: return choice >= 1 && choice <= ARRAY_SIZE(TABLE_ITEMS);
    case 3: return choice >= 1 && choice <= ARRAY_SIZE(RESERVATION_ITEMS);
    case 4: return choice >= 1 && choice <= ARRAY_SIZE(ORDER_ITEMS) && choice != 10;
    default: return FALSE;
    }
}

static void printMenu(const char *title, const char *const items[], int itemCount,
                      const char *zeroLabel, int module, int role)
{
    int i;
    printf("\n");
    printBorder('=');
    printCentered(title);
    printBorder('=');
    for (i = 0; i < itemCount; i++) {
        // SỬA: Ẩn mục bị cấm nhưng giữ nguyên số lựa chọn cho cả hai role.
        if (isMenuChoiceAllowed(module, i + 1, role)) printMenuItem(i + 1, items[i]);
    }
    printBorder('-');
    printMenuItem(0, zeroLabel);
    printBorder('=');
}

// ===================== SMALL INPUT / OUTPUT HELPERS =====================

static void report(int ok, const char *success, const char *failure)
{
    printf("%s\n", ok ? success : failure);
}

static const char *orderTypeName(int type)
{
    switch (type) {
    case DINE_IN: return "Dine-in";
    case TAKEAWAY: return "Takeaway";
    case DELIVERY: return "Delivery";
    default: return "Unknown";
    }
}

static int readTableID(const char *prompt, int *tableID)
{
    int index;
    *tableID = 0;
    if (S.tableCount == 0) {
        puts("No table in the system.");
        return TRUE;
    }
    for (;;) {
        if (!readInt(prompt, 0, INT_MAX, tableID)) return FALSE;
        if (*tableID == 0) return TRUE;
        // SỬA: Header có findTableByID, không có findTableByNumber; nhập đúng tableID.
        index = findTableByID(&S, *tableID);
        if (index >= 0 && index < S.tableCount) return TRUE;
        puts("Table not found. Please enter another ID.");
    }
}

// Ask for an order ID and find it. FALSE only on end of input.
// SỬA: Chỉ chọn đơn OPEN; nhập lại ID không tồn tại/đơn đã đóng.
// *index = -1 khi chọn 0 quay lại hoặc không có đơn OPEN.
static int askOrderIndex(int *orderID, int *index)
{
    int i;
    *orderID = 0;
    *index = -1;
    for (i = 0; i < S.orderCount; i++) {
        if (S.orders[i].orderStatus == ORDER_OPEN) break;
    }
    if (i == S.orderCount) {
        puts("No open orders.");
        return TRUE;
    }
    for (;;) {
        if (!readInt("Open order ID (0 Back): ", 0, INT_MAX, orderID)) return FALSE;
        if (*orderID == 0) return TRUE;
        i = findOrderByID(&S, *orderID);
        if (i >= 0 && i < S.orderCount && S.orders[i].orderStatus == ORDER_OPEN) {
            *index = i;
            return TRUE;
        }
        puts("Open order not found. Please enter another ID.");
    }
}

// ===================== MODULE 2: TABLES =====================

static int changeTableStatus(void)
{
    int tableID, index, choice, newStatus;
    showTables(S.tables, S.tableCount);
    if (!readTableID("Table ID (0 Back): ", &tableID)) return FALSE;
    if (tableID == 0) return TRUE;
    index = findTableByID(&S, tableID);
    if (index < 0 || index >= S.tableCount) return TRUE;

    // SỬA: Dùng showTables đã khai báo, bỏ statusToString chưa có trong header.
    showTables(&S.tables[index], 1);
    if (!readInt("New status (1 Available, 2 Booked, 3 Serving): ", 1, 3, &choice)) return FALSE;
    newStatus = choice == 1 ? TABLE_AVAILABLE : (choice == 2 ? TABLE_BOOKED : TABLE_SERVING);
    report(updateStatusTable(tableID, newStatus), "Table status updated.", "Table status was not changed.");
    return TRUE;
}

// ===================== MODULE 4: ORDERS & BILLING =====================

static void listOrders(int role)
{
    if (role != ROLE_MANAGER) {
        puts("Permission denied. Manager access is required.");
        return;
    }
    showAllOrderTable();
}

static void listPaidBills(void)
{
    int i;
    // SỬA: Staff vẫn được xem hóa đơn đã thanh toán theo yêu cầu "còn lại full quyền".
    printf("%-6s | %-6s | %-9s | %s\n", "Bill", "Order", "Type", "Amount due");
    for (i = 0; i < S.billCount; i++) {
        if (S.bills[i].billID <= 0) continue; // billID 0 = demo (report 3.8)
        printf("%-6d | %-6d | %-9s | %.2f\n", S.bills[i].billID, S.bills[i].orderID,
               orderTypeName(S.bills[i].orderType), S.bills[i].amountDue);
    }
}

static int feeSettings(void)
{
    ChargePolicy p = S.policy;
    printf("Current: dine-in %.2f%%, takeaway %.2f%%, delivery %.2f + %.2f per km\n",
           p.dineInRate, p.takeawayRate, p.deliveryBase, p.deliveryPerKm);
    if (!readDouble("Dine-in rate percent: ", 0, 100, &p.dineInRate) ||
        !readDouble("Takeaway rate percent: ", 0, 100, &p.takeawayRate) ||
        !readDouble("Delivery base fee: ", 0, MAX_PRICE, &p.deliveryBase) ||
        !readDouble("Delivery fee per km: ", 0, MAX_PRICE, &p.deliveryPerKm)) return FALSE;
    // SỬA: Gọi hàm nghiệp vụ đã khai báo; không malloc/copyState trong menu.
    report(configureCharges(p), "Fee settings saved.", "Cannot save fee settings.");
    return TRUE;
}

// ===================== ACTIONS DECLARED IN menu.h =====================
// menu.h contains declarations; menu.c implements navigation once.
// Login.c owns printMenuByRole() and the main-menu checkPermission().

int runAction(int module, int choice)
{
    if (S.productCount < 0 || S.productCount > MAX_PRODUCT ||
        S.tableCount < 0 || S.tableCount > MAX_TABLES ||
        S.resCount < 0 || S.resCount > MAX_RESERVATIONS ||
        S.orderCount < 0 || S.orderCount > MAX_ORDERS ||
        S.billCount < 0 || S.billCount > MAX_BILLS) {
        puts("Invalid restaurant data.");
        return FALSE;
    }
    switch (module) {
    case 1: return runProductAction(choice);
    case 2: return runTableAction(choice);
    case 3: return runReservationAction(choice);
    case 4: return runOrderAction(choice);
    default: return FALSE;
    }
}

int runProductAction(int choice)
{
    int role = menuAccount != NULL ? menuAccount->role : 0;
    if (role != ROLE_MANAGER && role != ROLE_STAFF) {
        puts("Please log in with a valid account first.");
        return FALSE;
    }
    if (!isMenuChoiceAllowed(1, choice, role)) {
        puts("Permission denied or invalid choice.");
        return TRUE;
    }
    switch (choice) {
    case 1: addProduct(); break;
    case 2: updateProduct(); break;
    case 3: deleteProduct(); break;
    case 4: if (!searchProductMenu()) return FALSE; break;
    case 5: if (!showProductListMenu()) return FALSE; break;
    }
    return !(feof(stdin) || ferror(stdin));
}

int runTableAction(int choice)
{
    int role = menuAccount != NULL ? menuAccount->role : 0;
    int guests, index;

    if (role != ROLE_MANAGER && role != ROLE_STAFF) {
        puts("Please log in with a valid account first.");
        return FALSE;
    }
    if (!isMenuChoiceAllowed(2, choice, role)) {
        puts("Permission denied or invalid choice.");
        return TRUE;
    }
    switch (choice) {
    case 1: showTables(S.tables, S.tableCount); break;
    case 2: if (!changeTableStatus()) return FALSE; break;
    case 3:
        if (!readInt("Number of customers (1-8): ", 1, 8, &guests)) return FALSE;
        index = findAvailableTable(&S, guests);
        // SỬA: findAvailableTable trả INDEX, không phải tableID.
        if (index < 0 || index >= S.tableCount) puts("No suitable table available.");
        else showTables(&S.tables[index], 1);
        break;
    case 4: sortTableByStatus(); break;
    }

    return !(feof(stdin) || ferror(stdin));
}

int runReservationAction(int choice)
{
    int role = menuAccount != NULL ? menuAccount->role : 0;
    char name[CUSTOMER_NAME_SIZE], phone[PHONE_SIZE];
    int guests, tableType, tableID, index, confirm;
    DateTime bookingTime;

    if (role != ROLE_MANAGER && role != ROLE_STAFF) {
        puts("Please log in with a valid account first.");
        return FALSE;
    }
    if (!isMenuChoiceAllowed(3, choice, role)) {
        puts("Permission denied or invalid choice.");
        return TRUE;
    }
    switch (choice) {
    case 1:
        // SỬA: Bỏ promptCreateReservation chưa khai báo; nhập rồi gọi createReservation.
        if (!readName("Customer name: ", TRUE, name) || !readPhone(phone) ||
            !readInt("Number of customers (1-8): ", 1, 8, &guests) ||
            !readBookingTime(&bookingTime)) return FALSE;
        tableType = guests <= 2 ? 2 : (guests <= 4 ? 4 : 8);
        showTables(S.tables, S.tableCount);
        for (;;) {
            if (!readInt("Preferred table ID (0 Auto-select): ", 0, INT_MAX, &tableID)) return FALSE;
            if (tableID == 0) break;
            index = findTableByID(&S, tableID);
            if (index >= 0 && index < S.tableCount &&
                S.tables[index].tableStatus == TABLE_AVAILABLE &&
                S.tables[index].tableType >= tableType) break;
            puts("Choose an available table with enough seats.");
        }
        report(createReservation(name, phone, guests, bookingTime, tableType, tableID),
               "Reservation created.", "Cannot create reservation.");
        break;
    case 2:
        // SỬA: cancelReservation nhận const char *phoneNumber, không nhận ID.
        if (!readPhone(phone)) return FALSE;
        if (!readInt("Confirm cancellation (1 Yes, 0 No): ", 0, 1, &confirm)) return FALSE;
        if (confirm) report(cancelReservation(phone), "Reservation cancelled.", "Cannot cancel reservation.");
        break;
    case 3: sortBookingByTime(); printReservation(); break;
    case 4:
        // SỬA: checkInReservation cũng nhận số điện thoại theo header mới.
        if (!readPhone(phone)) return FALSE;
        report(checkInReservation(phone), "Checked in.", "Cannot check in.");
        break;
    }

    return !(feof(stdin) || ferror(stdin));
}

int runOrderAction(int choice)
{
    int role = menuAccount != NULL ? menuAccount->role : 0;
    int id, orderIndex, quantity, confirm;
    double discount, tax;

    if (role != ROLE_MANAGER && role != ROLE_STAFF) {
        puts("Please log in with a valid account first.");
        return FALSE;
    }
    if (!isMenuChoiceAllowed(4, choice, role)) {
        puts("Permission denied or invalid choice.");
        return TRUE;
    }
    switch (choice) {
    /* Current OrderManager.c reads IDs/type/distance itself. Do not ask
     * for those values a second time here. It replaces the zero arguments.
     * updateOrderFood() reads its IDs, but uses the supplied quantity.
     */
    case 1:
        report(createOrderTable(0, 0, 0, 0, 0),
               "Order action completed.", "Cannot create or select order.");
        break;
    case 2:
        report(addOrderFood(0, 0, 0), "Food action completed.", "Cannot add food.");
        break;
    case 3:
        if (!readInt("New quantity: ", 1, MAX_QUANTITY, &quantity)) return FALSE;
        report(updateOrderFood(0, 0, quantity), "Food action completed.", "Cannot update food.");
        break;
    case 4:
        report(deleteOrderFood(0, 0), "Food action completed.", "Food was not removed.");
        break;
    case 5: // View items of an open order; closed orders belong to history.
        if (!askOrderIndex(&id, &orderIndex)) return FALSE;
        if (orderIndex != -1) showAllOrderFood(&S.orders[orderIndex]);
        break;
    case 6:
        /* The companion OrderManager.c removes the unfiltered history print. */
        listPreOrderedDishes(0);
        break;
    case 7: // Preview, then pay only after confirmation
        if (!askOrderIndex(&id, &orderIndex)) return FALSE;
        if (orderIndex == -1) break;
        if (!readDouble("Discount percent: ", 0, 100, &discount) ||
            !readDouble("Tax percent: ", 0, 100, &tax)) return FALSE;
        if (!printBillReport(id, discount, tax)) {
            puts("Cannot display the bill.");
            break;
        }
        if (!readInt("Confirm payment (1 Yes, 0 No): ", 0, 1, &confirm)) return FALSE;
        if (confirm) report(checkoutOrder(id, discount, tax), "Payment completed.", "Payment not completed.");
        break;
    case 8: // Cancel order
        if (!askOrderIndex(&id, &orderIndex)) return FALSE;
        if (orderIndex == -1) break;
        if (!readInt("Confirm cancellation (1 Yes, 0 No): ", 0, 1, &confirm)) return FALSE;
        if (confirm) report(cancelOrder(id), "Order cancelled.", "Cannot cancel order.");
        break;
    case 9: if (!feeSettings()) return FALSE; break;
    case 10: listOrders(role); break;
    case 11: listPaidBills(); break;
    }

    return !(feof(stdin) || ferror(stdin));
}

// ===================== NAVIGATION =====================

void showMainMenu(void)
{
    int role = menuAccount != NULL ? menuAccount->role : 0;
    printMenuByRole(role);
}

void showModuleMenu(int module)
{
    const char *title;
    const char *const *items;
    int count, choice, role;

    switch (module) {
    case 1: title = "MENU ITEM MANAGEMENT"; items = PRODUCT_ITEMS; count = ARRAY_SIZE(PRODUCT_ITEMS); break;
    case 2: title = "TABLE MANAGEMENT"; items = TABLE_ITEMS; count = ARRAY_SIZE(TABLE_ITEMS); break;
    case 3: title = "TABLE RESERVATION"; items = RESERVATION_ITEMS; count = ARRAY_SIZE(RESERVATION_ITEMS); break;
    case 4: title = "ORDERING & BILLING"; items = ORDER_ITEMS; count = ARRAY_SIZE(ORDER_ITEMS); break;
    default: return;
    }
    for (;;) {
        role = menuAccount != NULL ? menuAccount->role : 0;
        if (role != ROLE_MANAGER && role != ROLE_STAFF) {
            puts("Please log in with a valid account first.");
            return;
        }
        printMenu(title, items, count, "Back", module, role);
        if (!readInt("Choice: ", 0, count, &choice)) return;
        if (choice == 0) return;
        if (!runAction(module, choice)) return;
    }
}

/* productMenu() is defined only in ProductManager.c and delegates here. */
void tableMenu(void)       { showModuleMenu(2); }
void reservationMenu(void) { showModuleMenu(3); }
void orderMenu(void)       { showModuleMenu(4); }

void accountMenu(Account accounts[], int *accountCount, int currentIndex)
{
    int choice;
    if (accounts == NULL || accountCount == NULL) return;
    for (;;) {
        if (*accountCount <= 0 || *accountCount > MAX_ACCOUNTS ||
            currentIndex < 0 || currentIndex >= *accountCount) {
            puts("Invalid account data.");
            return;
        }
        // SỬA: Kiểm tra quyền thực mỗi lượt, kể cả khi Manager tự chuyển xuống Staff.
        if (menuAccount != &accounts[currentIndex] ||
            menuAccount->role != ROLE_MANAGER) {
            puts("Permission denied. Manager access is required.");
            return;
        }
        printMenu("ACCOUNTS & PASSWORDS", ACCOUNT_ITEMS, ARRAY_SIZE(ACCOUNT_ITEMS), "Back", 5, ROLE_MANAGER);
        if (!readInt(" >> Enter your choice: ", 0, ARRAY_SIZE(ACCOUNT_ITEMS), &choice)) return;
        if (choice == 0) return;
        switch (choice) {
        case 1:
            createStaffAccount(accounts, accountCount);
            if (*accountCount <= 0 || *accountCount > MAX_ACCOUNTS) {
                puts("Invalid account data.");
                return;
            }
            saveAccounts(accounts, *accountCount);
            break;
        case 2:
            listAccounts(accounts, *accountCount);
            updateUserRole(accounts, *accountCount);
            saveAccounts(accounts, *accountCount);
            break;
        case 3: listAccounts(accounts, *accountCount); break;
        case 4:
            updateOwnAccount(accounts, *accountCount, currentIndex);
            saveAccounts(accounts, *accountCount);
            break;
        case 5:
            // Manager được gọi hàm có sẵn trong Login.h; Staff không vào được menu này.
            listAccountsNPass(accounts, *accountCount);
            break;
        }
        if (feof(stdin) || ferror(stdin)) return;
    }
}
