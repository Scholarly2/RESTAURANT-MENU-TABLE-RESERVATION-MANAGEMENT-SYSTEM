#include <stdio.h>
#include <string.h>
#include <limits.h>
#include "common.h"
#include "Input.h"
#include "TableManager.h"
#include "ProductManager.h"
#include "ReservationManager.h"
#include "OrderManager.h"
#include "BillManager.h"
#include "menu.h"

#define MENU_WIDTH    52
#define TYPE_PROMPT   "Type 1 Starter, 2 Main dish, 3 Side dish, 4 Dessert, 5 Drink: "

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(a) ((int)(sizeof(a) / sizeof((a)[0])))
#endif

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
    "Change table status",
    "Find available table",
    "Sort tables by status"
};

static const char *const RESERVATION_ITEMS[] = {
    "Create reservation",
    "Cancel reservation",
    "Sort booking by time",
    "Check in"
};

static const char *const ORDER_ITEMS[] = {
    "Create or select order",
    "Add food",
    "Update food",
    "Delete food",
    "Pre-orders by table",
    "Draft bill",
    "Pay",
    "Cancel order",
    "Orders",
    "Fee settings",
    "Paid bills"
};

/* ===================== DRAWING HELPERS ===================== */

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
    int len   = (int)strlen(text);
    int left, right;

    if (len > inner) len = inner;
    left  = (inner - len) / 2;
    right = inner - len - left;

    printf("|%*s%.*s%*s|\n", left, "", len, text, right, "");
}

static void printMenuItem(int number, const char *label)
{
    char num[8];
    int  labelWidth = MENU_WIDTH - 2 - 6;

    snprintf(num, sizeof(num), "%d.", number);
    printf("|  %-4s%-*.*s|\n", num, labelWidth, labelWidth, label);
}

static void printMenu(const char *title, const char *const items[], int itemCount,
                      const char *zeroLabel)
{
    int i;

    printf("\n");
    printBorder('=');
    printCentered(title);
    printBorder('=');
    for (i = 0; i < itemCount; i++) printMenuItem(i + 1, items[i]);
    printBorder('-');
    printMenuItem(0, zeroLabel);
    printBorder('=');
}

/* ===================== SMALL INPUT / OUTPUT HELPERS ===================== */

static void report(int ok, const char *success, const char *failure)
{
    printf("%s\n", ok ? success : failure);
}

static const char *orderTypeName(int type)
{
    switch (type) {
        case DINE_IN:  return "Dine-in";
        case TAKEAWAY: return "Takeaway";
        case DELIVERY: return "Delivery";
        default:       return "Unknown";
    }
}

static const char *orderStatusName(int status)
{
    switch (status) {
        case ORDER_OPEN:      return "Open";
        case ORDER_PAID:      return "Paid";
        case ORDER_CANCELLED: return "Cancelled";
        default:              return "Unknown";
    }
}

static int readTableID(const char *prompt, int *tableID)
{
    int number, t;

    for (;;) {
        if (!readInt(prompt, 1, INT_MAX, &number)) return FALSE;
        t = findTableByNumber(&S, number);
        if (t != -1) {
            *tableID = S.tables[t].tableID;
            return TRUE;
        }
        printf("Error: table not found\n");
    }
}

/* Ask for an order ID and find it. *index = -1 when it does not exist. FALSE only on end of input. */
static int askOrderIndex(int *orderID, int *index)
{
    if (!readInt("Order ID: ", 1, INT_MAX, orderID)) return FALSE;
    *index = findOrderByID(&S, *orderID);
    if (*index == -1) printf("Order not found\n");
    return TRUE;
}

/* ===================== MODULE 1: MENU ITEMS ===================== */

