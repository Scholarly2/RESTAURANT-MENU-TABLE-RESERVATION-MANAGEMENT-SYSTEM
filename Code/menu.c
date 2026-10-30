#include <stdio.h>
#include <limits.h>

#include "RestaurantTypes.h"
#include "Input.h"
#include "Login.h"
#include "menu.h"

/* ===================== role-aware menu engine ===================== */

#define MENU_BACK        0
#define MENU_EOF         (-1)
#define MENU_MAX_ENTRIES 16
#define MENU_COUNT(a)    ((int)(sizeof(a) / sizeof((a)[0])))

typedef struct {
    const char *label;
    int action;        /* id handled by the switch of the owning menu (> 0) */
    int managerOnly;   /* TRUE: hidden from staff */
} MenuEntry;

static int menuIsValidRole(int role)
{
    return role == ROLE_MANAGER || role == ROLE_STAFF;
}

static int menuReadChoice(int min, int max, int *choice)
{
    while (!readInt("Choose: ", min, max, choice)) {
        if (feof(stdin) || ferror(stdin)) {
            return FALSE;
        }
    }
    return TRUE;
}

static int menuPick(const char *title, const MenuEntry *entries, int count, int role)
{
    int visible[MENU_MAX_ENTRIES];
    int shown = 0;
    int choice;
    int i;

    printf("\n=== %s ===\n", title);
    for (i = 0; i < count && shown < MENU_MAX_ENTRIES; i++) {
        if (entries[i].managerOnly && role != ROLE_MANAGER) {
            continue;
        }
        visible[shown++] = i;
        printf("%d. %s\n", shown, entries[i].label);
    }
    printf("0. Back\n");

    if (!menuReadChoice(0, shown, &choice)) {
        return MENU_EOF;
    }
    if (choice == 0) {
        return MENU_BACK;
    }
    return entries[visible[choice - 1]].action;
}

/* 1-2 guests -> 2-seat table, 3-4 -> 4-seat, 5-8 -> 8-seat */
static int menuTableTypeForGuests(int guests)
{
    if (guests <= 2) return 2;
    if (guests <= 4) return 4;
    return 8;
}

/* ===================== PRODUCT ===================== */

enum {
    ACT_PRODUCT_SEARCH = 1,
    ACT_PRODUCT_SHOW,
    ACT_PRODUCT_ADD,
    ACT_PRODUCT_UPDATE,
    ACT_PRODUCT_DELETE
};

static const MenuEntry menuProductEntries[] = {
    { "Search products",                   ACT_PRODUCT_SEARCH, FALSE },
    { "Show product list (sort)",          ACT_PRODUCT_SHOW,   FALSE },
    { "Add product",                       ACT_PRODUCT_ADD,    TRUE  },
    { "Update product",                    ACT_PRODUCT_UPDATE, TRUE  },
    { "Delete product (mark unavailable)", ACT_PRODUCT_DELETE, TRUE  }
};

static void menuSearchProduct(void)
{
    int choice;
    int id;
    int type;
    char keyword[NAME_SIZE];

    do {
        printf("\n=== SEARCH PRODUCT ===\n");
        printf("1. Search by ID\n");
        printf("2. Search by name\n");
        printf("3. Search by type\n");
        printf("0. Back\n");

        if (!menuReadChoice(0, 3, &choice)) return;

        switch (choice) {
        case 1:
            if (readInt("Product ID: ", 1, INT_MAX, &id)) {
                searchProductByID(id);
            }
            break;
        case 2:
            if (readName("Name or keyword: ", FALSE, keyword)) {
                searchProductByName(keyword);
            }
            break;
        case 3:
            printf("1. Starter\n");
            printf("2. Main dish\n");
            printf("3. Side dish\n");
            printf("4. Dessert\n");
            printf("5. Drink\n");
            if (readInt("Type (1-5): ", STARTER, DRINK, &type)) {
                searchProductByType(type);
            }
            break;
        default:
            break;
        }
    } while (choice != 0);
}

static void menuShowProductList(void)
{
    int choice;

    do {
        printf("\n=== SHOW PRODUCT LIST ===\n");
        printf("1. Sort by ID\n");
        printf("2. Sort by price (low to high)\n");
        printf("3. Sort by name (A-Z)\n");
        printf("0. Back\n");

        if (!menuReadChoice(0, 3, &choice)) return;

        switch (choice) {
        case 1: sortByID();    break;
        case 2: sortByPrice(); break;
        case 3: sortByName();  break;
        default: break;
        }
    } while (choice != 0);
}

