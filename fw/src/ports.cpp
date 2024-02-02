#include "ports.h"

// Object to manage the ports
PCA9555 ioport1(0x20); // relay expander
PCA9555 ioport2(0x21); // leds and buttons expander


Ports::Ports()
{
    EEPROM.begin(512);  //Initialize EEPROM

    // default input-output path
    switch_connection[0] = 0; // dummy

    // default
    pressed_button = 0;
    next_flip = 0;

    // define the path
    memset(routing, 0, sizeof(routing));

    // 1 -> 1 OFF(K1, K4)
    routing[1][1] = { .step = 0, .relay = {0}, .led = 1};
    // 1 -> 2 ON(K1, K9, K5)
    routing[1][2] = { .step = 3, .relay = {1, 9, 5}, .led = 2};
    // 1 -> 3 ON(K1, K6) OFF(K9)
    routing[1][3] = { .step = 2, .relay = {1, 6}, .led = 3};

    // 2 -> 1 ON(K2, K7, K4)
    routing[2][1] = { .step = 3, .relay = {2, 7, 4}, .led = 4};
    // 2 -> 2 OFF(K2, K12, K11, K5)
    routing[2][2] = { .step = 0, .relay = {0}, .led = 5};
    // 2 -> 3 OFF(K2, K6) ON(K12, K10)
    routing[2][3] = { .step = 2, .relay = {12, 10}, .led = 6};

    // 3 -> 1 ON(K3, K4) OFF(K7)
    routing[3][1] = { .step = 2, .relay = {3, 4}, .led = 7};
    // 3 -> 2 OFF(K3, K5) ON(K8, K11)
    routing[3][2] = { .step = 2, .relay = {8, 11}, .led = 8};
    // 3 -> 3 OFF(K3, K8, K10, K6)
    routing[3][3] = { .step = 0, .relay = {0}, .led = 9};

    memset(in_port_name, '\0', sizeof(in_port_name));
    memset(out_port_name, '\0', sizeof(out_port_name));
}

Ports::~Ports()
{
}

void Ports::init()
{
    // initialiaze the pca9555 ports
    ioport1.begin();
    ioport2.begin();

    // sets the I2C clock to 400kHz
    ioport1.setClock(400000);
    ioport2.setClock(400000);

    // load stored routing from the EEPROM
    load_routes();

    // load ports name
    load_port_name();
}


void Ports::initialize_leds()
{
    uint8_t i;
    // configure the led ports
    for (i=0; i < sizeof(led_pinout); i++) {
        ioport2.pinMode(led_pinout[i], OUTPUT);
    }
}

void Ports::leds_enable()
{
    uint8_t i;
    for (i=0; i < sizeof(led_pinout); i++) {
        ioport2.digitalWrite(led_pinout[i], LOW);
    }
}

void Ports::leds_disable()
{
    uint8_t i;
    for (i=0; i < sizeof(led_pinout); i++) {
        ioport2.digitalWrite(led_pinout[i], HIGH);
    }
}

void Ports::initialize_buttons()
{
    uint8_t i;
    for (i=0; i < sizeof(button_pinout); i++) {
      ioport2.pinMode(button_pinout[i], INPUT);
     }
}

void Ports::initialize_relays()
{
    uint8_t i;
    for (i=0; i < sizeof(relay_pinout); i++) {
        ioport1.pinMode(relay_pinout[i], OUTPUT);
     }
}

void Ports::relays_enable()
{
    uint8_t i;
    for (i=0; i < sizeof(relay_pinout); i++) {
        ioport1.digitalWrite(relay_pinout[i], HIGH);
    }
}

void Ports::relays_disable()
{
    uint8_t i;
    for (i=0; i < sizeof(relay_pinout); i++) {
        ioport1.digitalWrite(relay_pinout[i], LOW);
    }
}

// configure the relays based on the
// lookup table and the configuration
void Ports::make_routing()
{
    // disable all led before enable it
    leds_disable();

    // reset all ports to the default status
    relays_disable();

    uint8_t input_port;
    uint8_t tmp_r;
    path tmp_path;
    for(input_port=1; input_port <= INPUT_PORTS; input_port++)
    {
        tmp_path = routing[input_port][switch_connection[input_port]];

        // power on the right led
        for (tmp_r = 0; tmp_r < tmp_path.step; tmp_r++) {
            ioport1.digitalWrite(relay_pinout[tmp_path.relay[tmp_r]], HIGH);
        }

        // power on the right led
        ioport2.digitalWrite(led_pinout[tmp_path.led], LOW);
    }
}

void Ports::check_buttons()
{
    uint8_t i;
    
    if (pressed_button != 0) {
        if (ioport2.digitalRead(button_pinout[pressed_button]) == HIGH)
           pressed_button = 0;  // reset the status
    } else {
        // check if there is a press
        for (i=1; i <= BUTTON_NUMBER; i++) {
           if (ioport2.digitalRead(button_pinout[i]) == LOW) {
               DEBUG("Button ");
               DEBUG(i);
               DEBUGLN(" is pressed");
               pressed_button = i;
               change_next_route(pressed_button);
           }
        }
    }

    // just a bit of delay
    delay(10);
}