static int runProductAction(int choice)
{
    char   text[NAME_SIZE];
    double number = 0;
    int    id, type, qty, field, value, confirm;

    switch (choice) {
        case 1:  /* Add */
            if (!readName("Item name: ", FALSE, text))              return FALSE;
            if (!readInt(TYPE_PROMPT, 1, 5, &type))                 return FALSE;
            if (!readDouble("Price: ", 0.01, MAX_PRICE, &number))   return FALSE;
            if (!readInt("Quantity: ", 0, MAX_QUANTITY, &qty))      return FALSE;
            report(addProduct(text, type, number, qty),
                   "Item added", "Cannot add item");
            break;

        case 2:  /* Update */
            if (!readInt("Item ID: ", 1, INT_MAX, &id)) return FALSE;
            if (!readInt("Field 1 Name, 2 Price, 3 Type, 4 Quantity, 5 Status (0 back): ",
                         0, 5, &field)) return FALSE;
            if (field == 0) break;
            text[0] = '\0';
            switch (field) {
                case 1:
                    if (!readName("New name: ", FALSE, text)) return FALSE;
                    break;
                case 2:
                    if (!readDouble("New price: ", 0.01, MAX_PRICE, &number)) return FALSE;
                    break;
                case 3:
                    if (!readInt(TYPE_PROMPT, 1, 5, &value)) return FALSE;
                    number = value;
                    break;
                case 4:
                    if (!readInt("New quantity: ", 0, MAX_QUANTITY, &value)) return FALSE;
                    number = value;
                    break;
                default:
                    if (!readInt("Status 0 Available, 1 Unavailable: ", 0, 1, &value)) return FALSE;
                    number = value;
                    break;
            }
            report(updateProduct(id, field, text, number),
                   "Item updated", "Cannot update item");
            break;

        case 3:  /* Delete: search by name first, then pick the ID (report 3.19) */
            if (!readName("Name to search: ", FALSE, text)) return FALSE;
            searchProductByName(text);
            if (!readInt("Item ID to delete (0 = cancel): ", 0, INT_MAX, &id)) return FALSE;
            if (id == 0) break;
            if (!readInt("Confirm delete 1 Yes 0 No: ", 0, 1, &confirm)) return FALSE;
            if (confirm) report(deleteProduct(id), "Item deleted", "Cannot delete item");
            break;

        case 4:
            if (!readName("Keyword: ", FALSE, text)) return FALSE;
            searchProductByName(text);
            break;

        case 5:
            if (!readInt(TYPE_PROMPT, 1, 5, &type)) return FALSE;
            searchProductByType(type);
            break;

        case 6: sortByPrice(); break;
        case 7: sortByName();  break;
        case 8: printListProduct(S.products, S.productCount); break;
        default: break;
    }
    return TRUE;
}

/* ===================== MODULE 2: TABLES ===================== */

static int changeTableStatus(void)
{
    int number, choice, t, newStatus;

    if (S.tableCount == 0) {
        printf("No table in the system\n");
        return TRUE;
    }
    showTables(S.tables, S.tableCount);

    if (!readInt("Enter table number (0 to quit): ", 0, INT_MAX, &number)) return FALSE;
    if (number == 0) return TRUE;

    t = findTableByNumber(&S, number);
    if (t == -1) {
        printf("Error: table not found\n");
        return TRUE;
    }
    printf("Current status: %s\n", statusToString(S.tables[t].tableStatus));

    if (!readInt("Enter new status (1: Available, 2: Booked, 3: Serving): ",
                 1, 3, &choice)) return FALSE;
    newStatus = (choice == 1) ? TABLE_AVAILABLE
              : (choice == 2) ? TABLE_BOOKED : TABLE_SERVING;

    report(updateStatusTable(S.tables[t].tableID, newStatus),
           "Table status updated", "Table status was not changed");
    return TRUE;
}

static int runTableAction(int choice)
{
    int n, t;

    switch (choice) {
        case 1:
            showTables(S.tables, S.tableCount);
            break;
        case 2:
            return changeTableStatus();
        case 3:
            if (!readInt("Enter number of customers (1-8): ", 1, 8, &n)) return FALSE;
            t = findAvailableTable(&S, n);
            if (t == -1) {
                printf("No suitable table available\n");
            } else {
                printf("Suggested table: number %d (seats up to %d)\n",
                       S.tables[t].tableNumber, S.tables[t].tableType);
            }
            break;
        case 4:
            sortTableByStatus();
            break;
        default:
            break;
    }
    return TRUE;
}

/* ===================== MODULE 3: RESERVATIONS ===================== */

static int runReservationAction(int choice)
{
    int id, confirm;

    switch (choice) {
        case 1:
            report(promptCreateReservation(0),
                   "Reservation created", "Cannot create reservation");
            break;
        case 2:
            sortBookingByTime();                      /* lists the booking IDs */
            if (!readInt("Reservation ID to cancel (0 = back): ", 0, INT_MAX, &id)) return FALSE;
            if (id == 0) break;
            if (!readInt("Confirm cancellation 1 Yes 0 No: ", 0, 1, &confirm)) return FALSE;
            if (confirm) report(cancelReservation(id),
                                "Reservation cancelled", "Cannot cancel reservation");
            break;
        case 3:
            sortBookingByTime();
            break;
        case 4:
            sortBookingByTime();
            if (!readInt("Reservation ID to check in (0 = back): ", 0, INT_MAX, &id)) return FALSE;
            if (id == 0) break;
            report(checkInReservation(id), "Checked in", "Cannot check in");
            break;
        default:
            break;
    }
    return TRUE;
}

/* ===================== MODULE 4: ORDERS & BILLING ===================== */

