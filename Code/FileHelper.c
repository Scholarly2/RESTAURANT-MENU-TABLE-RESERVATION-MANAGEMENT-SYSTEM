#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <limits.h>
#include "FileHelper.h"
#include "RestaurantTypes.h"
#include "Input.h"
#include "common.h"
#define ORDER_HISTORY_FILE "Data/OrderHistory.txt"
#define BILL_HISTORY_FILE "Data/BillHistory.txt"
#define dataFile "products.txt"

int loadProducts(State *changing, const char *path)
{
    FILE *file;
    MenuItem products[MAX_PRODUCT] = {0};

    char line[512];
    char versionText[16], countText[16], lastIDText[16];
    char idText[32], name[NAME_SIZE], typeText[16], priceText[64], quantityText[32], statusText[32];

    int version, count, lastID;
    int id, type, status, quantity;
    int i,j, used;
    double price;

    if (changing ==NULL || path == NULL)
    {
        return FALSE;
    }

    file = fopen(path, "r");
    if (file == NULL)
    {
        printf("Failed to open file: %s\n", path);
        return FALSE;
    }

    if (fgets(line, sizeof(line), file) == NULL)
    {
        fclose(file);
        return FALSE;
    }
    
    used = 0;
    
    if (sscanf(line, "PRODUCT|%15[^|]|%15[^\r\n]%n",
        versionText, countText, &used) != 3)
        {
            fclose(file);
            return FALSE;
        }
    
    if (used == 0 || strspn(line + used, "\r\n") != strlen(line + used))
    {
        fclose(file);
        return FALSE;
    }

    if (!parseInt(versionText, 2, 2, &version) || !parseInt(countText, 0, MAX_PRODUCT, &count) || !parseInt(lastIDText, 0, INT_MAX, &lastID))
    {
        fclose(file);
        return FALSE;
    }
    
    for(i = 0; i< count; i++)
    {
        if (fgets(line, sizeof(line), file) == NULL)
        {
            fclose(file);
            return FALSE;
        }

        used = 0;
        if (sscanf(line, "%31[^|]|%99[^|]|%63[^|]|%31[^|]|%31[^|]|%31[^\r\n]%n",
            idText, name, typeText, priceText, quantityText, statusText, &used) != 6)
        {
            fclose(file);
            return FALSE;
        }

        if (used == 0 || strspn(line + used, "\r\n") != strlen(line + used))
        {
            fclose(file);
            return FALSE;
        }
        //check data 
        if (!parseInt(idText, 1, INT_MAX, &id) ||
            !isValidName(name) ||
            !parseInt(typeText, STARTER, DRINK, &type) ||
            !parsePrice(priceText, &price) ||
            !parseInt(quantityText, 0, MAX_QUANTITY, &quantity) ||
            !parseInt(statusText, PRODUCT_AVAILABLE, PRODUCT_UNAVAILABLE, &status))
        {
            fclose(file);
            return FALSE;
        }

        if(id > lastID)
        {
            fclose(file);
            return FALSE;
        }

        if(fabs(price) - round(price * 100) / 100 > 0.0001)
        {
            fclose(file);
            return FALSE;
        }

        for(j=0; i<i; j++)
        {
            if(products[j].productID == id) //checkID
            {
                fclose(file);
                return FALSE;
            }
        }

        products[i].productID = id;
        strcpy(products[i].productName, name);
        products[i].productPrice = price;
        products[i].productType = (enum ProductType)type;
        products[i].quantity = quantity;
        products[i].productStatus = (enum ProductStatus)status;

    }

    if (fgetc(file) != EOF || ferror(file))
    {
        fclose(file);
        return FALSE;
    }

    if(fclose(file) != 0)
    {
        return FALSE;
    }

    //update product if read successfully
    memcpy(changing->products, products, sizeof(products));
    changing->productCount = count;
    changing->lastProductID = lastID;
    return TRUE;
}

