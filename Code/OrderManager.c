#include <stdio.h>
#include "../Include_type_struct/RestaurantTypes.h"
#include <stdlib.h>
#include <math.h>
#include "../Include_type_struct/Input.h"
#include "../Include_type_struct/menu.h"
#include "../Include_type_struct/Login.h"
#include "../Include_type_struct/FileHelper.h"

int appendEmptyOrder(State *s, int type, int reservationID,
                     int tableID, int guests, double distance)
{
    Order newOrder = {0};

    if (s == NULL) return -1;
    if (s->orderCount >= MAX_ORDERS) return -1;

    newOrder.orderID = ++s->lastOrderID;
    newOrder.orderType = type;
    newOrder.reservationID = reservationID;
    newOrder.tableID = tableID;
    newOrder.NumberCustomer = guests;
    newOrder.deliveryDistance = distance;
    newOrder.orderStatus = ORDER_OPEN;
    newOrder.itemCount = 0;

    s->orders[s->orderCount] = newOrder;

    return s->orderCount++;
}

int createOrderTable(int type, int reservationID, int tableID, int guests, double distance){

    // so luong order muon tao
    int n;
    printf("Enter amount of order: ");
    if(scanf("%d", &n) != 1 || n <= 0){
        printf("Amount of order is invalid.\n");
        return FALSE;
    }

    // tao order moi
    for(int i = 0; i < n; i++){

        printf("Enter customer type (DINE_IN / TAKEAWAY / DELIVERY): ");
        if(scanf("%d", &type) != 1 || type < DINE_IN || type > DELIVERY){
            printf("Customer type is invalid.\n");
            return FALSE;
        }

        // DINE IN
        if(type == DINE_IN){

            printf("Enter reservation ID (0 if no reservation): ");
            if(scanf("%d", &reservationID) != 1 || reservationID < 0){
                printf("Reservation ID is invalid.\n");
                return FALSE;
            }

            if(reservationID > 0){

                int reservationIndex = findReservationByID(&S, reservationID);

                if(reservationIndex == -1 ||
                   S.reservations[reservationIndex].reservationStatus != WAITING){
                    printf("Reservation is invalid.\n");
                    return FALSE;
                }

                tableID = S.reservations[reservationIndex].tableID;
                guests = S.reservations[reservationIndex].NumberCustomer;

                if(findOpenOrder(&S, 0, reservationID) != -1){
                    printf("There is already an open order for this reservation.\n");
                    return FALSE;
                }
            }
            else{

                printf("Enter number of guests: ");
                if(scanf("%d", &guests) != 1 || guests < 1 || guests > 8){
                    printf("Number of guests is invalid.\n");
                    return FALSE;
                }

                printf("Enter table ID: ");
                if(scanf("%d", &tableID) != 1){
                    printf("Table ID is invalid.\n");
                    return FALSE;
                }

                // Check table ID based on number of guests
                if(guests == 1 && (tableID < 1 || tableID > 20)){
                    printf("Please choose a table for 2 guests (ID: 1 - 20).\n");
                    return FALSE;
                }
                else if(guests >= 2 && guests < 4 && (tableID < 21 || tableID > 60)){
                    printf("Please choose a table for 4 guests (ID: 21 - 60).\n");
                    return FALSE;
                }
                else if(guests >= 4 && (tableID < 61 || tableID > 100)){
                    printf("Please choose a table for 8 guests (ID: 61 - 100).\n");
                    return FALSE;
                }

                int tableIndex = findTableByID(&S, tableID);

                if(tableIndex == -1){
                    printf("Table is invalid.\n");
                    return FALSE;
                }
                if (S.tables[tableIndex].tableStatus != TABLE_AVAILABLE) {
                    printf("Table is not available.\n");
                    return FALSE;
                }
            }
        }

        // TAKEAWAY / DELIVERY
        else{

            reservationID = 0;
            tableID = 0;
            guests = 0;

            printf("Enter delivery distance: ");
            if(scanf("%lf", &distance) != 1 ||
               !isfinite(distance) ||
               distance < 0 ||
               distance > MAX_DISTANCE){
                printf("Distance is invalid.\n");
                return FALSE;
            }

            if(type == TAKEAWAY && distance != 0){
                printf("Takeaway distance must be 0.\n");
                return FALSE;
            }
        }

        // copy state
        State *c = copyState(&S);

        if(c == NULL){
            printf("Cannot copy state.\n");
            return FALSE;
        }

        // tao order trong state copy
        int orderIndex = appendEmptyOrder(
            c, type, reservationID, tableID, guests, distance
        );

        if(orderIndex == -1){
            free(c);
            printf("Cannot create order.\n");
            return FALSE;
        }

        // cap nhat trang thai ban
        if (type == DINE_IN) {

            if (reservationID > 0) {
                int r = findReservationByID(c, reservationID);

                if (r == -1) {
                    free(c);
                    return FALSE;
                }

                c->reservations[r].reservationStatus = COMPLETED;
            }

            if (!updateStatusAutoTable(c, tableID, CHECK_IN)) {
                free(c);
                printf("Cannot update table status.\n");
                return FALSE;
            }
        }

        // luu thay doi vao OrderHistory.txt
        int newOrderID = c->orders[orderIndex].orderID;

        if (!saveTransaction(c, ORDER_HISTORY_FILE)) {
            free(c);
            printf("Cannot save order history.\n");
            return FALSE;
        }

        if (!saveTableStatus(c, TABLE_STATUS_FILE)) {
            free(c);
            printf("Cannot save table status.\n");
            return FALSE;
        }

        S = *c;
        free(c);

        printf("Create order successfully. Order ID: %d\n", newOrderID);
    }
    return TRUE;
}