static void listOrders(void)
{
    int i;

    printf("%-6s | %-9s | %-6s | %-9s | %s\n", "Order", "Type", "Table", "Status", "Items");
    for (i = 0; i < S.orderCount; i++) {
        printf("%-6d | %-9s | %-6d | %-9s | %d\n",
               S.orders[i].orderID,
               orderTypeName(S.orders[i].orderType),
               S.orders[i].tableID,
               orderStatusName(S.orders[i].orderStatus),
               S.orders[i].itemCount);
    }
}

static void listPaidBills(void)
{
    int i;

    printf("%-6s | %-6s | %-9s | %s\n", "Bill", "Order", "Type", "Amount due");
    for (i = 0; i < S.billCount; i++) {
        if (S.bills[i].billID <= 0) continue;          /* billID 0 = demo (report 3.8) */
        printf("%-6d | %-6d | %-9s | %.2f\n",
               S.bills[i].billID, S.bills[i].orderID,
               orderTypeName(S.bills[i].orderType), S.bills[i].amountDue);
    }
}

static int feeSettings(void)
{
    ChargePolicy p = S.policy;
    State       *c;

    printf("Current: dine-in %.2f%%, takeaway %.2f%%, delivery %.2f + %.2f per km\n",
           p.dineInRate, p.takeawayRate, p.deliveryBase, p.deliveryPerKm);

    if (!readDouble("Dine-in rate percent: ", 0, 100, &p.dineInRate))        return FALSE;
    if (!readDouble("Takeaway rate percent: ", 0, 100, &p.takeawayRate))     return FALSE;
    if (!readDouble("Delivery base fee: ", 0, MAX_PRICE, &p.deliveryBase))   return FALSE;
    if (!readDouble("Delivery fee per km: ", 0, MAX_PRICE, &p.deliveryPerKm)) return FALSE;

    c = copyState(&S);
    if (c == NULL) {
        printf("Cannot save fee settings\n");
        return TRUE;
    }
    c->policy = p;
    report(commitChange(c, "UPDATE_POLICY", 0, "-", "-"),
           "Fee settings saved", "Cannot save fee settings");
    return TRUE;
}

static int runOrderAction(int choice)
{
    int    id, o, productID, qty, type, resID, tableID, guests, t, confirm;
    double distance, discount, tax;
    Bill   b;

    switch (choice) {
        case 1:  /* Create or select order */
            resID = 0; tableID = 0; guests = 0; distance = 0;
            if (!readInt("Order type 1 Dine-in, 2 Takeaway, 3 Delivery: ",
                         DINE_IN, DELIVERY, &type)) return FALSE;
            if (type == DINE_IN) {
                if (!readInt("Reservation ID (0 = walk-in): ", 0, INT_MAX, &resID)) return FALSE;
                if (resID == 0) {
                    if (!readTableID("Table number: ", &tableID)) return FALSE;
                    t = findTableByID(&S, tableID);
                    if (!readInt("Guests: ", 1, S.tables[t].tableType, &guests)) return FALSE;
                }
            } else if (type == DELIVERY) {
                if (!readDouble("Distance (km): ", 0, MAX_DISTANCE, &distance)) return FALSE;
            }
            report(createOrderTable(type, resID, tableID, guests, distance),
                   "Order created or selected", "Cannot create order");
            break;

        case 2:  /* Add food */
            if (!askOrderIndex(&id, &o)) return FALSE;
            if (o == -1) break;
            if (!readInt("Item ID: ", 1, INT_MAX, &productID)) return FALSE;
            if (!readInt("Quantity: ", 1, MAX_QUANTITY, &qty)) return FALSE;
            report(addOrderFood(id, productID, qty), "Food added", "Cannot add food");
            break;

        case 3:  /* Update food */
            if (!askOrderIndex(&id, &o)) return FALSE;
            if (o == -1) break;
            if (!readInt("Item ID: ", 1, INT_MAX, &productID)) return FALSE;
            if (!readInt("New quantity: ", 1, MAX_QUANTITY, &qty)) return FALSE;
            report(updateOrderFood(id, productID, qty), "Food updated", "Cannot update food");
            break;

        case 4:  /* Delete food */
            if (!askOrderIndex(&id, &o)) return FALSE;
            if (o == -1) break;
            if (!readInt("Item ID: ", 1, INT_MAX, &productID)) return FALSE;
            if (!readInt("Confirm delete 1 Yes 0 No: ", 0, 1, &confirm)) return FALSE;
            if (confirm) report(deleteOrderFood(id, productID), "Food removed", "Cannot remove food");
            break;

        case 5:  /* Pre-orders by table */
            if (!readTableID("Table number: ", &tableID)) return FALSE;
            listPreOrderedDishes(tableID);
            break;

        case 6:  /* Draft bill (no payment) */
            if (!askOrderIndex(&id, &o)) return FALSE;
            if (o == -1) break;
            if (!readDouble("Discount percent: ", 0, 100, &discount)) return FALSE;
            if (!readDouble("Tax percent: ", 0, 100, &tax)) return FALSE;
            if (calculateBill(&S.orders[o], S.policy, discount, tax, &b)) displayBillReport(&b);
            else printf("Cannot calculate the bill\n");
            break;

        case 7:  /* Pay */
            if (!askOrderIndex(&id, &o)) return FALSE;
            if (o == -1) break;
            report(promptPayOrder(o), "Payment completed", "Payment not completed");
            break;

        case 8:  /* Cancel order */
            if (!askOrderIndex(&id, &o)) return FALSE;
            if (o == -1) break;
            if (!readInt("Confirm cancellation 1 Yes 0 No: ", 0, 1, &confirm)) return FALSE;
            if (confirm) report(cancelOrder(id), "Order cancelled", "Cannot cancel order");
            break;

        case 9:  listOrders();    break;
        case 10: return feeSettings();
        case 11: listPaidBills(); break;
        default: break;
    }
    return TRUE;
}