void ProductMenu(int role)
{
    int action;

    if (!menuIsValidRole(role)) {
        puts("Invalid account role.");
        return;
    }
    if (S.productCount < 0 || S.productCount > MAX_PRODUCT) {
        puts("Invalid product data. Load a valid product file first.");
        return;
    }

    for (;;) {
        action = menuPick("PRODUCT MANAGEMENT", menuProductEntries,
                          MENU_COUNT(menuProductEntries), role);
        if (action == MENU_BACK || action == MENU_EOF) return;

        switch (action) {
        case ACT_PRODUCT_SEARCH: menuSearchProduct();   break;
        case ACT_PRODUCT_SHOW:   menuShowProductList(); break;
        case ACT_PRODUCT_ADD:    addProduct();          break;
        case ACT_PRODUCT_UPDATE: updateProduct();       break;
        case ACT_PRODUCT_DELETE: deleteProduct();       break;
        default: break;
        }
    }
}

/* ===================== TABLE (same for both roles) ===================== */

void TableMenu(void)
{
    int choice;
    int tableID;
    int option;
    int guests;
    int index;
    int newStatus;

    if (S.tableCount < 0 || S.tableCount > MAX_TABLES) {
        puts("Invalid table data. Load valid data first.");
        return;
    }

    do {
        printf("\n=== TABLE MANAGEMENT ===\n");
        printf("1. Show tables\n");
        printf("2. Change table status\n");
        printf("3. Find available table\n");
        printf("4. Sort tables by status\n");
        printf("0. Back\n");

        if (!menuReadChoice(0, 4, &choice)) return;

        switch (choice) {
        case 1:
            showTables(S.tables, S.tableCount);
            break;

        case 2:
            showTables(S.tables, S.tableCount);
            if (!readInt("Table ID (0 = back): ", 0, INT_MAX, &tableID)) break;
            if (tableID == 0) break;
            if (findTableByID(&S, tableID) == -1) {
                printf("Table not found.\n");
                break;
            }
            if (!readInt("New status (1 Available, 2 Booked, 3 Serving): ",
                         1, 3, &option)) break;

            newStatus = (option == 1) ? TABLE_AVAILABLE
                      : (option == 2) ? TABLE_BOOKED
                                      : TABLE_SERVING;

            if (updateStatusTable(tableID, newStatus)) {
                printf("Table status updated.\n");
            } else {
                printf("Table status was not changed.\n");
            }
            break;

        case 3:
            if (!readInt("Number of customers (1-8): ", 1, 8, &guests)) break;
            index = findAvailableTable(&S, guests);
            if (index < 0 || index >= S.tableCount) {
                printf("No suitable table available.\n");
            } else {
                showTables(&S.tables[index], 1);
            }
            break;

        case 4:
            sortTableByStatus();
            break;

        default:
            break;
        }
    } while (choice != 0);
}

/* ===================== RESERVATION (same for both roles) ===================== */

static void menuCreateReservation(void)
{
    char name[CUSTOMER_NAME_SIZE];
    char phone[PHONE_SIZE];
    DateTime when;
    int guests;
    int tableID;

    if (!readInt("Number of guests (1-8): ", 1, 8, &guests) ||
        !readName("Customer name: ", TRUE, name) ||
        !readPhone(phone) ||
        !readBookingTime(&when)) {
        printf("Invalid input. Reservation was not created.\n");
        return;
    }

    showTables(S.tables, S.tableCount);
    if (!readInt("Table ID (0 = assign automatically): ", 0, INT_MAX, &tableID)) {
        printf("Invalid input. Reservation was not created.\n");
        return;
    }

    if (!createReservation(name, phone, guests, when,
                           menuTableTypeForGuests(guests), tableID)) {
        printf("Reservation was not created.\n");
    }
}

