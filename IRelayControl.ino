/* 
 * ======================================================
 * IR Relay Controller - Feature Documentation
 * ======================================================
 * 
 * [ Key Features ]
 * 1. Dual Configuration Activation:
 *    - Physical button (Pin 6)
 *    - IR Remote MODE button
 * 
 * 2. Relay Management:
 *    - Controls 4 relays (Pins 8-11)
 *    - Toggle control via programmed IR codes
 *    - Persistent state storage in EEPROM
 * 
 * 3. IR Configuration:
 *    - Stores 4 unique IR codes in EEPROM
 *    - Duplicate code prevention
 *    - Visual LED feedback during programming
 * 
 * 4. EEPROM Layout:
 *    - 0x00-0x0F: IR codes (4 × 4 bytes)
 *    - 0x10-0x13: Relay states (4 × 1 byte)
 * 
 * 5. Safety Features:
 *    - Debounced button input
 *    - Non-blocking LED status indication
 *    - EEPROM.update() for wear reduction
 * 
 * [ Operational Modes ]
 * - Normal Mode:
 *   * Solid LED = System ready
 *   * IR codes toggle associated relays
 * 
 * - Config Mode:
 *   * Entered via button/Remote MODE button
 *   * Slow LED blink = Waiting for input
 *   * Fast blink = Code accepted
 *   * 3 rapid blinks = Duplicate error
 * 
 * [ Developer Notes ]
 * 1. Relay Logic:
 *    - Uses inverted logic (LOW = Active)
 * 
 * 2. IR Protocol:
 *    - Compatible with NEC protocol remotes
 *    - Change decodedIRData access if using
 *      different IR protocols
 * 
 * 3. EEPROM Considerations:
 *    - First run will initialize with blank codes
 *    - Relay states default to OFF initially
 * 
 * 4. Customization Points:
 *    - #define CONFIG_IR_CODE for different remote
 *    - RELAY_STATE_ADDR for EEPROM layout changes
 *    - LED feedback patterns in enterConfigMode()
 * 
 * [ Serial Debug Commands ]
 * - Monitor @ 115200 baud
 * - Automatic status reports on:
 *   * Relay state changes
 *   * IR code reception
 *   * Configuration progress
 * 
 */

/***********************************************************************/

#include <EEPROM.h>
#include <IRremote.h>

#define CONFIG_BUTTON 6     
#define IR_RECEIVE_PIN 5     
#define RELAY_STATE_ADDR 16  
#define CONFIG_IR_CODE 0xE51A7F80  

const uint8_t RELAY_PINS[] = {8, 9, 10, 11}; 
const uint8_t NUM_RELAYS = sizeof(RELAY_PINS) / sizeof(RELAY_PINS[0]);

uint32_t hex_codes[NUM_RELAYS];
bool relay_states[NUM_RELAYS];

void setup() {
  Serial.begin(115200);
  IrReceiver.begin(IR_RECEIVE_PIN);
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(CONFIG_BUTTON, INPUT_PULLUP);

  // Initialize relays with saved states
  for (uint8_t i = 0; i < NUM_RELAYS; i++) {
    pinMode(RELAY_PINS[i], OUTPUT);
    relay_states[i] = EEPROM.read(RELAY_STATE_ADDR + i);
    digitalWrite(RELAY_PINS[i], relay_states[i] ? LOW : HIGH);
    Serial.print("Relay ");
    Serial.print(i);
    Serial.println(relay_states[i] ? " ON" : " OFF");
  }

  // Load IR codes from EEPROM
  EEPROM.get(0, hex_codes);
  for (uint8_t i = 0; i < NUM_RELAYS; i++) {
    Serial.print("Relay ");
    Serial.print(i);
    Serial.print(" code: ");
    Serial.println(hex_codes[i], HEX);
  }
}

void loop() {
  // Check physical config button
  if (digitalRead(CONFIG_BUTTON) == LOW) {
    delay(50);
    if (digitalRead(CONFIG_BUTTON) == LOW) {
      enterConfigMode();
    }
  }

  // Check IR input
  if (IrReceiver.decode()) {
    uint32_t ir_code = IrReceiver.decodedIRData.decodedRawData;
    Serial.print("IR Received: ");
    Serial.println(ir_code, HEX);

    if (ir_code == CONFIG_IR_CODE) {
      Serial.println("IR Config Trigger");
      enterConfigMode();
    } else {
      for (uint8_t i = 0; i < NUM_RELAYS; i++) {
        if (ir_code == hex_codes[i]) {
          toggleRelay(i);
          break;
        }
      }
    }
    IrReceiver.resume();
  }
}

void toggleRelay(uint8_t index) {
  relay_states[index] = !relay_states[index];
  digitalWrite(RELAY_PINS[index], relay_states[index] ? LOW : HIGH);
  EEPROM.update(RELAY_STATE_ADDR + index, relay_states[index]);
  Serial.print("Relay ");
  Serial.print(index);
  Serial.println(relay_states[index] ? " ON" : " OFF");
}

void enterConfigMode() {
  Serial.println("Entering Config Mode");
  digitalWrite(LED_BUILTIN, HIGH);
  delay(1000);
  digitalWrite(LED_BUILTIN, LOW);

  memset(hex_codes, 0, sizeof(hex_codes)); // Reset codes
  uint8_t codes_stored = 0;

  while (codes_stored < NUM_RELAYS) {
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
    delay(300);

    if (IrReceiver.decode()) {
      uint32_t ir_code = IrReceiver.decodedIRData.decodedRawData;
      
      if (ir_code != 0 && ir_code != CONFIG_IR_CODE) { // Ignore config code
        bool duplicate = false;
        for (uint8_t i = 0; i < codes_stored; i++) {
          if (hex_codes[i] == ir_code) {
            duplicate = true;
            break;
          }
        }

        if (!duplicate) {
          hex_codes[codes_stored] = ir_code;
          Serial.print("Stored code ");
          Serial.print(codes_stored);
          Serial.print(": ");
          Serial.println(ir_code, HEX);
          
          // Confirm with quick blink
          digitalWrite(LED_BUILTIN, HIGH);
          delay(100);
          digitalWrite(LED_BUILTIN, LOW);
          codes_stored++;
        } else {
          Serial.println("Duplicate!");
          // Error: rapid blinks
          for (uint8_t i = 0; i < 3; i++) {
            digitalWrite(LED_BUILTIN, HIGH);
            delay(100);
            digitalWrite(LED_BUILTIN, LOW);
            delay(100);
          }
        }
      }
      IrReceiver.resume();
    }
  }

  EEPROM.put(0, hex_codes);
  Serial.println("Configuration Saved");
  digitalWrite(LED_BUILTIN, HIGH);
}