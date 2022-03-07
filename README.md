# R2Dino
### Alarm clock, temperature reading and an old school arcade game made with Arduino Uno

![R2Dino](./img/r2dino.jpg "R2Dino")

#### Story
This is my first Arduino project and c++ programming. I had an old serial 4x20 LCD display and I wanted to use it somehow along with an Arduino Student kit i bought. I decided to create an alarm clock with the help of DS1307 RTC clock which will also display room temperature with the help of an LM35 sensor. I decided also to add a little fun with a mini arcade game based on on the classic Space Invaders.

### Hardware description

![Schematic](./img/R2Dino_schematic.jpg "Schematic")

#### Joystick
Joystick's X-axis output is connected to A0 and Y-axis to A1. Both lines have a 1K resistor to ground in order to have the output voltage close to internal 1.1V reference. The joystick is used to control r2d2 in the arcade game as long as selecting actions in the different menu

#### LM35
LM35's output is connected to A2. I've also set the analog reference to internal. This will provide better temperature resolution.

#### DS1307
This RTC module is communicating via an I2C connection with Arduino. The default I2C implementation uses A4 for SDA and A4 for SCL.

#### Buzzer
Buxxer is connected directly to D10 and uses its PWM output capabilities.

#### Fire button
Is a standard tact switch connected to D3. D3 is used to make r2d2 fire its laser and also select a highlighted menu entry.

#### LCD
I used the SoftwareSerial library to connect the LCD with the Arduino. Using Arduino's default TX/RX pins (D0, D1) was corrupting the lcd during program upload. By using different pins for its communication I resolved this. I only use D12 to transmit serial data to the LCD.

The LCD has a backlight input. I control this input with the help of a PNP transistor. The BC557's base is connected to D13 via an 1K resistor. When D13 is LOW then the backlight is powered, when it's HIGH backlight is off. This helps to reduce power consuption by turning it off after 30secs of inactivity.

