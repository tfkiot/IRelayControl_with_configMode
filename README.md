# IRelayControl_with_configMode
IR Relay Controller is an Arduino-based system that controls up to 4 relays via an IR remote. It saves relay states and IR codes in EEPROM for persistence. A config mode allows programming new IR codes using a button or a special IR signal. Built-in LED provides feedback. Ideal for home automation and remote-controlled relay applications. 🚀

## Features  
- Control up to 4 relays using an IR remote  
- Save relay states in EEPROM for persistence across power cycles  
- Store custom IR codes for each relay  
- Config mode to program new IR codes using push button or any remote button   
- Status feedback via serial monitor and built-in LED  

## Hardware Requirements  
- Arduino board (e.g., Uno, Nano)  
- IR receiver module (e.g., TSOP1838)  
- 4-channel relay module  
- Push button for config mode  
- Infrared remote control  

## Wiring  
| Component      | Pin Connection |
|---------------|---------------|
| IR Receiver   | D5            |
| Config Button | D6            |
| Relay 1       | D8            |
| Relay 2       | D9            |
| Relay 3       | D10           |
| Relay 4       | D11           |

## Installation  
1. Clone this repository:  
   ```sh
   git clone https://github.com/tfkiot/IRelayControl_with_configMode.git