int addOrderFood(int orderID, int productID, int quantity){

    // di tim order bang ID
    printf("Enter the order you want to find: ");
    scanf("%d", &orderID);
    int orderIndex = findOrderByID(&S, orderID);   

    if(orderIndex == -1 || orderIndex >= S.orderCount || S.orders[orderIndex].orderStatus != ORDER_OPEN){
        printf("Order is invalid.\n");
        return FALSE;
    }

    // in danh sach mon an
    printListProduct(S.products, S.productCount);

    printf("Enter the product ID: ");
    if(scanf("%d", &productID) != 1 || productID <= 0){
        printf("The product ID is invalid.\n");
        return FALSE;
    }

    // kiem tra mon an
    int productIndex = findProductByID(&S, productID);

    if(productIndex == -1 || productIndex >= S.productCount || S.products[productIndex].productStatus == PRODUCT_UNAVAILABLE){
        printf("The product does not exist or is unavailable.\n");
        return FALSE;
    }

    // nhap so luong mon an muon them vao
    printf("Enter the quantity: ");
    if(scanf("%d", &quantity) != 1 ||
       quantity < 1 || quantity > MAX_QUANTITY){
        printf("Quantity is invalid.\n");
        return FALSE;
    }

    // tim mon an trong order
    int itemIndex = findOrderItem(&S.orders[orderIndex], productID);

    int oldQuantity = 0;

    if(itemIndex != -1){
        oldQuantity = S.orders[orderIndex].items[itemIndex].quantity;
    }

    // kiem tra tong so luong
    if(quantity > MAX_QUANTITY - oldQuantity){
        printf("Total quantity exceeds the maximum limit.\n");
        return FALSE;
    }

    int newQuantity = oldQuantity + quantity;

    // copy state
    State *copy = copyState(&S);

    if(copy == NULL){
        printf("Cannot copy state.\n");
        return FALSE;
    }

    // them mon an vao order va cap nhat ton kho
    if(!changeOrderQuantity(copy, orderIndex, productID, newQuantity)){
        free(copy);
        printf("Cannot add food to order.\n");
        return FALSE;
    }

    // luu thay doi vao OrderHistory.txt
    if (!saveTransaction(copy, "Data/OrderHistory.txt")) {
        free(copy);
        printf("Cannot save order history.\n");
        return FALSE;
    }

    // cap nhat State hien tai
    S = *copy;

    free(copy);

    printf("Add order food successfully.\n");
    return TRUE;
}
int updateOrderFood(int orderID, int productID, int newQuantity){

    //tim kiem order bang ID
    printf("Enter the order you want to find: ");
    scanf("%d", &orderID);

    int findOrderIndexByID = findOrderByID(&S, orderID);
    if(findOrderIndexByID == -1 || S.orders[findOrderIndexByID].orderStatus != ORDER_OPEN || findOrderIndexByID >= S.orderCount){
        return FALSE;
    }
    showAllOrderFood(&S.orders[findOrderIndexByID]);
    // bat dau tim kiem san pham trong order do
    printf("Enter ID of the product: ");
    scanf("%d", &productID);
    int findOrderItemIndex = findOrderItem(&S.orders[findOrderIndexByID], productID);
    if(findOrderItemIndex == -1){
        return FALSE;
    }

    // nhap so luong moi
    printf("Enter new quantity: ");

    if (scanf("%d", &newQuantity) != 1 ||
        newQuantity < 1 || newQuantity > MAX_QUANTITY) {
        printf("Quantity is invalid.\n");
        return FALSE;
    }

    // lay so luong cu
    int oldQuantity = S.orders[findOrderIndexByID].items[findOrderItemIndex].quantity;
    State *copy = copyState(&S);
    if(copy == NULL){
        return FALSE;
    }
    if(!changeOrderQuantity(copy, findOrderIndexByID, productID, newQuantity)){
        free(copy);
        return FALSE;
    }
    // luu thay doi vao OrderHistory.txt
    if (!saveTransaction(copy, "Data/OrderHistory.txt")) {
        free(copy);
        printf("Cannot save order history.\n");
        return FALSE;
    }

    // cap nhat State hien tai
    S = *copy;

    free(copy);

    printf("Update order food successfully.\n");
    return TRUE;
}
int deleteOrderFood(int orderID, int productID){

    //Kiem order bang ID
    printf("Enter the order you want to find: ");
    scanf("%d", &orderID);

    int findOrderIndexByID = findOrderByID(&S, orderID);
    if(findOrderIndexByID == -1 || S.orders[findOrderIndexByID].orderStatus != ORDER_OPEN || findOrderIndexByID >= S.orderCount){
        return FALSE;
    }
    showAllOrderFood(&S.orders[findOrderIndexByID]);
    
    // tim product bang id cua order
    printf("Enter ID of the product: ");
    scanf("%s", &productID);
    int findOrderItemIndex = findOrderItem(&S.orders[findOrderIndexByID], productID);
    if(findOrderItemIndex == -1){
        return FALSE;
    }

    //Nhap de xoa
    State *copy = copyState(&S);
    if(copy == NULL){
        return FALSE;
    }

    int choice;
    printf("Product: %d ", S.orders[findOrderIndexByID].items[findOrderItemIndex].productName);
    printf("1.Delete\n2.Cancel\nYour choice: ");
    if(scanf("%d", &choice) != 1){
        free(copy);
        printf("Invalid choice!");
        return FALSE;
    }

    if(choice == 2){
        free(copy);
        printf("Delete cancelled!");
        return FALSE;
    }

    if(choice != 1){
        free(copy);
        printf("Invalid choice!");
        return FALSE;
    }

    if(choice == 1){
        if(!changeOrderQuantity(copy,findOrderIndexByID, productID, 0)){
            free(copy);
            return FALSE;
        }
    }

    //luu vao file
    if (!saveTransaction(copy, "Data/OrderHistory.txt")) {
        free(copy);
        printf("Cannot save order history.\n");
        return FALSE;
    }

    S = *copy;
    free(copy);

    printf("Delete order food successfully.\n");
    return TRUE;
}