void ReservationMenu(void)
{
    int choice;
    char phone[PHONE_SIZE];

    if (S.resCount < 0 || S.resCount > MAX_RESERVATIONS) {
        puts("Invalid reservation data. Load valid data first.");
        return;
    }

    do {
        printf("\n=== RESERVATION MANAGEMENT ===\n");
        printf("1. Create reservation\n");
        printf("2. Cancel reservation\n");
        printf("3. Check in reservation\n");
        printf("4. List reservations (sorted by booking time)\n");
        printf("0. Back\n");

        if (!menuReadChoice(0, 4, &choice)) return;

        switch (choice) {
        case 1:
            menuCreateReservation();
            break;

        case 2:
            if (!readPhone(phone)) {
                printf("Invalid phone number.\n");
                break;
            }
            if (!cancelReservation(phone)) {
                printf("Reservation was not cancelled.\n");
            }
            break;

        case 3:
            if (!readPhone(phone)) {
                printf("Invalid phone number.\n");
                break;
            }
            if (!checkInReservation(phone)) {
                printf("Check-in failed.\n");
            }
            break;

        case 4:
            sortBookingByTime();
            printReservation();
            break;

        default:
            break;
        }
    } while (choice != 0);
}

/* ===================== ORDER AND BILL ===================== */

enum {
    ACT_ORDER_CREATE = 1,
    ACT_ORDER_ADD_FOOD,
    ACT_ORDER_UPDATE_FOOD,
    ACT_ORDER_DELETE_FOOD,
    ACT_ORDER_VIEW_OPEN,
    ACT_ORDER_PRE_ORDERED,
    ACT_ORDER_CHECKOUT,
    ACT_ORDER_CANCEL,
    ACT_ORDER_CHARGES,
    ACT_ORDER_HISTORY,
    ACT_ORDER_BILL_HISTORY
};

static const MenuEntry menuOrderEntries[] = {
    { "Create order",                   ACT_ORDER_CREATE,       FALSE },
    { "Add food to order",              ACT_ORDER_ADD_FOOD,     FALSE },
    { "Update food quantity",           ACT_ORDER_UPDATE_FOOD,  FALSE },
    { "Delete food from order",         ACT_ORDER_DELETE_FOOD,  FALSE },
    { "View open orders",               ACT_ORDER_VIEW_OPEN,    FALSE },
    { "View pre-ordered dishes",        ACT_ORDER_PRE_ORDERED,  FALSE },
    { "Checkout and print bill",        ACT_ORDER_CHECKOUT,     FALSE },
    { "Cancel order",                   ACT_ORDER_CANCEL,       FALSE },
    { "Configure charge policy",        ACT_ORDER_CHARGES,      FALSE },
    { "Order history (all orders)",     ACT_ORDER_HISTORY,      TRUE  },
    { "Bill history",                   ACT_ORDER_BILL_HISTORY, TRUE  }
};

static const char *menuOrderTypeName(int type)
{
    switch (type) {
    case DINE_IN:  return "Dine-in";
    case TAKEAWAY: return "Takeaway";
    case DELIVERY: return "Delivery";
    default:       return "Unknown";
    }
}

/* What staff may see: only orders that are still open (no history). */
static void menuShowOpenOrders(void)
{
    int i;
    int found = FALSE;

    for (i = 0; i < S.orderCount && i < MAX_ORDERS; i++) {
        const Order *o = &S.orders[i];

        if (o->orderStatus != ORDER_OPEN) {
            continue;
        }
        found = TRUE;
        printf("\nOrder %d | Table %d | Reservation %d | %s | Guests %d\n",
               o->orderID, o->tableID, o->reservationID,
               menuOrderTypeName(o->orderType), o->NumberCustomer);
        showAllOrderFood(o);
    }

    if (!found) {
        printf("No open orders.\n");
    }
}

static void menuPrintBillHistory(void)
{
    int i;

    if (S.billCount <= 0) {
        printf("No bills found.\n");
        return;
    }

    printf("\n%-8s %-8s %-8s %-16s %14s\n",
           "Bill", "Order", "Table", "Time", "Amount due");
    printf("--------------------------------------------------------------\n");
    for (i = 0; i < S.billCount && i < MAX_BILLS; i++) {
        const Bill *b = &S.bills[i];

        printf("%-8d %-8d %-8d %02d/%02d/%04d %02d:%02d %14.2f\n",
               b->billID, b->orderID, b->tableID,
               b->timeBill.day, b->timeBill.month, b->timeBill.year,
               b->timeBill.hour, b->timeBill.minute,
               b->amountDue);
    }
}

