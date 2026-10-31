#include <stdio.h>
#include <math.h>
#include "../Include_type_struct/RestaurantTypes.h"
#include <time.h>
#include <stdlib.h>
#include "../Include_type_struct/FileHelper.h"

State *copyState(const State *source)
{
    State *copy;

    if (source == NULL)
        return NULL;

    copy = malloc(sizeof(*copy));

    if (copy == NULL)
        return NULL;

    *copy = *source; // Chép toàn bộ, gồm tables và tableCount. //

    return copy;
}

double roundMoney(double value)
{
    return round(value * 100.0) / 100.0;
}
int calculateOrderTotal(const Order *order, double *total){
    //check cac gia tri nhap vao
    if(total == NULL || order == NULL || order->itemCount < 0 || order->itemCount > MAX_ITEMS){
        return FALSE;
    }

    //tinh tong tien cua order
    double sum = 0;
    for(int i = 0; i < order->itemCount; i++){
        double itemTotal = order->items[i].productPrice * order->items[i].quantity;
        if(!isfinite(itemTotal) || itemTotal < 0 || itemTotal > MAX_MONEY){
            return FALSE;
        }
        sum += itemTotal;
    }

    //kiem tra gia tri sum
    if(!isfinite(sum) || sum < 0 || sum > MAX_MONEY){
        return FALSE;
    }

    *total = sum;
    return TRUE;
    
}
int calculateServiceCharge(int orderType, double subtotal, ChargePolicy policy, double distance, double *fee){
    // kiem tra phi nhap vao
    if(fee == NULL || !isfinite(subtotal) || subtotal < 0 || subtotal > MAX_MONEY){
        return FALSE;
    }
    //check la DINE_IN , TAKE_AWAY, DELIVERY
    switch(orderType){

        case DINE_IN:
        return applyDineInCharge(subtotal, policy.dineInRate, fee);

        case TAKEAWAY:
        return applyTakeawayCharge(subtotal, policy.takeawayRate, fee);

        case DELIVERY:
        return applyDeliveryCharge(distance, policy.deliveryBase, policy.deliveryPerKm, fee);

        default:
        return FALSE;

    }
}

int applyDineInCharge(double subtotal, double rate, double *fee){
    
    //check cac gia tri phi, gia phu co sai hay khong
    if(fee == NULL || !isfinite(subtotal) || subtotal < 0 || subtotal > MAX_MONEY){
        return FALSE;
    }

    //kiem tra rate truoc khi bat dau tinh toan
    if(!isfinite(rate) || rate < 0 || rate > 100){
        return FALSE;
    }

    //tinh phi sau do lam tron den gia tien gan nhat
    *fee = roundMoney(subtotal *rate / 100);
    return TRUE;
}

int applyTakeawayCharge(double subtotal, double rate, double *fee){

    //check cac gia tri phi, gia phu co sai hay khong
    if(fee == NULL || !isfinite(subtotal) || subtotal < 0 || subtotal > MAX_MONEY){
        return FALSE;
    }

    //kiem tra rate truoc khi bat dau tinh toan
    if(!isfinite(rate) || rate < 0 || rate > 100){
        return FALSE;
    }

    //tinh phi sau do lam tron den gia tien gan nhat
    *fee = roundMoney(subtotal *rate / 100);
    return TRUE;
}

int applyDeliveryCharge(double distance, double baseFee, double perKm, double *fee){

    //check cai gia tri
    if(fee == NULL || !isfinite(distance) || distance < 0 || distance > MAX_DISTANCE){
        return FALSE;
    }

    //tinh value theo km

    if(!isfinite(baseFee) || !isfinite(distance) || baseFee < 0 || baseFee > MAX_MONEY || distance < 0 || distance > MAX_DISTANCE){
        return FALSE;
    }

    double value = baseFee + distance * perKm;

    //kiem tra gia tri value

    if(!isfinite(perKm) || perKm < 0 || perKm > MAX_MONEY){
        return FALSE;
    }

    *fee = roundMoney(value);
    return TRUE;

}