int saveProducts(const State *s, const char *path)
{
    FILE *file;
    int i,j;
    int success = TRUE;

    if (s==NULL || path == NULL || s->productCount < 0 || s->productCount > MAX_PRODUCT || s->lastProductID < 0)
    {
        return FALSE;
    }

    for (i=0; i< s->productCount; i++)
    {
        const MenuItem *item = &s->products[i];

        if (memchr(item, 0, sizeof(MenuItem)) == NULL)
        {
            return FALSE;
        }

        if (item->productID <=0||
            item->productID > s->lastProductID||
            !isValidName(item->productName)||
            !isValidPrice(item->productPrice)||
            item->productType < STARTER || item->productType > DRINK||
            (item->productStatus != PRODUCT_AVAILABLE && item->productStatus != PRODUCT_UNAVAILABLE)
            || item->quantity < 0 ||
            item->quantity > MAX_QUANTITY )
        {
            return FALSE;
        }

        if(fabs(item->productPrice - round(item->productPrice*100.0)/100.0) > 0.000001)
        {
            return FALSE;
        }
        
        for (j=0; i<i; j++)
        {
            if(s->products[j].productID == item->productID)
            {
                return FALSE;
            }
        }
    }

    file = fopen(path, "w");

    if (file == NULL)
    {
        return FALSE;
    }

    if(fprintf(file, "PRODUCT|2|%d|%d\n", s->productCount, s->lastProductID) < 0)
    {
        success = FALSE;
    }

    //in san pham
    for (i=0; i< s->productCount && success; i++)
    {
        const MenuItem *item = &s -> products[i];
        if(fprinf(file, "p|%d|%s|%.2f|%d|%d|%d\n", 
                    item->productID,
                    item->productName,
                    item->productPrice,
                    (int)item -> productType,
                    (int)item -> productStatus,
                    item->quantity) < 0 )
        {
                success = FALSE;
        }
    }
    if (fclose(file) != 0)
    {
        success = FALSE;
    }
    return success;
}
int loadProduct(Product *list, int *count)
{
    Product item;
    char    line[LINE_SIZE];
    FILE   *f;
    int     n, i, isDuplicate;

    *count = 0;

    f = fopen(PRODUCT_FILE, "r");
    if (f == NULL) {
        printf("File not found, start with an empty menu\n");
        return FALSE;
    }

    while (*count < MAX_PRODUCT && fgets(line, sizeof(line), f) != NULL) {
        n = sscanf(line, "%9[^,],%49[^,],%d,%f,%d,%d",
                   item.productID, item.productName, &item.productType,
                   &item.price, &item.status, &item.quantity);

        if (n != 6) continue;
        if (item.productType < 1 || item.productType > 5) continue;
        if (item.price < 0) continue;
        if (item.status != 1 && item.status != 2) continue;
        if (item.quantity < 0) continue;

        isDuplicate = FALSE;
        for (i = 0; i < *count; i++) {
            if (strcmp(list[i].productID, item.productID) == 0) {
                isDuplicate = TRUE;
                break;
            }
        }
        if (isDuplicate) continue;

        list[*count] = item;
        (*count)++;
    }

    fclose(f);
    printf("Loaded %d products\n", *count);
    return TRUE;
}

int saveProduct(const Product *list, int count)
{
    FILE *f = fopen(PRODUCT_FILE, "w");
    int i;

    if (f == NULL) {
        printf("Error: cannot open file to save\n");
        return FALSE;
    }

    for (i = 0; i < count; i++) {
        fprintf(f, "%s,%s,%d,%.2f,%d,%d\n",
                list[i].productID, list[i].productName, list[i].productType,
                list[i].price, list[i].status, list[i].quantity);
    }

    fclose(f);
    return TRUE;
}