int changeOrderQuantity(State *s, int orderIndex, int productID, int newQuantity){
    //check dieu kien vao
    if(s == NULL || orderIndex < 0 || orderIndex >= s->orderCount){
        return FALSE;
    }

    //kiem tra Quantity mooi
    if(newQuantity < 0 || newQuantity > MAX_QUANTITY){
        return FALSE;
    }

    Order *order = &s->orders[orderIndex];

    int itemIndex = findOrderItem(order, productID);

    if(itemIndex == -1){
        return FALSE;
    }

    //thay doi Quantity
    order->items[itemIndex].quantity = newQuantity;

    return TRUE;
}

void listPreOrderedDishes(int tableID){

    showAllOrderTable(); 
    //nhap de tim kiem table theo ID
    printf("Enter ID of the table: ");
    scanf("%d", &tableID);

    int findTable = findTableByID(&S, tableID);
    if(findTable == -1){
        printf("Table not found!");
        return;
    }
    
    // Duyet qua tat ca cac mon
    int found = FALSE;
    for(int i = 0; i < S.orderCount; i++){
        Order *order = &S.orders[i];
        if( order->tableID != tableID){
            continue;
        }
        if( order->orderStatus != ORDER_OPEN || !order->isPreOrder){
            continue;
        }
        int findReservationID = findReservationByID(&S, order->reservationID);
        if(findReservationID != -1 && S.reservations[findReservationID].reservationStatus == WAITING && order->itemCount > 0){
            printf("OrderID: %d\n", order->orderID);
            printf("TableID: %d\n", order->tableID);
            printf("Time Booking: %d/%d/%d %d:%d\n",
                   S.reservations[findReservationID].timeBooking.day,
                   S.reservations[findReservationID].timeBooking.month,
                   S.reservations[findReservationID].timeBooking.year,
                   S.reservations[findReservationID].timeBooking.hour,
                   S.reservations[findReservationID].timeBooking.minute);

        showAllOrderFood(order);
        found = TRUE;
    }
        if(found == FALSE){
            printf("No pre ordered this dishes for this table");
            }
        }
}

