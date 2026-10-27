#include <stdio.h>
#include <RestaurantTypes.h>
#include <stdlib.h>

int createOrderTable(int type, int reservationID, int tableID, int guests, double distance){

    // so luong order muon tao
    int n;
    printf("Enter amount of order: ");
    if(scanf("%d", &n) != 1 || n <= 0){
        printf("Amount of order is invalid.\n");
        return -1;
    }

    // tao order moi
    for(int i = 0; i < n; i++){

        printf("Enter customer type (DINE_IN / TAKEAWAY / DELIVERY): ");
        if(scanf("%d", &type) != 1 || type < DINE_IN || type > DELIVERY){
            printf("Customer type is invalid.\n");
            return -1;
        }

        // DINE IN
        if(type == DINE_IN){

            printf("Enter reservation ID (0 if no reservation): ");
            if(scanf("%d", &reservationID) != 1 || reservationID < 0){
                printf("Reservation ID is invalid.\n");
                return -1;
            }

            if(reservationID > 0){

                int reservationIndex = findReservationByID(&S, reservationID);

                if(reservationIndex == -1 ||
                   S.reservations[reservationIndex].reservationStatus != WAITING){
                    printf("Reservation is invalid.\n");
                    return -1;
                }

                tableID = S.reservations[reservationIndex].tableID;
                guests = S.reservations[reservationIndex].NumberCustomer;

                if(findOpenOrder(&S, 0, reservationID) != -1){
                    printf("There is already an open order for this reservation.\n");
                    return -1;
                }
            }
            else{

                printf("Enter table ID: ");
                if(scanf("%d", &tableID) != 1 || tableID < 1 || tableID > 100){
                    printf("Table ID is invalid.\n");
                    return -1;
                }

                printf("Enter number of guests: ");
                if(scanf("%d", &guests) != 1 || guests < 1 || guests > 8){
                    printf("Number of guests is invalid.\n");
                    return -1;
                }

                int tableIndex = findTableByID(&S, tableID);

                if(tableIndex == -1){
                    printf("Table is invalid.\n");
                    return -1;
                }

                if(S.tables[tableIndex].tableStatus == TABLE_SERVING){

                    int openOrderIndex = findOpenOrder(&S, tableID, 0);

                    if(openOrderIndex == -1 || S.orders[openOrderIndex].isPreOrder){
                        printf("Cannot use existing order.\n");
                        return -1;
                    }

                    printf("Use existing order %d.\n",
                           S.orders[openOrderIndex].orderID);

                    return S.orders[openOrderIndex].orderID;
                }

                if(S.tables[tableIndex].tableStatus != TABLE_AVAILABLE){
                    printf("Table is not available.\n");
                    return -1;
                }

                if(guests > S.tables[tableIndex].tableType){
                    printf("Number of guests exceeds table capacity.\n");
                    return -1;
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
                return -1;
            }

            if(type == TAKEAWAY && distance != 0){
                printf("Takeaway distance must be 0.\n");
                return -1;
            }
        }

        // copy state
        State *c = copyState(&S);

        if(c == NULL){
            printf("Cannot copy state.\n");
            return -1;
        }

        // tao order trong state copy
        int orderIndex = appendEmptyOrder(
            c, type, reservationID, tableID, guests, distance
        );

        if(orderIndex == -1){
            free(c);
            printf("Cannot create order.\n");
            return -1;
        }

        // cap nhat trang thai ban
        if(type == DINE_IN && reservationID == 0){

            if(!updateStatusAutoTable(c, tableID, CHECK_IN)){
                free(c);
                printf("Cannot update table status.\n");
                return -1;
            }
        }

        // luu thay doi
        int newOrderID = c->orders[orderIndex].orderID;

        if(!commitChange(c, "CREATE_ORDER", newOrderID, "-", "OPEN")){
            printf("Cannot save order.\n");
            return -1;
        }

        printf("Create order successfully. Order ID: %d\n", newOrderID);
    }
    return 0;
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
    int orderIndex = findOrderByID(&S, orderID);

    if(orderIndex == -1 || orderIndex >= S.orderCount || S.orders[orderIndex].orderStatus != ORDER_OPEN){
        printf("Order is invalid.\n");
        return -1;
    }

    // in danh sach mon an
    printListProduct(S.products, S.productCount);

    printf("Enter the product ID: ");
    if(scanf("%d", &productID) != 1 || productID <= 0){
        printf("The product ID is invalid.\n");
        return -1;
    }

    // kiem tra mon an
    int productIndex = findProductByID(&S, productID);

    if(productIndex == -1 || productIndex >= S.productCount || S.products[productIndex].productStatus == PRODUCT_UNAVAILABLE){
        printf("The product does not exist or is unavailable.\n");
        return -1;
    }

    // nhap so luong mon an muon them vao
    printf("Enter the quantity: ");
    if(scanf("%d", &quantity) != 1 ||
       quantity < 1 || quantity > MAX_QUANTITY){
        printf("Quantity is invalid.\n");
        return -1;
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
        return -1;
    }

    int newQuantity = oldQuantity + quantity;

    // copy state
    State *copy = copyState(&S);

    if(copy == NULL){
        printf("Cannot copy state.\n");
        return -1;
    }

    // them mon an vao order va cap nhat ton kho
    if(!changeOrderQuantity(copy, orderIndex, productID, newQuantity)){
        free(copy);
        printf("Cannot add food to order.\n");
        return -1;
    }

    // luu thay doi
    char oldValue[20];
    char newValue[20];

    sprintf(oldValue, "%d", oldQuantity);
    sprintf(newValue, "%d", newQuantity);

    if(!commitChange(copy, "ADD_FOOD", orderID, oldValue, newValue)){
        printf("Cannot save order food.\n");
        return -1;
    }

    printf("Add order food successfully.\n");
    return 1;
}
int updateOrderFood(int orderID, int productID, int newQuantity){

    //tim kiem order bang ID
    int findOrderIndexByID = findOrderByID(&S, orderID);
    if(findOrderIndexByID == -1 || S.orders[findOrderIndexByID].orderStatus != ORDER_OPEN || findOrderIndexByID >= S.orderCount){
        return FALSE;
    }
    showAllOrderItem(&S.orders[findOrderIndexByID]);
    // bat dau tim kiem san pham trong order do
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
    if(!commitChange(copy, "UPDATE_FOOD", orderID, oldValue, newValue)){
        printf("Cannot save updated order food.\n");
        return FALSE;
    }

    printf("Update order food successfully.\n");
    return TRUE;
}