int saveTransaction(void) { return TRUE; }  
int saveBillReport(void)  { return TRUE; }  
nt loadTransaction(State *changing, const char *path){
    FILE *file;
    ChargePolicy policy = {0};
    char line[LINE_SIZE];
    int version = 0, tableCount = 0, resCount = 0, orderCount = 0;
    int lastResID = 0, lastOrderID = 0;
    int i, j, type, status, preOrder;
    int success = TRUE;
    time_t timestamp;
// changing va path khong hop le//
    if (changing == NULL || path == NULL) return FALSE;
    file = fopen(path, "r");
    if (file == NULL) return FALSE;
    // đoc du lieu thong qua changing.//
    // TX|2|tableCount|resCount|orderCount|lastResID|lastOrderID
    if (fgets(line, sizeof(line), file) == NULL ||
        sscanf(line, "TX|%d|%d|%d|%d|%d|%d",
               &version, &tableCount, &resCount, &orderCount,
               &lastResID, &lastOrderID) != 6)
        success = FALSE;
        // kiem tra cac gia tri 
    if (success && (version != 2 ||
        tableCount < 0 || tableCount > MAX_TABLES ||
        resCount < 0 || resCount > MAX_RESERVATIONS ||
        orderCount < 0 || orderCount > MAX_ORDERS ||
        lastResID < 0 || lastOrderID < 0))
        success = FALSE;

    // C|dineInRate|takeawayRate|deliveryBase|deliveryPerKm
    if (success && (fgets(line, sizeof(line), file) == NULL ||
        sscanf(line, "C|%lf|%lf|%lf|%lf",
               &policy.dineInRate, &policy.takeawayRate,
               &policy.deliveryBase, &policy.deliveryPerKm) != 4))
        success = FALSE;

    if (success && (!isfinite(policy.dineInRate) ||
        !isfinite(policy.takeawayRate) ||
        !isfinite(policy.deliveryBase) ||
        !isfinite(policy.deliveryPerKm)))
        success = FALSE;
// doc tung ban//
    // T|tableID|tableNumber|tableType|tableTypeID|tableStatus
    for (i = 0; i < tableCount && success; i++) {
        Table *t = &changing->tables[i];
        if (fgets(line, sizeof(line), file) == NULL ||
            sscanf(line, "T|%d|%d|%d|%d|%d",
                   &t->tableID, &t->tableNumber, &t->tableType,
                   &t->tableTypeID, &status) != 5 ||
            t->tableID <= 0 || status < TABLE_AVAILABLE || status > TABLE_SERVING) {
            success = FALSE;
        } else {
            t->tableStatus = (enum TableStatus)status;
        }
    }
// doc reservation//
    // R|id|name|phone|guests|year|month|day|hour|minute|type|tableID|status
    for (i = 0; i < resCount && success; i++) {
        Reservation *r = &changing->reservations[i];
        *r = (Reservation){0};
        if (fgets(line, sizeof(line), file) == NULL ||
            sscanf(line, "R|%d|%49[^|]|%14[^|]|%d|%d|%d|%d|%d|%d|%d|%d|%d",
                   &r->reservationID, r->nameCustomer, r->phoneNumber,
                   &r->NumberCustomer, &r->timeBooking.year,
                   &r->timeBooking.month, &r->timeBooking.day,
                   &r->timeBooking.hour, &r->timeBooking.minute,
                   &r->tableType, &r->tableID, &status) != 12) {
            success = FALSE;
        } else if (r->reservationID <= 0 || r->reservationID > lastResID ||
                   !isValidName(r->nameCustomer) || !isValidPhone(r->phoneNumber) ||
                   !toTimestamp(r->timeBooking, &timestamp) ||
                   r->NumberCustomer <= 0 || r->NumberCustomer > 8 ||
                   status < WAITING || status > CANCELLED) {
            success = FALSE;
        } else {
            r->reservationStatus = (enum ReservationStatus)status;
        }
    }
// doc order table
    // O|id|tableID|reservationID|type|preOrder|guests|distance|status|itemCount
    for (i = 0; i < orderCount && success; i++) {
        Order *o = &changing->orders[i];
        *o = (Order){0};
        if (fgets(line, sizeof(line), file) == NULL ||
            sscanf(line, "O|%d|%d|%d|%d|%d|%d|%lf|%d|%d",
                   &o->orderID, &o->tableID, &o->reservationID,
                   &type, &preOrder, &o->NumberCustomer,
                   &o->deliveryDistance, &status, &o->itemCount) != 9) {
            success = FALSE;
        } else if (o->orderID <= 0 || o->orderID > lastOrderID ||
                   type < DINE_IN || type > DELIVERY ||
                   (preOrder != 0 && preOrder != 1) ||
                   !isfinite(o->deliveryDistance) || o->deliveryDistance < 0 ||
                   status < ORDER_OPEN || status > ORDER_CANCELLED ||
                   o->itemCount < 0 || o->itemCount > MAX_ITEMS) {
            success = FALSE;
        } else {
            o->orderType = (enum OrderType)type;
            o->orderStatus = (enum OrderStatus)status;
            o->isPreOrder = preOrder;
        }
// doc orderItem
        // I|productID|name|type|price|quantity
        for (j = 0; j < o->itemCount && success; j++) {
            OrderItem *item = &o->items[j];
            if (fgets(line, sizeof(line), file) == NULL ||
                sscanf(line, "I|%d|%99[^|]|%d|%lf|%d",
                       &item->productID, item->productName, &type,
                       &item->productPrice, &item->quantity) != 5) {
                success = FALSE;
            } else if (item->productID <= 0 || !isValidName(item->productName) ||
                       type < STARTER || type > DRINK ||
                       !isfinite(item->productPrice) || item->productPrice < 0 ||
                       item->quantity <= 0) {
                success = FALSE;
            } else {
                item->productType = (enum ProductType)type;
            }
        }
    }
    // tat ca dong phai dc doc sai thi dong
    if (success && (fgetc(file) != EOF || ferror(file))) success = FALSE;
    if (fclose(file) != 0) success = FALSE;
// cap nhap thong tin tra ve ket qua
    if (success) {
        changing->policy = policy;
        changing->tableCount = tableCount;
        changing->resCount = resCount;
        changing->orderCount = orderCount;
        changing->lastResID = lastResID;
        changing->lastOrderID = lastOrderID;
    }
    return success;
}
int saveTransaction(const State *s, const char *path){
    FILE *f;
    int i, j;
    if (s == NULL || path == NULL ||
        s->tableCount < 0 || s->tableCount > MAX_TABLES ||
        s->resCount < 0 || s->resCount > MAX_RESERVATIONS ||
        s->orderCount < 0 || s->orderCount > MAX_ORDERS)
        return FALSE;
    for (i = 0; i < s->orderCount; i++) {
        if (s->orders[i].itemCount < 0 ||
            s->orders[i].itemCount > MAX_ITEMS)
            return FALSE;
    }
    f = fopen(path, "w");
    if (f == NULL) {
        printf("Cannot open transaction file!\n");
        return FALSE;
    }
    fprintf(f, "TX|2|%d|%d|%d|%d|%d\n",
            s->tableCount, s->resCount, s->orderCount,
            s->lastResID, s->lastOrderID);
    fprintf(f, "C|%.17g|%.17g|%.17g|%.17g\n",
            s->policy.dineInRate, s->policy.takeawayRate,
            s->policy.deliveryBase, s->policy.deliveryPerKm);

    // luu table//
    for (i = 0; i < s->tableCount; i++) {
        const Table *t = &s->tables[i];

        fprintf(f, "T|%d|%d|%d|%d|%d\n",
                t->tableID, t->tableNumber, t->tableType,
                t->tableTypeID, t->tableStatus);
    }

    // Luu reservation
    for (i = 0; i < s->resCount; i++) {
        const Reservation *r = &s->reservations[i];

        fprintf(f, "R|%d|%s|%s|%d|%d|%d|%d|%d|%d|%d|%d|%d\n",
                r->reservationID, r->nameCustomer,
                r->phoneNumber, r->NumberCustomer,
                r->timeBooking.year, r->timeBooking.month,
                r->timeBooking.day, r->timeBooking.hour,
                r->timeBooking.minute, r->tableType,
                r->tableID, r->reservationStatus);
    }

    // Luu orderID,Item
    for (i = 0; i < s->orderCount; i++) {
        const Order *o = &s->orders[i];
        fprintf(f, "O|%d|%d|%d|%d|%d|%d|%.17g|%d|%d\n",
                o->orderID, o->tableID, o->reservationID,
                o->orderType, o->isPreOrder, o->NumberCustomer,
                o->deliveryDistance, o->orderStatus, o->itemCount);
        for (j = 0; j < o->itemCount; j++) {
            const OrderItem *item = &o->items[j];
            fprintf(f, "I|%d|%s|%d|%.17g|%d\n",
                    item->productID, item->productName,
                    item->productType, item->productPrice,
                    item->quantity);
        }
    }
    int success = !ferror(f);
    if (fclose(f) != 0) return FALSE;
    return success;
}
int loadBillReport(State *changing, const char *path)
{
    FILE *file;
    char line[LINE_SIZE];
    int version = 0, billCount = 0, lastBillID = 0;
    int i, j, type;
    int success = TRUE;
    time_t timestamp;
    if (changing == NULL || path == NULL) return FALSE;
    file = fopen(path, "r");
    if (file == NULL) return FALSE;
    // BILL|2|billCount|lastBillID
    if (fgets(line, sizeof(line), file) == NULL ||
        sscanf(line, "BILL|%d|%d|%d", &version, &billCount, &lastBillID) != 3)
        success = FALSE;
    if (success && (version != 2 || billCount < 0 || billCount > MAX_BILLS ||
                    lastBillID < 0)) success = FALSE;

    for (i = 0; i < billCount && success; i++) {
        Bill *b = &changing->bills[i];
        *b = (Bill){0};
        // B|billID|orderID|tableID|staffID|orderType|year|month|day|hour|minute|
        // itemCount|total|discountRate|discount|taxRate|tax|surcharge|amountDue|
        // dineInRate|takeawayRate|deliveryBase|deliveryPerKm|deliveryDistance
        if (fgets(line, sizeof(line), file) == NULL ||
            sscanf(line,
              "B|%d|%d|%d|%d|%d|%d|%d|%d|%d|%d|%d|%lf|%lf|%lf|%lf|%lf|%lf|%lf|%lf|%lf|%lf|%lf|%lf",
              &b->billID, &b->orderID, &b->tableID, &b->staffID, &type,
              &b->timeBill.year, &b->timeBill.month, &b->timeBill.day,
              &b->timeBill.hour, &b->timeBill.minute, &b->itemCount,
              &b->totalMoney, &b->discountRate, &b->discount,
              &b->taxRate, &b->tax, &b->surcharge, &b->amountDue,
              &b->policySnapshot.dineInRate, &b->policySnapshot.takeawayRate,
              &b->policySnapshot.deliveryBase, &b->policySnapshot.deliveryPerKm,
              &b->deliveryDistance) != 23) {
            success = FALSE;
        } else if (b->billID <= 0 || b->billID > lastBillID ||
                   type < DINE_IN || type > DELIVERY ||
                   !toTimestamp(b->timeBill, &timestamp) ||
                   b->itemCount < 0 || b->itemCount > MAX_ITEMS ||
                   !isfinite(b->totalMoney) || !isfinite(b->discountRate) ||
                   !isfinite(b->discount) || !isfinite(b->taxRate) ||
                   !isfinite(b->tax) || !isfinite(b->surcharge) ||
                   !isfinite(b->amountDue) ||
                   !isfinite(b->policySnapshot.dineInRate) ||
                   !isfinite(b->policySnapshot.takeawayRate) ||
                   !isfinite(b->policySnapshot.deliveryBase) ||
                   !isfinite(b->policySnapshot.deliveryPerKm) ||
                   !isfinite(b->deliveryDistance)) {
            success = FALSE;
        } else {
            b->orderType = (enum OrderType)type;
        }
        for (j = 0; j < b->itemCount && success; j++) {
            OrderItem *item = &b->items[j];
            if (fgets(line, sizeof(line), file) == NULL ||
                sscanf(line, "I|%d|%99[^|]|%d|%lf|%d",
                       &item->productID, item->productName, &type,
                       &item->productPrice, &item->quantity) != 5) {
                success = FALSE;
            } else if (item->productID <= 0 || !isValidName(item->productName) ||
                       type < STARTER || type > DRINK ||
                       !isfinite(item->productPrice) || item->productPrice < 0 ||
                       item->quantity <= 0) {
                success = FALSE;
            } else {
                item->productType = (enum ProductType)type;
            }
        }
    }

    if (success && (fgetc(file) != EOF || ferror(file))) success = FALSE;
    if (fclose(file) != 0) success = FALSE;

    if (success) {
        changing->billCount = billCount;
        changing->lastBillID = lastBillID;
    }
    return success;
}
int saveBillReport(const State *s, const char *path)
{
    FILE *file;
    int i, j, success = TRUE;
    time_t timestamp;

    if (s == NULL || path == NULL ||
        s->billCount < 0 || s->billCount > MAX_BILLS ||
        s->lastBillID < 0) return FALSE;

    for (i = 0; i < s->billCount; i++) {
        const Bill *b = &s->bills[i];
        if (b->billID <= 0 || b->billID > s->lastBillID ||
            !toTimestamp(b->timeBill, &timestamp) ||
            b->itemCount < 0 || b->itemCount > MAX_ITEMS) return FALSE;
    }

    file = fopen(path, "w");
    if (file == NULL) return FALSE;

    if (fprintf(file, "BILL|2|%d|%d\n", s->billCount, s->lastBillID) < 0)
        success = FALSE;

    for (i = 0; i < s->billCount && success; i++) {
        const Bill *b = &s->bills[i];
        if (fprintf(file,
              "B|%d|%d|%d|%d|%d|%d|%d|%d|%d|%d|%d|%.17g|%.17g|%.17g|%.17g|%.17g|%.17g|%.17g|%.17g|%.17g|%.17g|%.17g|%.17g\n",
              b->billID, b->orderID, b->tableID, b->staffID,
              (int)b->orderType, b->timeBill.year, b->timeBill.month,
              b->timeBill.day, b->timeBill.hour, b->timeBill.minute,
              b->itemCount, b->totalMoney, b->discountRate, b->discount,
              b->taxRate, b->tax, b->surcharge, b->amountDue,
              b->policySnapshot.dineInRate, b->policySnapshot.takeawayRate,
              b->policySnapshot.deliveryBase, b->policySnapshot.deliveryPerKm,
              b->deliveryDistance) < 0) success = FALSE;
        for (j = 0; j < b->itemCount && success; j++) {
            const OrderItem *item = &b->items[j];
            if (fprintf(file, "I|%d|%s|%d|%.17g|%d\n",
                        item->productID, item->productName,
                        (int)item->productType, item->productPrice,
                        item->quantity) < 0) success = FALSE;
        }
    }
    if (fclose(file) != 0) success = FALSE;
    return success;
}