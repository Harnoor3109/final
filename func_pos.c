


void pos_display(uint8_t coil0_duty, uint8_t coil1_duty) {
    uint8_t show_sseg;


    if (coil0_duty <= 3) {
        show_sseg = pos_lut[1];
    }

    else if (coil0_duty > 3 && coil0_duty < 7
      && coil1_duty > 14 && coil1_duty < 19) {
    show_sseg =  pos_lut[0];
    }
    else if (coil1_duty <= 15){
        show_sseg =  pos_lut[2];
    }
    else if (coil0_duty >= 7 && coil1_duty > 19) {
        show_sseg =  pos_lut[3];
    }

    }

