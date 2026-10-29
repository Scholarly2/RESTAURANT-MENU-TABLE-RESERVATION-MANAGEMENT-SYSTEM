#include <stdio.h>
#include <math.h>
#include <RestaurantTypes.h>

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