/* Edit one field, then hand the whole policy to configureCharges(). */
static void menuConfigureCharges(void)
{
    ChargePolicy policy;
    double value;
    int choice;
    int changed;

    do {
        policy = S.policy;
        changed = FALSE;

        printf("\n=== CONFIGURE CHARGE POLICY ===\n");
        printf("Dine-in rate     : %.2f%%\n", policy.dineInRate);
        printf("Takeaway rate    : %.2f%%\n", policy.takeawayRate);
        printf("Delivery base fee: %.2f\n", policy.deliveryBase);
        printf("Delivery per km  : %.2f\n", policy.deliveryPerKm);
        printf("1. Change dine-in rate\n");
        printf("2. Change takeaway rate\n");
        printf("3. Change delivery base fee\n");
        printf("4. Change delivery fee per km\n");
        printf("0. Back\n");

        if (!menuReadChoice(0, 4, &choice)) return;

        switch (choice) {
        case 1:
            if (readDouble("New dine-in rate (0-100): ", 0.0, 100.0, &value)) {
                policy.dineInRate = value;
                changed = TRUE;
            }
            break;
        case 2:
            if (readDouble("New takeaway rate (0-100): ", 0.0, 100.0, &value)) {
                policy.takeawayRate = value;
                changed = TRUE;
            }
            break;
        case 3:
            if (readDouble("New delivery base fee: ", 0.0, MAX_PRICE, &value)) {
                policy.deliveryBase = value;
                changed = TRUE;
            }
            break;
        case 4:
            if (readDouble("New delivery fee per km: ", 0.0, MAX_PRICE, &value)) {
                policy.deliveryPerKm = value;
                changed = TRUE;
            }
            break;
        default:
            break;
        }

        if (changed) {
            if (configureCharges(policy)) {
                printf("Charge policy updated.\n");
            } else {
                printf("Charge policy was not changed.\n");
            }
        }
    } while (choice != 0);
}

/*
 * createOrderTable, addOrderFood, updateOrderFood, deleteOrderFood,
 * listPreOrderedDishes and cancelOrder (OrderManager.c) ask for their own
 * input and overwrite their parameters, so the menu only passes placeholders.
 * checkoutOrder uses its parameters, so those are read here with readBillInput.
 */
void OrderMenu(int role)
{
    int action;
    int orderID;
    double discount;
    double tax;

    if (!menuIsValidRole(role)) {
        puts("Invalid account role.");
        return;
    }
    if (S.orderCount < 0 || S.orderCount > MAX_ORDERS ||
        S.billCount < 0 || S.billCount > MAX_BILLS) {
        puts("Invalid order or bill data. Load valid data first.");
        return;
    }

    for (;;) {
        action = menuPick("ORDER AND BILL MANAGEMENT", menuOrderEntries,
                          MENU_COUNT(menuOrderEntries), role);
        if (action == MENU_BACK || action == MENU_EOF) return;

        switch (action) {
        case ACT_ORDER_CREATE:       createOrderTable(DINE_IN, 0, 0, 0, 0.0); break;
        case ACT_ORDER_ADD_FOOD:     addOrderFood(0, 0, 0);                   break;
        case ACT_ORDER_UPDATE_FOOD:  updateOrderFood(0, 0, 0);                break;
        case ACT_ORDER_DELETE_FOOD:  deleteOrderFood(0, 0);                   break;
        case ACT_ORDER_VIEW_OPEN:    menuShowOpenOrders();                    break;
        case ACT_ORDER_PRE_ORDERED:  listPreOrderedDishes(0);                 break;
        case ACT_ORDER_CHECKOUT:
            if (!readBillInput(&orderID, &discount, &tax)) {
                printf("Invalid bill information.\n");
            } else if (!checkoutOrder(orderID, discount, tax)) {
                printf("Checkout failed.\n");
            } else {
                printf("Payment completed.\n");
            }
            break;
        case ACT_ORDER_CANCEL:       cancelOrder(0);                          break;
        case ACT_ORDER_CHARGES:      menuConfigureCharges();                  break;
        case ACT_ORDER_HISTORY:      showAllOrderTable();                     break;
        case ACT_ORDER_BILL_HISTORY: menuPrintBillHistory();                  break;
        default: break;
        }
    }
}