int calculateBill(const Order *order, ChargePolicy policy, double discountRate, double taxRate, Bill *bill){
    //check cac gia tri nhap vao
    if(order == NULL || bill == NULL || !isfinite(discountRate) || !isfinite(taxRate) || discountRate < 0 || discountRate > 100 || taxRate < 0 || taxRate > 100){
        return FALSE;
    }

    //tinh tong tien cua order
    double subtotal;
    if(!calculateOrderTotal(order, &subtotal)){ 
        return FALSE;
    }
    //tinh phi dich vu
    double serviceCharge;
    if(!calculateServiceCharge(order->orderType, subtotal, policy, order->deliveryDistance, &serviceCharge)){
        return FALSE;
    }

    //tinh discount va tax
    double discount = roundMoney(subtotal * discountRate / 100);
    double tax = roundMoney((subtotal - discount + serviceCharge) * taxRate / 100);

    double total = subtotal - discount + serviceCharge + tax;
    if (!isfinite(total) || total < 0 || total > MAX_MONEY) {
    return FALSE;
    }

    //tinh tong tien cuoi cung
    bill->totalMoney = subtotal;
    bill->surcharge = serviceCharge;
    bill->discountRate = discountRate;
    bill->discount = discount;
    bill->taxRate = taxRate;
    bill->tax = tax;
    bill->amountDue = total;

    return TRUE;
}

int printBillReport(int orderID, double discountRate, double taxRate){
    //tim kiem order bang ID
    int findOrderIndexByID = findOrderByID(&S, orderID);
    if(findOrderIndexByID == -1 || S.orders[findOrderIndexByID].orderStatus != ORDER_OPEN || findOrderIndexByID >= S.orderCount){
        return FALSE;
    }

    //tinh hoa don
    Bill bill;
    if(!calculateBill(&S.orders[findOrderIndexByID], S.policy, discountRate, taxRate, &bill)){
        return FALSE;
    }

    //in hoa don ra man hinh
    printf("\n========== BILL ==========\n");
    printf("Bill ID: %d\n", bill.billID);
    printf("Order ID: %d\n", bill.orderID);
    printf("Table ID: %d\n", bill.tableID);
    printf("The time of the bill: %02d/%02d/%04d %02d:%02d\n", bill.timeBill.day, bill.timeBill.month, bill.timeBill.year, bill.timeBill.hour, bill.timeBill.minute);
    printf("Name of the products:\n");
    for(int i = 0; i < bill.itemCount; i++){
        printf("%s x %d: %.2f\n", bill.items[i].productName, bill.items[i].quantity, bill.items[i].productPrice * bill.items[i].quantity);
    }
    printf("Subtotal: %.2f\n", bill.totalMoney);
    printf("Service charge: %.2f\n", bill.surcharge);
    printf("Discount: %.2f\n", bill.discount);
    printf("Tax: %.2f\n", bill.tax);
    printf("Amount due: %.2f\n", bill.amountDue);

    printf("==========================\n");

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

    //luu hoa don vao bo nho
    if(S.billCount >= MAX_BILLS){
        printf("Bill storage is full.\n");
        return FALSE;
    }

    State *copy = copyState(&S);

    if(copy == NULL){
        return FALSE;
    }
    // tao hoa don
    Bill bill = {0};

    Order *order = &copy->orders[findOrderIndexByID];

    if(!calculateBill(order, copy->policy, discountRate, taxRate, &bill)){
        free(copy);
        return FALSE;
    }

    //lay thoi gian
    time_t currentTime = time(NULL);
    struct tm *timeInfo = localtime(&currentTime);

    if(timeInfo == NULL){
        free(copy);
        return FALSE;
    }

    bill.timeBill.year = timeInfo->tm_year + 1900;
    bill.timeBill.month = timeInfo->tm_mon + 1;
    bill.timeBill.day = timeInfo->tm_mday;
    bill.timeBill.hour = timeInfo->tm_hour;
    bill.timeBill.minute = timeInfo->tm_min;

    // gan thong tin hoa don
    bill.billID = copy->lastBillID + 1;
    bill.orderID = order->orderID;
    bill.tableID = order->tableID;
    bill.orderType = order->orderType;
    bill.itemCount = order->itemCount;
    bill.deliveryDistance = order->deliveryDistance;
    bill.policySnapshot = copy->policy;

    // copy danh sach mon an
    for(int i = 0; i < order->itemCount; i++){
        bill.items[i] = order->items[i];
    }

    // luu hoa don vao bo nho
    copy->bills[copy->billCount] = bill;
    copy->billCount++;
    copy->lastBillID = bill.billID;

    // cap nhat trang thai order thanh ORDER_PAID
    order->orderStatus = ORDER_PAID;

    // luu Order
    if(!saveTransaction(copy, "Data/OrderHistory.txt")){
        free(copy);
        printf("Cannot save order history.\n");
        return FALSE;
    }

    // luu Bill
    if(!saveTransaction(copy, "Data/OrderHistory.txt")){
        free(copy);
        printf("Cannot save order history.\n");
        return FALSE;
    }

    // cap nhat State
    S = *copy;
    free(copy);

    // in hoa don
    printf("Checkout successfully.\n");

    return TRUE;
}
