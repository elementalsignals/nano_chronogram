//#include <ss_oled.h>
#include <TimerOne.h>
#include <EncoderButton.h>

//SSOLED ssoled;
#define SDA_PIN             12
#define SCL_PIN             11
#define RESET_PIN           -1
#define OLED_ADDR           0x3C
#define FLIP180             0
#define INVERT              0
#define USE_HW_I2C          0
#define PIN_ENCODER_CLK     8
#define PIN_ENCODER_DT      9
#define PIN_ENCODER_SWITCH  10
#define PIN_KNOB_FREQ       20
#define PIN_KNOB_0          14
#define NUM_DIVISORS        12

#define DISPLAY_RATE_MS 10

#define TRIGGER_DURATION 5000 //microseconds

volatile int counts[5] =         { 0,  0,  0,  0,  0};
volatile byte divisors[5] =      { 1,  1,  1,  1,  1};
volatile bool trigger_flags[5] = { 0,  0,  0,  0,  0};

const byte divPins[5] = {14, 15, 16, 17, 18};
const byte outPins[5] = { 2,  3,  4,  5,  6};

volatile uint32_t ctr_0 = 0;
uint32_t base_period = 5000;          //0.5s
float base_period_reciprocal = 0;
//uint8_t divisor[NUM_DIVISORS] = {16, 32, 64, 128, , 6, 7, 8, 9, 10, 11, 12};
uint8_t idx_div = 0;
double trigger_duration_counts = 1;

EncoderButton encoder(PIN_ENCODER_DT, PIN_ENCODER_CLK, PIN_ENCODER_SWITCH);









void setup()
{
    //Serial.begin(9600);
    // int rc;
    // rc = oledInit(&ssoled, OLED_128x32, OLED_ADDR, FLIP180, INVERT, USE_HW_I2C, SDA_PIN, SCL_PIN, RESET_PIN, 400000L);

    pinMode(PIN_ENCODER_SWITCH, INPUT);
    pinMode(PIN_ENCODER_DT, INPUT);
    pinMode(PIN_ENCODER_CLK, INPUT);

    for (byte i = 0; i < 5; i++){
        pinMode(divPins[i], INPUT);
        pinMode(outPins[i], OUTPUT);
    }

    Timer1.initialize(base_period);
    Timer1.attachInterrupt(interruptHandler);

    encoder.setEncoderHandler(onRotate);
    // encoder.setClickHandler(onClick);
    // encoder.setLongPressHandler(onLongPress);
    // encoder.setTripleClickHandler(onTripleClick);

    // oledFill(&ssoled, 0, 1);


}











void onRotate(EncoderButton& encoder){

    if (encoder.increment() > 0){
        base_period -= encoder.increment() * encoder.increment() * 16; //subtract when clockwise since decreasing the base period makes the clock faster
        encoder.resetPosition();
    } else if (encoder.increment() < 0){
        base_period += encoder.increment() * encoder.increment() * 16;
        encoder.resetPosition();
    }
    
    //Serial.print("base_period = "); Serial.println(base_period);

    //i_tempo = (60.0f / base_period) * 10000; //get BPM from base_period

    //itoa(i_tempo, tempo, 10); //convert BPM to char array, ss_oled library won't print anything else

    Timer1.setPeriod(base_period);

    //oledFill(&ssoled, 0, 1);
    // oledWriteString(&ssoled, 0, 16, 0, tempo, FONT_STRETCHED, 0, 1);

}











void loop(){

    encoder.update();
    // base_period = map(analogRead(PIN_KNOB_FREQ), 0, 1024, 100, TRIGGER_DURATION); //base period should not exceed trigger duration since we need to count at least one period to turn the trigger off


    // for (int i = 0; i < 5; i++){
    //     divisors[i] = map(analogRead(divPins[i]), 0, 1024, 0, NUM_DIVISORS);
    // }

    trigger_duration_counts = TRIGGER_DURATION / base_period;
}









void interruptHandler(){

    //to keep everything in sync, ALL possible clock divisions need to start at once, and run in parallel, internally, forever.
    //we'll call these always-running parallel clocks 'lanes'. the div knobs just route the state of the chosen lane to their output pin.
    //something funny going on with D4 output

    for (int i = 0; i < 5; i++){
        divisors[i] = map(analogRead(divPins[i]), 0, 1024, 0, NUM_DIVISORS);
    }

    for (int i = 0; i < 5; i++){
        ++counts[i];
        if (counts[i] > ((1 + divisors[i]) * divisors[i])){
            bitSet(PORTD, outPins[i]);
            counts[i] = 0;
        }

        trigger_flags[i] = bitRead(PORTD, outPins[i]);

        if (trigger_flags[i] && counts[i] >= trigger_duration_counts){
            bitClear(PORTD, outPins[i]);
        }
    }
}

