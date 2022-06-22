//
// Created by marvi on 22/06/2022.
//

#ifndef UNTITLED_MAIN_H
#define UNTITLED_MAIN_H


enum class WorkState {
    STARTUP, WIFI_NOT_CONNECTED, MQTT_NOT_CONNECTED, WORKING, ERROR
};

WorkState currentState = WorkState::STARTUP;
WorkState previousState = currentState;

/********************
 * Connected Devices
 ********************/
EspMQTTClient client("devolo-3224", "q1w2e3r4", "poshome.ddns.net", "SV_2_DemoBoard");
Adafruit_BME680 bme;
hd44780_I2Cexp lcd;
/********************/

/*******************
 * Some const values
 *******************/
static constexpr int LCD_COLS = 20;
static constexpr int LCD_ROWS = 4;

static constexpr float SEA_LEVEL_PRESSURE = 1013.529;

/**************************/

void checkWifiConnected();

void checkMQTTConnected();

void publishData();

void workingMachine();

#endif //UNTITLED_MAIN_H
