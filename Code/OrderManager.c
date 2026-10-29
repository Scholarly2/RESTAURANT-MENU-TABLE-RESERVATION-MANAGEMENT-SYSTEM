#include <stdio.h>
#include <RestaurantTypes.h>
#include <stdlib.h>
#include <math.h>

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

                printf("Enter table ID: ");
                if(scanf("%d", &tableID) != 1 || tableID < 1 || tableID > 100){
                    printf("Table ID is invalid.\n");
                    return FALSE;
                }

                printf("Enter number of guests: ");
                if(scanf("%d", &guests) != 1 || guests < 1 || guests > 8){
                    printf("Number of guests is invalid.\n");
                    return FALSE;
                }

                int tableIndex = findTableByID(&S, tableID);

                if(tableIndex == -1){
                    printf("Table is invalid.\n");
                    return FALSE;
                }

                if(S.tables[tableIndex].tableStatus == TABLE_SERVING){

                    int openOrderIndex = findOpenOrder(&S, tableID, 0);

                    if(openOrderIndex == -1 || S.orders[openOrderIndex].isPreOrder){
                        printf("Cannot use existing order.\n");
                        return FALSE;
                    }

                    printf("Use existing order %d.\n",
                           S.orders[openOrderIndex].orderID);

                    return S.orders[openOrderIndex].orderID;
                }

                if(S.tables[tableIndex].tableStatus != TABLE_AVAILABLE){
                    printf("Table is not available.\n");
                    return FALSE;
                }

                if(guests > S.tables[tableIndex].tableType){
                    printf("Number of guests exceeds table capacity.\n");
                    return FALSE;
                }
            }

            distance = 0;
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
        if(type == DINE_IN && reservationID == 0){

            if(!updateStatusAutoTable(c, tableID, CHECK_IN)){
                free(c);
                printf("Cannot update table status.\n");
                return FALSE;
            }
        }

        // luu thay doi
        int newOrderID = c->orders[orderIndex].orderID;
        printf("Create order successfully. Order ID: %d\n", newOrderID);
    }
    return TRUE;
}

int findOrderByID(const State*s, int id){
    //tim kiem bang ID
    for(int i = 0; i < s->orderCount; i++){
        if(s->orders[i].orderID  == id)
            return i;
    }
    return -1;
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

    // luu thay doi
    char oldValue[20];
    char newValue[20];

    sprintf(oldValue, "%d", oldQuantity);
    sprintf(newValue, "%d", newQuantity);

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
    showAllOrderItem(&S.orders[findOrderIndexByID]);
    // bat dau tim kiem san pham trong order do
    printf("Enter ID of the product: ");
    scanf("%d", &productID);
    int findOrderItemIndex = findOrderItem(&S.orders[findOrderIndexByID], productID);
    if(findOrderItemIndex == -1){
        return FALSE;
    }

    // kiem tra so luong moi co hop le khong
    if (newQuantity < 1 || newQuantity > MAX_QUANTITY){
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
        // chuyen so luong thanh chuoi
    char oldValue[20];
    char newValue[20];

    sprintf(oldValue, "%d", oldQuantity);
    sprintf(newValue, "%d", newQuantity);

    // luu thay doi
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
    showAllOrderItem(&S.orders[findOrderIndexByID]);
    
    // tim product bang id cua order
    printf("Enter ID of the product: ");
    scanf("%d", &productID);
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
        printf("Invaild choose!");
        return FALSE;
    }
    if(choice == 2){
        printf("Delete cancelled!");
        return FALSE;
    }
    if(choice == -1 ){
        printf("Invaild choose!");
        return FALSE;
    }

    if(choice == 1){
        if(!changeOrderQuantity(copy,findOrderIndexByID, productID, 0)){
            free(copy);
            return FALSE;
        }
    }

    //Luu thay doi
    char oldValuse[20];
    sprint(oldValuse, "%d", productID);
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
            retuern i;
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

void showAllOrderFood(const Order *order){
    //hien thi tat ca cac mon trong order;
    for(int i = 0; i < order->itemCount; i++){
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
    int findOrderIndexByID = findOrderByID(&S, orderID);
    if(findOrderIndexByID == -1 || S.orders[findOrderIndexByID].orderStatus != ORDER_OPEN || findOrderIndexByID >= S.orderCount){
        return FALSE;
    }

    // cancel order va giai phong ton kho
    if(!releaseOrderStock(&S, findOrderIndexByID)){
        return FALSE;
    }

    return TRUE;
}

int checkoutOrder(int orderID, double discountRate, double taxRate){

    //tim kiem order bang ID
    int findOrderIndexByID = findOrderByID(&S, orderID);
    if(findOrderIndexByID == -1 || S.orders[findOrderIndexByID].orderStatus != ORDER_OPEN || findOrderIndexByID >= S.orderCount){
        return FALSE;
    }

    //kiem tra discount va tax co hop le khong
    if(!isfinite(discountRate) || !isfinite(taxRate) || discountRate < 0 || discountRate > 100 || taxRate < 0 || taxRate > 100){
        return FALSE;
    }

    //tinh hoa don
    Bill bill;
    if(!calculateBill(&S.orders[findOrderIndexByID], S.policy, discountRate, taxRate, &bill)){
        return FALSE;
    }

    //luu hoa don vao bo nho
    if(S.billCount >= MAX_BILLS){
        return FALSE;
    }
    S.bills[S.billCount] = bill;
    S.billCount++;
    // cap nhat trang thai order thanh ORDER_PAID
    S.orders[findOrderIndexByID].orderStatus = ORDER_PAID;
    return TRUE;
}

