#include "xil_printf.h"
#include "stdint.h"

#define DELAY_UNIT 81
#define BUTTONS (* (unsigned volatile *) 0x40000000)
#define ANODES (* (unsigned volatile *) 0x40006000)
#define SEG (* (unsigned volatile *) 0x40020000)
#define LEDS (* (unsigned volatile *) 0x40005000)
#define JXADC_CH6 (*(uint16_t volatile *)0x44a20258)
#define JXADC_CH14 (*(uint16_t volatile *)0x44a20278)
#define BTN ( *(volatile unsigned *) 0x40030000 ) // GPIO-3 4-bit, (btnR, btnL, btnD, btnU)

#define THRESHOLD 50  // Minimum change needed to consider a new value

void delay_ms(unsigned t)
{
    unsigned cntr1, cntr2;
    while (t--)
    {
        for (cntr1 = 0; cntr1 < 100; cntr1++)
        {
            for (cntr2 = 0; cntr2 < DELAY_UNIT; cntr2++) {}
        }
    }
}

_Bool up_button_press()
{
    static unsigned char prevState = 0;
    unsigned char currentState = BTN & 0x1;

    if (currentState && !prevState) {
        prevState = currentState;
        return 1;
    } else {
        prevState = currentState;
        return 0;
    }
}

int measure_duty_cycle_coil0()
{
    int samples = 1000;
    int high_count = 0;

    for (int i = 0; i < samples; i++)
    {
        uint16_t reading = ((JXADC_CH6 >> 4) * 240 / 1000);
        if (reading > 500)
        {
            high_count++;
        }
    }

    return (high_count * 100) / samples;
}

int measure_duty_cycle_coil1()
{
    int samples = 1000;
    int high_count = 0;

    for (int i = 0; i < samples; i++)
    {
        uint16_t reading = ((JXADC_CH14 >> 4) * 240 / 1000);
        if (reading > 500)
        {
            high_count++;
        }
    }

    return (high_count * 100) / samples;
}

void strength_meter()
{
	int coil0_duty = measure_duty_cycle_coil0();
	int coil1_duty = measure_duty_cycle_coil1();

	// LED logic (updated for clarity and unique thresholds)
	if (coil0_duty <= 1 || coil1_duty <= 13)
	{
		LEDS = 0xFFFF;
	}
	else if (coil0_duty <= 2 || coil1_duty <= 14)
	{
		LEDS = 0x3FFF;
	}
	else if (coil0_duty <= 3 || coil1_duty <= 15)
	{
		LEDS = 0x07FF;
	}
	else if (coil0_duty <= 4 || coil1_duty <= 16)
	{
		LEDS = 0x007F;
	}
	else if (coil0_duty <= 5 || coil1_duty <= 17)
	{
		LEDS = 0x000F;
	}
	else if (coil0_duty <= 6 || coil1_duty <= 18)
	{
		LEDS = 0x0007;
	}
	else if (coil0_duty <= 7 || coil1_duty <= 19)
	{
		LEDS = 0x0003;
	}
	else
	{
		LEDS = 0x0000;
	}
}


void display_location(unsigned left_ctr, unsigned mid_ctr, unsigned right_ctr, unsigned pos)
{
    unsigned data[4];
    unsigned pos_lut[4] = { 0x39, 0x38, 0x50, 0x00 }; // Position codes
    unsigned sevenSegLUT[16] = {
        0x40, 0x79, 0x24, 0x30,
        0x19, 0x12, 0x02, 0x78,
        0x00, 0x10, 0x77, 0x7C,
        0x39, 0x5E, 0x79, 0x71
    };

    data[0] = left_ctr%16;
    data[1] = mid_ctr%16;
    data[2] = right_ctr%16;
    data[3] = pos%4;

    static uint8_t state = 0;

    ANODES = ~(1 << state); // Activate one digit

    switch(state)
    {
    case 0:
        SEG = sevenSegLUT[data[0]]; // left counter
        break;
    case 1:
        SEG = sevenSegLUT[data[1]]; // mid counter
        break;
    case 2:
        SEG = sevenSegLUT[data[2]]; // right counter
        break;
    case 3:
        SEG = pos_lut[data[3]];     // last detected position
        break;
    }

    state = (state + 1) % 4; // Move to next digit
}

int main()
{
    uint8_t left_ctr  = 0,
            mid_ctr   = 0,
            right_ctr = 0,
            pos       = 0;

    print("Start here!\n");

    while (1)
    {
        // 1) Continuously measure and update LEDs
        int coil0_duty = measure_duty_cycle_coil0();
        int coil1_duty = measure_duty_cycle_coil1();
        strength_meter();

        if (up_button_press())
        {
            if (coil0_duty <= 3)
            {
                left_ctr++;
                pos = 0;
            }
            else if (coil0_duty > 3 && coil0_duty < 7 && coil1_duty > 14 && coil1_duty < 19)
            {
                mid_ctr++;
                pos = 1;
            }
            else if (coil1_duty <= 15)
            {
                right_ctr++;
                pos = 2;
            }
            else if (coil0_duty >= 7 && coil1_duty > 19)
            {
                pos = 3;
            }
        }

        display_location(left_ctr, mid_ctr, right_ctr, pos);

        delay_ms(1);
    }
}

//int main()
//{
//	uint8_t left_ctr =0, mid_ctr = 0, right_ctr =0 , pos =0 ;
//    print("Start here! \n");
//
//
//
//    while (1)
//    {
//    	int coil0_duty = measure_duty_cycle_coil0();
//    	int coil1_duty = measure_duty_cycle_coil1();
//    	strength_meter();
//
//
//		if (up_button_press())
//		{
//			if (coil0_duty <= 3)
//			{
//				left_ctr++;
//				pos = 0;
//				display_location(left_ctr, mid_ctr, right_ctr, pos);
//			}
//			else if (coil0_duty > 3 && coil0_duty < 7 && coil1_duty > 14 && coil1_duty < 19)
//			{
//				mid_ctr++;
//				pos = 1;
//				display_location(left_ctr, mid_ctr, right_ctr, pos);
//			}
//			else if (coil1_duty <= 15)
//			{
//				right_ctr++;
//				pos = 2;
//				display_location(left_ctr, mid_ctr, right_ctr, pos);
//			}
//			else if (coil0_duty >= 7 && coil1_duty > 19)
//			{
//				pos = 3;
//				display_location(left_ctr, mid_ctr, right_ctr, pos);
//			}
//
//			// --- Display on Seven Segment ---
//			display_location(left_ctr, mid_ctr, right_ctr, pos);
//		}
//
//		// --- Small Delay for Multiplexing ---
//		delay_ms(1);
//
//
//
//    }
//}