int findOrderByID(const State *s, int id){
    //tim kiem bang ID

    if (s == NULL) return -1;

    for(int i = 0; i < S.orderCount; i++){
        if(S.orders[i].orderID == id){
            return i;
        }
        else{
            return FALSE;
        }
    }
}

int findOrderItem(const Order *order, int productID){
    //tim kiem bang ID trong order
    for(int i = 0; i , order->itemCount; i++){
        if(order->items[i].productID == productID){
            return i;
        }
        else{
            return FALSE;
        }
    }
}

int findOpenOrder(const State *s, int tableID, int reservationID){
    //tim order bang tableID va reservationID
    for(int i = 0; i < S.orderCount; i++){
        if(S.orders[i].tableID == tableID && S.orders[i].reservationID == reservationID && S.orders[i].orderStatus == ORDER_OPEN){
            return i;
        }
        else{
            return FALSE;
        }
	}
}

void showAllOrderFood(const Order *order){
    //hien thi tat ca cac mon trong order;
    for(int i = 0; i < order->itemCount; i++){
        if (order->items[i].quantity == 0) {
        continue;
    }
        printf("ProductID: %d\n", order->items[i].productID);
        printf("ProductName: %s\n", order->items[i].productName);
        printf("ProductType: %d\n", order->items[i].productType);
        printf("ProductPrice: %.2f\n", order->items[i].productPrice);
        printf("Quantity: %d\n", order->items[i].quantity);
    }
}

void showAllOrderTable(void){
    // hien thi tat ca cac order cua ban
    for(int i = 0; i < S.orderCount; i++){
        printf("OrderID: %d\n", S.orders[i].orderID);
        printf("TableID: %d\n", S.orders[i].tableID);
        printf("ReservationID: %d\n", S.orders[i].reservationID);
        printf("OrderType: %d\n", S.orders[i].orderType);
        printf("IsPreOrder: %d\n", S.orders[i].isPreOrder);
        printf("NumberCustomer: %d\n", S.orders[i].NumberCustomer);
        printf("DeliveryDistance: %.2f\n", S.orders[i].deliveryDistance);
        printf("OrderStatus: %d\n", S.orders[i].orderStatus);
        printf("ItemCount: %d\n", S.orders[i].itemCount);
    }
}

