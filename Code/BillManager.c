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