// Manage the buttons changing the routing incrementally
void Ports::change_next_route(uint8_t input)
{
    uint8_t tmp, next, next2;

    next = input+1 > INPUT_PORTS ? 1 : input+1;
    next2 = input+2 > INPUT_PORTS ? (input == 2 ? 1 : 2) : input+2;

    if (next_flip && old_button == input) {
        tmp = switch_connection[input];
        switch_connection[input] = switch_connection[next];
        switch_connection[next] = tmp;
        next_flip = 0;
    } else {
        tmp = switch_connection[next];
        switch_connection[next] = switch_connection[next2];
        switch_connection[next2] = tmp;
        next_flip = 1;
        old_button = input;
    }

    DEBUG("Button pressed; move to ports: 1=");
    DEBUG(switch_connection[1]);
    DEBUG(" 2=");
    DEBUG(switch_connection[2]);
    DEBUG(" 3=");
    DEBUGLN(switch_connection[3]);

    make_routing();

    // when we press the buttons
    // we always store the new configuration
    store_routes();
}

// maybe use an array to be easily scalable
void Ports::set_inout(uint8_t in1_out, uint8_t in2_out, uint8_t in3_out, uint8_t save)
{
    // just to avoid useless cycles
    if (switch_connection[1] == in1_out &&
        switch_connection[2] == in2_out &&
        switch_connection[3] == in3_out &&
        save == 0)
        return;

    switch_connection[1] = in1_out > OUTPUT_PORTS ? 1 : in1_out;
    switch_connection[2] = in2_out > OUTPUT_PORTS ? 2 : in2_out;
    switch_connection[3] = in3_out > OUTPUT_PORTS ? 3 : in3_out;

    // check if we need to save
    if (save == 1) {
        store_routes();
    }

    make_routing();
}

void Ports::store_routes()
{
    EEPROM.write(EEPROM_IN1_ADDR, switch_connection[1]);
    EEPROM.write(EEPROM_IN2_ADDR, switch_connection[2]);
    EEPROM.write(EEPROM_IN3_ADDR, switch_connection[3]);
    EEPROM.commit();
}

void Ports::load_routes()
{
    // check if have good values stored
    if (EEPROM.read(EEPROM_CANARY_ADDR) != EEPROM_CANARY_VALUE) {
        EEPROM.write(EEPROM_CANARY_ADDR, EEPROM_CANARY_VALUE);
        DEBUGLN("Initialize EEPROM for switch connection");
        // default input-output path
        switch_connection[1] = 1;
        switch_connection[2] = 2;
        switch_connection[3] = 3;
        store_routes();
    }

    switch_connection[1] = EEPROM.read(EEPROM_IN1_ADDR);
    switch_connection[2] = EEPROM.read(EEPROM_IN2_ADDR);
    switch_connection[3] = EEPROM.read(EEPROM_IN3_ADDR);
}

void Ports::load_port_name()
{
    uint8_t i, y;

    // check if have good values stored
    if (EEPROM.read(EEPROM_CANARY_NAME_ADDR) != EEPROM_CANARY_NAME_VALUE) {
        DEBUGLN("Initialize EEPROM for ports name");
        EEPROM.write(EEPROM_CANARY_NAME_ADDR, EEPROM_CANARY_NAME_VALUE);
        // default input-output name
        // NOTE: only valid UP to 9
        for (i=1; i <= INPUT_PORTS; i++) {
            in_port_name[i][0] = '0'+i;
            in_port_name[i][1] = '\0';
        }
        for (i=1; i <= OUTPUT_PORTS; i++) {
            out_port_name[i][0] = '0'+i;
            out_port_name[i][1] = '\0';
        }
        store_names();
    }
    
    // we use different cycles to generalize the code for non symmetric matrix switches
    for (i=1; i <= INPUT_PORTS; i++) {
        for (y=0; y < NAME_STRING_LENGTH; y++) {
            in_port_name[i][y] = EEPROM.read(in_name_eeprom_array[i]+y);
        }
    }

    for (i=1; i <= OUTPUT_PORTS; i++) {
        for (y=0; y < NAME_STRING_LENGTH; y++) {
            out_port_name[i][y] = EEPROM.read(out_name_eeprom_array[i]+y);
        }
    }
}

void Ports::store_names()
{
    uint8_t i, y;
    for (i=1; i <= INPUT_PORTS; i++) {
        for (y=0; y < NAME_STRING_LENGTH; y++) {
            EEPROM.write(in_name_eeprom_array[i]+y, in_port_name[i][y]);
        }
    }

    for (i=1; i <= OUTPUT_PORTS; i++) {
        for (y=0; y < NAME_STRING_LENGTH; y++) {
            EEPROM.write(out_name_eeprom_array[i]+y, out_port_name[i][y]);
        }
    }

    EEPROM.commit();
}

void Ports::set_port_name(String in1, String in2, String in3, String out1, String out2, String out3)
{
    in1.toCharArray(in_port_name[1], sizeof(in_port_name[1]));
    in2.toCharArray(in_port_name[2], sizeof(in_port_name[2]));
    in3.toCharArray(in_port_name[3], sizeof(in_port_name[3]));

    out1.toCharArray(out_port_name[1], sizeof(out_port_name[1]));
    out2.toCharArray(out_port_name[2], sizeof(out_port_name[2]));
    out3.toCharArray(out_port_name[3], sizeof(out_port_name[3]));

    store_names();
}


// declare the object
Ports ports;