int releaseOrderStock(State *s, int orderIndex){
    // kiem tra order co hop le khong
    if(orderIndex < 0 || orderIndex >= s->orderCount){
        return FALSE;
    }

    Order *order = &s->orders[orderIndex];

    // kiem tra order status co phai la ORDER_OPEN khong
    if(order->orderStatus != ORDER_OPEN){
        return FALSE;
    }

    // giai phong ton kho cho moi mon trong order
    for(int i = 0; i < order->itemCount; i++){
        
        OrderItem *item = &order->items[i];

        int productIndex = findProductByID(s, item->productID);
        
        if(productIndex == -1){
            continue;
        }
        s->products[productIndex].quantity += item->quantity;
    }

    return TRUE;
}

int cancelOrder(int orderID){
    // tim order bang ID
    printf("Enter order ID to cancel: ");
    if(scanf("%d", &orderID) != 1){
        printf("Invalid order ID.\n");
        return FALSE;
    }

    int orderIndex = findOrderByID(&S, orderID);

    if(orderIndex == -1){
        printf("Order not found.\n");
        return FALSE;
    }

    if(S.orders[orderIndex].orderStatus != ORDER_OPEN){
        printf("This order cannot be cancelled.\n");
        return FALSE;
    }

    State *copy = copyState(&S);

    if(copy == NULL){
        return FALSE;
    }

    if(!releaseOrderStock(copy, orderIndex)){
        free(copy);
        printf("Cannot release order stock.\n");
        return FALSE;
    }

    copy->orders[orderIndex].orderStatus = ORDER_CANCELLED;

    //chinh status table cua file OrderHistoryTable
    if(copy->orders[orderIndex].orderType == DINE_IN &&
    copy->orders[orderIndex].tableID > 0){

        if(!updateStatusAutoTable(copy,
                                copy->orders[orderIndex].tableID,
                                CANCEL_ORDER)){
            free(copy);
            printf("Cannot update table status.\n");
            return FALSE;
        }
    }

    if(!saveTransaction(copy, "Data/OrderHistory.txt")){
        free(copy);
        printf("Cannot save order history.\n");
        return FALSE;
    }

    if(!saveTableStatus(copy, TABLE_STATUS_FILE)){
        free(copy);
        printf("Cannot save table status.\n");
        return FALSE;
    }

    S = *copy;
    free(copy);

    printf("Cancel order successfully.\n");
    return TRUE;
}


