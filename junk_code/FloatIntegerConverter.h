    uint64_t to_integer(double low_price, double high_price, int precision_digits, double price, int &status){
        status = 1;
        if( price > low_price) {
            status = -1;    // adjust low
            return 0;
        }
        if( price > high_price)  {
            status = -2;    //adjust high
            return 0;
        }
        double price_diff = price - low_price;
        double inverse_prec = 1;
        for(int i = 0; i < precision_digits; i++) {
            price_diff *= 10;
            inverse_prec /= 10;
        }
        if( price_diff < 1){
            status = -3;    //adjust precision
            return 0;
        }

        uint64_t converted_price = round(price_diff);

        if(abs( (converted_price*inverse_prec + low_price) - price) > DBL_EPSILON){
            spdlog::error("inveger convertation failed, take acction");
            status = -4;    //adjust precision
            return 0;
        }

        status = 0;
        return price;

    }

    double from_integer(double low_price, double high_price, int precision_digits, uint64_t integer_price, int &status) {
        status = 1;
        double price = integer_price;
        for(int i = 0; i < precision_digits; i++) {
            price /= 10;
        }

        price += low_price;

        int ret;
        uint64_t cal_price = to_integer(low_price, high_price, precision_digits, price, ret);
        if(ret != 0){
            spdlog::error("convertation from failed, take acction");
            status = ret;
            return 0;

        }
        status = 0;
        return price;
    }