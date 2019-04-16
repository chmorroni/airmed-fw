
#include "hardware.h"

#include "board.h"
#include "buttons.h"
#include "buzzer.h"
#include "data_types.h"
#include "drum.h"
#include "file.h"
#include "leds.h"
#include "sensor.h"
#include "shutter.h"

#include <ti/drivers/GPIO.h>

extern dispense_counts_t dispense_counts;

void init_button_interrupts()
{
    GPIO_setCallback(BOARD_GPIO_BUTTON_PATIENT, patient_button_callback);
}

#define BUZZER_FREQ (1000)
#define DISPENSE_RETRIES (2)

void dispense()
{
    uint8_t pills_dispensed;
    bool dispense_failed = false;

    for (uint8_t medication_idx = 0; medication_idx < NUM_MEDICATIONS; medication_idx++)
    {
        if (dispense_counts.medication[medication_idx] > 0)
        {
            drum_set_hopper(medication_idx);
            pills_dispensed = shutter_dispense();

            for (uint8_t i = 0; i < DISPENSE_RETRIES && pills_dispensed < dispense_counts.medication[medication_idx]; i++)
            {
                pills_dispensed += shutter_dispense();
            }

            if (pills_dispensed != dispense_counts.medication[medication_idx])
            {
                dispense_failed = true;
                break;
            }
        }
    }

    // TODO update quantities

    if (dispense_failed == true)
    {
        led_error_on();
    }
    else
    {
        led_status_on();
        buzzer_set_freq(BUZZER_FREQ);
        GPIO_enableInt(BOARD_GPIO_BUTTON_PATIENT);
    }
}

void patient_button_callback(uint_least8_t index)
{
    GPIO_disableInt(index);
    led_status_off();
    buzzer_off();
}