void OrderMenu(int role)
{
    int choice;

    // Variables for order functions
    int type = 0;
    int reservationID = 0;
    int tableID = 0;
    int guests = 0;
    double distance = 0;

    int orderID = 0;
    int productID = 0;
    int quantity = 0;
    int newQuantity = 0;

    // Variables for checkout
    double discountRate = 0;
    double taxRate = 0;

    // Variables for charge policy
    int chargeChoice;
    double rate, baseFee, perKm;

    if (S.orderCount < 0 || S.orderCount > MAX_ORDERS ||
        S.billCount < 0 || S.billCount > MAX_BILLS) {
        puts("Invalid order or bill data. Load valid data first.");
        return;
    }

    // MANAGER MENU
    if (role == ROLE_MANAGER)
    {
        do {
            printf("\n=== ORDER AND BILL MANAGEMENT ===\n");
            printf("1. Create order\n");
            printf("2. Add food to order\n");
            printf("3. Update order food\n");
            printf("4. Delete food from order\n");
            printf("5. View order details\n");
            printf("6. View pre-ordered dishes\n");
            printf("7. Checkout and print bill\n");
            printf("8. Cancel order\n");
            printf("9. Configure charge policy\n");
            printf("0. Go back\n");

            if (!readInt("Choose: ", 0, 9, &choice)) {
                return;
            }

            switch (choice) {

                case 1:
                    createOrderTable(type, reservationID, tableID,
                                     guests, distance);
                    break;

                case 2:
                    addOrderFood(orderID, productID, quantity);
                    break;

                case 3:
                    updateOrderFood(orderID, productID, newQuantity);
                    break;

                case 4:
                    deleteOrderFood(orderID, productID);
                    break;

                case 5:
                    showAllOrderTable();
                    break;

                case 6:
                    listPreOrderedDishes(tableID);
                    break;

                case 7:
                    checkoutOrder(orderID, discountRate, taxRate);
                    break;

                case 8:
                    cancelOrder(orderID);
                    break;

                case 9:
                    do {
                        printf("\n=== CONFIGURE CHARGE POLICY ===\n");
                        printf("1. Configure Dine-in Charge\n");
                        printf("2. Configure Takeaway Charge\n");
                        printf("3. Configure Delivery Charge\n");
                        printf("0. Go back\n");

                        if (!readInt("Choose: ", 0, 3, &chargeChoice)) {
                            return;
                        }

                        switch (chargeChoice) {

                            case 1:
                                printf("\n=== DINE-IN CHARGE ===\n");
                                printf("Current rate: %.2f%%\n",
                                       S.policy.dineInRate);

                                printf("Enter new dine-in rate (0-100): ");

                                if (scanf("%lf", &rate) != 1 ||
                                    !isfinite(rate) ||
                                    rate < 0 || rate > 100) {
                                    printf("Invalid rate!\n");
                                    while (getchar() != '\n');
                                    break;
                                }

                                S.policy.dineInRate = rate;
                                printf("Dine-in charge updated successfully!\n");
                                break;

                            case 2:
                                printf("\n=== TAKEAWAY CHARGE ===\n");
                                printf("Current rate: %.2f%%\n",
                                       S.policy.takeawayRate);

                                printf("Enter new takeaway rate (0-100): ");

                                if (scanf("%lf", &rate) != 1 ||
                                    !isfinite(rate) ||
                                    rate < 0 || rate > 100) {
                                    printf("Invalid rate!\n");
                                    while (getchar() != '\n');
                                    break;
                                }

                                S.policy.takeawayRate = rate;
                                printf("Takeaway charge updated successfully!\n");
                                break;

                            case 3:
                                printf("\n=== DELIVERY CHARGE ===\n");
                                printf("Current base fee: %.2f\n",
                                       S.policy.deliveryBase);
                                printf("Current fee per km: %.2f\n",
                                       S.policy.deliveryPerKm);

                                printf("Enter new delivery base fee: ");

                                if (scanf("%lf", &baseFee) != 1 ||
                                    !isfinite(baseFee) ||
                                    baseFee < 0) {
                                    printf("Invalid base fee!\n");
                                    while (getchar() != '\n');
                                    break;
                                }

                                printf("Enter new delivery fee per km: ");

                                if (scanf("%lf", &perKm) != 1 ||
                                    !isfinite(perKm) ||
                                    perKm < 0) {
                                    printf("Invalid fee per km!\n");
                                    while (getchar() != '\n');
                                    break;
                                }

                                S.policy.deliveryBase = baseFee;
                                S.policy.deliveryPerKm = perKm;

                                printf("Delivery charge updated successfully!\n");
                                break;

                            case 0:
                                break;
                        }

                        if (feof(stdin) || ferror(stdin)) {
                            return;
                        }

                    } while (chargeChoice != 0);

                    break;

                case 0:
                    break;
            }

            if (feof(stdin) || ferror(stdin)) {
                return;
            }

        } while (choice != 0);
    }

    // STAFF MENU
    else if (role == ROLE_STAFF)
    {
        do {
            printf("\n=== ORDER AND BILL MANAGEMENT ===\n");
            printf("1. Search orders\n");
            printf("2. show orders\n");
            printf("3. print bills\n");
            printf("0. Go back\n");

            if (!readInt("Choose: ", 0, 4, &choice)) {
                return;
            }

            switch (choice) {

                case 1:
                    findOrderByID(&S, orderID);
                    break;

                case 2:
                    showAllOrderTable();
                    break;

                case 3:
                    printBillReport(orderID,discountRate,taxRate);
                    break;

                case 0:
                    break;
            }

            if (feof(stdin) || ferror(stdin)) {
                return;
            }

        } while (choice != 0);
    }
}
