//Main-board LED
#define MAIN_LED LED_BUILTIN

// expansion board
// LED's
#define LED0 2
#define LED1 3
#define LED2 4
#define LED3 5

#define LED4 6
#define LED5 7
#define LED6 8
#define LED7 9
// Temperature sensor (analog input 5)
#define TEMP A5

// Additional Defines
#define ON  1       //Led is on
#define OFF 0       // Led is off
#define MAIN_LED_INTERVALL  200

enum States
{
    Waiting,
    RunningLed,
    Measure,
};

States CurrentState = Waiting;

// Global Variables
unsigned char _ledArray[8] = { LED0,LED1,LED2,LED3,LED4,LED5,LED6,LED7 };
unsigned long _nextInterval;
unsigned long _nextMainLedInterval;
unsigned long _interval = 500;
unsigned long _currentMillis = 0;

/***************************************************************************************
 setup()
 runs once at the initialization of the program.
 */
void setup() {
    //analogReference(EXTERNAL); // switching from intern to extern reference voltage

    // switching pins with leds to output
    pinMode(MAIN_LED, OUTPUT);
    for (int i = 0; i < 8; i++)
        pinMode(_ledArray[i], OUTPUT);

    // Initialize serial interface
    Serial.begin(115200);

    // Initialize led blinking
    _nextMainLedInterval = millis() + MAIN_LED_INTERVALL;

    Serial.println("Version 1.1, 14.08.2022");
    Serial.println("-h for help");

}

// Main-program infinite loop
void loop() {
    // determine current runtime
    _currentMillis = millis();

    // data on serial interface available
    if (Serial.available() > 0){
        String s = Serial.readString();

        if (s.length() > 0){
            int value = -1;
            Serial.print("ECHO: ");
            Serial.println(s);

            if (s.length() > 2)
                value = GetCommandValue(s);

            s.toLowerCase();
            if (s[0] == '-'){
                switch (s[1]){
                    case 'h':
                        Serial.println("###################################");
                        Serial.println("############# Commands ############");
                        Serial.println("###################################");
                        Serial.println("\n-h: show help");
                        Serial.println("-b [0-255]: show a decimal value on the leds, for example: -b 85");
                        Serial.println("-l [DELAY in MS]: running lights with the velocity, for example: -l 300");
                        Serial.println("-t: measure temperature 1x and output to serial interface");
                        Serial.println("-d [DELAY in MS]: Measure temperature and output to LED (BCD encoded) and serial interface, for example: -d 500");
                        Serial.println("-p: stop the current application");

                        CurrentState = Waiting;
                        break;

                    case 'p':
                        Serial.println("Stopping ...");
                        Byte2Led(0);
                        CurrentState = Waiting;
                        break;

                    case 'b':
                        Serial.println("Set LED value ...");
                        if (value >= 0 && value <= 255) {
                            Byte2Led(value);
                        }
                        else {
                            Serial.println("invalid value");
                        }
                        CurrentState = Waiting;
                        break;

                    case 't':
                        CalculateTemp();
                        break;

                    case 'd':
                        Serial.println("starting measuring ...");
                        if (value >= 0) {
                            _nextInterval = _currentMillis;
                            _interval = value;
                            CurrentState = Measure;
                        }
                        else {
                            Serial.println("invalid value!");
                            CurrentState = Waiting;
                        }
                        break;
                }
            }
        }
    }

    // State-machine
    if (CurrentState != Waiting && _currentMillis >= _nextInterval) {
        _nextInterval = _currentMillis + _interval;
        float v;
        switch (CurrentState) {
            case Measure:
                v = CalculateTemp();
                ShowBcd(v);
                break;
            case RunningLed:
                LedRun();
                break;
            default:
                break;
        }
    }

    // Flashing of the LED on the motherboard
    if (_currentMillis >= _nextMainLedInterval)
    {
        _nextMainLedInterval += MAIN_LED_INTERVALL;
        if (digitalRead(MAIN_LED))
        {
            digitalWrite(MAIN_LED, 0);
        }
        else
        {
            digitalWrite(MAIN_LED, 1);
        }
    }
}

void LedRun(){
  Byte2Led(current);

  if (current >= 128 && direction == 0){
    direction = 1;
  }
  else if (current <= 1 && direction == 1){
      direction = 0;
  }

  if (direction == 0){
      current = current << 1;
  }
  else if (direction == 1){
      current >> 1;
  }
}

/*
* Read out the current ADC value,
* convert it to a voltage and then convert it to convert the current temperature and give it a look.
* The temperature can be set in this function on the serial interface be output
*/
float CalculateTemp(){
    float sensorValue = analogread(Temp);
    Serial.print("AnalogIn: ");
    Serial.print(sensorValue);
    float digitalTemp = (sensorValue * 5000)/1024;
    float digitalTemp = (digitalTemp - 600)/10;
    Serial.print("Current temperature: ");
    Serial.print(Temp);

    return Temp;
}

/*
 * Convert the current temperature to an integer value, then convert it to an integer value.
 * Convert BCD and output it to the LEDs.
 * If less than 0 then all should LED lights up.
 */
void ShowBcd(float value){
    int Temp = (int) Value;

    int TenDigit = (Temp / 10) << 4;
    int OneDigit = Temp % 10;
    int erg = TenDigit | OneDigit;

    //Byte2Led(0xFF);
    Byte2Led(erg);
}

/*
 * Auxiliary function for receiving data via the serial interface.
 */
int GetCommandValue(String command){
    if (command.length() < 2)
        return -2;
    String number = command.substring(2);
    number.trim();

    Serial.println(number);
    return number.toInt();
}

/*
 * Auxiliary function to display one byte (unsigned char) on the 8 LEDs.
 */
void Byte2Led(unsigned char byte){
    unsigned char consoleData[9];

    String output;
    for (int i = 0; i < 8; i++)
    {
        if (byte & 0x01)
        {
            digitalWrite(_ledArray[i], ON);
            output += "1";
        }
        else
        {
            digitalWrite(_ledArray[i], OFF);
            output += "0";
        }
        byte = byte >> 1;
    }

    Serial.print("LED: ");
    for (int i = 7; i >= 0; i--)
        Serial.print(output[i]);
    Serial.println("");
}