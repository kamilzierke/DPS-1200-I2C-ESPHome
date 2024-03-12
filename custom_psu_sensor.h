#include "esphome.h"

using namespace esphome;

class CustomPSUSensor : public PollingComponent {
 public:
  Sensor *grid_voltage_sensor = new Sensor();
  Sensor *grid_current_sensor = new Sensor();
  Sensor *output_voltage_sensor = new Sensor();
  Sensor *output_current_sensor = new Sensor();
  Sensor *internal_temp_sensor = new Sensor();
  Sensor *fan_speed_sensor = new Sensor();

  CustomPSUSensor() : PollingComponent(1000) {}  // Polling interval: 15 seconds

  void setup() override {
    Wire.begin();
  }

  void update() override {
    const uint8_t addy = 0x5F;  // Power Supply I2C address
    uint8_t reg[6] = {0x08, 0x0a, 0x0e, 0x10, 0x1c, 0x1e};  // Register addresses
    float measurements[6] = {0, 0, 0, 0, 0, 0};  // To store calculated values

    for (int i = 0; i < 6; i++) {
      // Calculate checksum for address and register
      uint8_t cs = (addy << 1) + reg[i];
      uint8_t regCS = ((0xff - cs) + 1) & 0xff;

      // Begin I2C transmission and send register address and checksum
      Wire.beginTransmission(addy);
      Wire.write(reg[i]);
      Wire.write(regCS);
      Wire.endTransmission();

      // Request 3 bytes of data from the power supply
      Wire.requestFrom(static_cast<uint8_t>(addy), static_cast<uint8_t>(3));
      if (Wire.available() == 3) {
        uint16_t msg[3];
        msg[0] = Wire.read();
        msg[1] = Wire.read();  // Actual data bytes
        msg[2] = Wire.read();  // Checksum byte, not used in calculation

        // Calculate the actual measurement value
        float ret = (msg[1] << 8) + msg[0];
        float stat = 0;
        switch (i) {
          case 0:  // Grid Voltage
            stat = ret / 32.0;
            measurements[i] = stat;
            grid_voltage_sensor->publish_state(stat);
            break;
          case 1:  // Grid Current
            stat = ret / 128.0;
            measurements[i] = stat;
            grid_current_sensor->publish_state(stat);
            break;
          case 2:  // Output Voltage
            stat = ret / 256.0;
            measurements[i] = stat;
            output_voltage_sensor->publish_state(stat);
            break;
          case 3:  // Output Current
            stat = ret / 64.0;
            measurements[i] = stat;
            output_current_sensor->publish_state(stat);
            break;
          case 4:  // Internal Temperature
            stat = ret / 64.0;
            measurements[i] = stat;
            internal_temp_sensor->publish_state(stat);
            break;
          case 5:  // Fan Speed
            stat = ret;  // No scale factor for fan speed
            measurements[i] = stat;
            fan_speed_sensor->publish_state(stat);
            break;
        }
      }
    }
  }
};
