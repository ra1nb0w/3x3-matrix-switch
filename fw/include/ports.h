#ifndef _PORTS_H_
#define _PORTS_H_

#include <Arduino.h>
#include <EEPROM.h>
#include "clsPCA9555.h"
#include "config.h"

// ports available on the matrix switch
#define INPUT_PORTS 3
#define OUTPUT_PORTS 3

// number of elements available
#define LED_NUMBER 9
#define BUTTON_NUMBER 3
#define RELAY_NUMBER 12

// EEPROM addresses
#define EEPROM_CANARY_ADDR 0x00
#define EEPROM_CANARY_VALUE 0xAE
#define EEPROM_IN1_ADDR 0x01
#define EEPROM_IN2_ADDR 0x02
#define EEPROM_IN3_ADDR 0x03
#define EEPROM_CANARY_NAME_ADDR 0x04
#define EEPROM_CANARY_NAME_VALUE 0xA1
#define EEPROM_IN1_NAME_ADDR 0x05 // 15 chars + terminator
#define EEPROM_IN2_NAME_ADDR 0x15
#define EEPROM_IN3_NAME_ADDR 0x25
#define EEPROM_OUT1_NAME_ADDR 0x35
#define EEPROM_OUT2_NAME_ADDR 0x45
#define EEPROM_OUT3_NAME_ADDR 0x55

#define NAME_STRING_LENGTH 16 // with null terminator

class Ports
{
public:
    Ports();
    ~Ports();
    void init();
    void initialize_leds();
    void leds_enable();
    void leds_disable();
    void initialize_buttons();
    void initialize_relays();
    void relays_enable();
    void relays_disable();
    void make_routing();
    void check_buttons();
    void change_next_route(uint8_t input);
    void set_inout(uint8_t in1_out, uint8_t in2_out, uint8_t in3_out, uint8_t save);
    uint8_t *get_inout();
    void store_routes();
    void load_routes();
    void load_port_name();
    void store_names();
    void set_port_name(String in1, String in2, String in3, String out1, String out2, String out3);

    // NOT GOOD since should be private but we need to read from webserver
    // runtime routing connection between inut and output
    uint8_t switch_connection[INPUT_PORTS+1];

    // NOT GOOD but used in the webserver
    char in_port_name[INPUT_PORTS+1][NAME_STRING_LENGTH];
    char out_port_name[OUTPUT_PORTS+1][NAME_STRING_LENGTH];

private:
    // on second PCA9555
    const uint8_t led_pinout[LED_NUMBER+1] = {
        0,  // NULL
        2,  // IN1OUT1
        4,  // IN1OUT2
        6,  // IN1OUT3
        1,  // IN2OUT1
        3,  // IN2OUT2
        5,  // IN2OUT3
        12, // IN3OUT1
        13, // IN3OUT2
        14  // IN3OUT3
    };

    // on second PCA9555; inverted polarity
    const uint8_t button_pinout[BUTTON_NUMBER+1] = {
        0,  // NULL
        9,  // BUTTON 1
        10, // BUTTON 2
        11  // BUTTON 3
    };

    // on first PCA9555
    const uint8_t relay_pinout[RELAY_NUMBER+1] = {
        0,  // NULL
        6,  // K1
        5,  // K2
        3,  // K3
        13, // K4
        11, // K5
        10, // K6
        4,  // K7
        2,  // K8
        12, // K9
        0,  // K10
        1,  // K11
        14  // K12
    };

    typedef struct {
        // number of relay to enable
        uint8_t step;
        // we admit maximum 5 relays on the path
        // since we disable all relay before any
        // configuration we declare only on relay
        uint8_t relay[5];
        // led to power on for this configuration
        // reference led_pinout[] position
        uint8_t led;
    } path;

    // lookup table to build the path from input to output
    path routing[INPUT_PORTS+1][OUTPUT_PORTS+1];

    // the button that it is pressed
    // 0 mean nothing
    uint8_t pressed_button;
    uint8_t old_button;

    // used to flip ports on the next stage
    uint8_t next_flip;

    // array that contains the address of the name ports
    // index 0 is null
    const uint8_t in_name_eeprom_array[INPUT_PORTS+1] = {
        0, // NULL
        EEPROM_IN1_NAME_ADDR,
        EEPROM_IN2_NAME_ADDR,
        EEPROM_IN3_NAME_ADDR
    };
    const uint8_t out_name_eeprom_array[OUTPUT_PORTS+1] = {
        0, // NULL
        EEPROM_OUT1_NAME_ADDR,
        EEPROM_OUT2_NAME_ADDR,
        EEPROM_OUT3_NAME_ADDR
    };
};

extern Ports ports;

#endif /* _PORTS_H_ */
