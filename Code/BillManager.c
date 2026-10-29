#include <stdio.h>
#include <math.h>
#include <RestaurantTypes.h>

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
    if(!isfinite(rate) < 0 || rate > 100){
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
    if(!isfinite(rate) < 0 || rate > 100){
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

    if( !isfinite(value) || value > MAX_MONEY){
        return  FALSE;
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
    bill->subtotal = subtotal;
    bill->serviceCharge = serviceCharge;
    bill->discount = discount;
    bill->tax = tax;
    bill->total = total;

    return TRUE;
}