static int runAction(int module, int choice)
{
    switch (module) {
        case 1:  return runProductAction(choice);
        case 2:  return runTableAction(choice);
        case 3:  return runReservationAction(choice);
        case 4:  return runOrderAction(choice);
        default: return TRUE;
    }
}
static int runReservationAction(int choice) {
    char phoneNumber[PHONE_SIZE];
    switch (choice) {
 char name[CUSTOMER_NAME_SIZE];
    char phone[PHONE_SIZE];
    DateTime time;
    int guests, neededType;
    int id, confirm;
    int ok = TRUE;
    switch (choice)
    {
        case 1:
            // Create reservation
            if (!readName("Customer name: ", TRUE, name) ||
                !readPhone(phone) ||
                !readInt("Number of guests (1-8): ", 1, 8, &guests) ||
                !readBookingTime(&time))
            {
                return FALSE;
            }
            if (guests <= 2)
                neededType = 2;
            else if (guests <= 4)
                neededType = 4;
            else
                neededType = 8;

            ok = createReservation(name, phone, guests,
                     time, neededType, 0);
            break;
        }
        case 2:
            if (!readPhone(phoneNumber))
                return FALSE;
            if (!cancelReservation(phoneNumber))
                printf("Failed to cancel reservation!\n");
            return TRUE;
        case 3:
            // Sort and print reservations
            sortBookingByTime();
            printReservation(S.reservations, S.resCount);
            return TRUE;
        case 4:
            sortBookingByTime();
            printReservation();
            return TRUE;
        case 4:
    if (!readPhone(phone))
    {
        return FALSE;
    }
    ok = CheckInReservation(phone);
    break;
        default:
            return TRUE;
}
if (!ok){
    printf("Reservation action rejected!\n");
}
return TRUE;
}

/* ===================== NAVIGATION (3.28 / 3.29) ===================== */

void showMainMenu(void)
{
    int choice;
    int count = ARRAY_SIZE(MAIN_ITEMS);

    for (;;) {
        printMenu("RESTAURANT MANAGEMENT SYSTEM", MAIN_ITEMS, count, "Exit");
        if (!readInt(" >> Enter your choice: ", 0, count, &choice)) break;
        if (choice == 0) break;
        showModuleMenu(choice);
    }

    printf("\n");
    printBorder('=');
    printCentered("Thank you & goodbye!");
    printBorder('=');
}
void showModuleMenu(int module)
{
    const char        *title;
    const char *const *items;
    int                count, choice;

    switch (module) {
        case 1:
            title = "MENU ITEM MANAGEMENT";
            items = PRODUCT_ITEMS;     count = ARRAY_SIZE(PRODUCT_ITEMS);
            break;
        case 2:
            title = "TABLE MANAGEMENT";
            items = TABLE_ITEMS;       count = ARRAY_SIZE(TABLE_ITEMS);
            break;
        case 3:
            title = "TABLE RESERVATION";
            items = RESERVATION_ITEMS; count = ARRAY_SIZE(RESERVATION_ITEMS);
            break;
        case 4:
            title = "ORDERING & BILLING";
            items = ORDER_ITEMS;       count = ARRAY_SIZE(ORDER_ITEMS);
            break;
        default:
            return;                   
    }

    for (;;) {
        printMenu(title, items, count, "Back");
        if (!readInt(" >> Enter your choice: ", 0, count, &choice)) return;
        if (choice == 0) return;
        if (!runAction(module, choice)) return;
    }
}
