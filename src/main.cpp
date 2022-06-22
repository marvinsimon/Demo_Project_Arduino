#include <Arduino.h>
#include <sstream>
#include "EspMQTTClient.h"
#include "Adafruit_BME680.h"
#include "hd44780.h"
#include "hd44780ioClass/hd44780_I2Cexp.h"
#include "main.h"


static constexpr auto clearLine = [](int line) {
    lcd.setCursor(0, line);
    for (int i = 0; i < LCD_COLS; ++i) {
        lcd.write(" ");
    }
    lcd.setCursor(0, line);
};

template<typename T> static constexpr auto printLine = [](T line, int lineNumber) {
    lcd.setCursor(0, lineNumber);
    lcd.print(line);
};

template<typename T> static constexpr auto printLines =
        [](std::initializer_list<T> list, int lineCounter, bool clear = true) {
            std::for_each(list.begin(), list.end(), [&](auto &element) {
                if (clear) {
                    clearLine(lineCounter);
                }
                printLine<String>(element, lineCounter++);
            });
        };

void setup() {
    if(currentState == WorkState::STARTUP) {
        bool result = lcd.begin(LCD_COLS, LCD_ROWS);
        if (result)
            hd44780::fatalError(1);
    }

    while (!bme.begin()) {
        printLines<String>({"Couldn't find a val-", "id BME680 sensor,", "check wiring!"}, 0, false);
        delay(10);
    }
    // Set up oversampling and filter initialization
    bme.setTemperatureOversampling(BME680_OS_8X);
    bme.setHumidityOversampling(BME680_OS_2X);
    bme.setPressureOversampling(BME680_OS_4X);
    bme.setIIRFilterSize(BME680_FILTER_SIZE_3);
    bme.setGasHeater(320, 150); // 320*C for 150 ms

    lcd.clear();
    currentState = WorkState::WIFI_NOT_CONNECTED;
}

void loop() {
    client.loop();
    bool stateChanged = currentState != previousState;
    previousState = currentState;
    switch (currentState) {
        case WorkState::STARTUP:
            setup();
        case WorkState::WIFI_NOT_CONNECTED:
            checkWifiConnected();
            printLines<String>({"WiFi: Not Connected", "MQTT: Not Connected"}, 0, stateChanged);
            break;
        case WorkState::MQTT_NOT_CONNECTED:
            printLines<String>({"WiFi: Connected"}, 0, stateChanged);
            checkMQTTConnected();
            break;
        case WorkState::WORKING:
            printLines<String>({"MQTT: Connected"}, 1, stateChanged);
            workingMachine();
            break;
        case WorkState::ERROR:
            printLines<String>({"Error reading data"}, 2, stateChanged);
            workingMachine();
            break;
    }
}

void workingMachine() {
    if (bme.performReading() && client.isConnected()) {
        currentState = WorkState::WORKING;
        publishData();
    } else if (!client.isConnected()) {
        currentState = WorkState::WIFI_NOT_CONNECTED;
    } else {
        currentState = WorkState::ERROR;
    }
}


void publishData() {
    client.publish("BME680/Temperature", String(bme.temperature), false);
    client.publish("BME680/Pressure", String(bme.pressure / 100), false);
    client.publish("BME680/Humidity", String(bme.humidity), false);
    client.publish("BME680/GasResistance", String(bme.gas_resistance / 1000), false);
    client.publish("BME680/Altitude", String(bme.readAltitude(SEA_LEVEL_PRESSURE)));
}

void checkMQTTConnected() {
    if (client.isMqttConnected()) {
        currentState = WorkState::WORKING;
    } else {
        currentState = WorkState::MQTT_NOT_CONNECTED;
    }
}

void checkWifiConnected() {
    if (client.isWifiConnected()) {
        currentState = WorkState::MQTT_NOT_CONNECTED;
    } else {
        currentState = WorkState::WIFI_NOT_CONNECTED;
    }
}


//-------------------------------------------------------
void onConnectionEstablished() {
    client.subscribe("BME680/Temperature", [](const String &payload) {
        if (bme.performReading()) {
            printLines<String>({"Temperature: " + payload + "*C"}, 2, false);
        }
    });
}
