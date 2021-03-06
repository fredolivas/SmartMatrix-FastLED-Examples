// Funky Clouds Spiral
// Modified by Jason Coon for Smart Matrix:
// 
// Funky Clouds 2014
// Toys For Matrix Effects
// www.stefan-petrick.de/wordpress_beta
// http://pastebin.com/pFdQSsFD
// https://www.youtube.com/watch?v=0Ehj7sEwOy4

#include "SmartMatrix.h"
#include "FastLED.h"

#define HAS_IR_REMOTE 0

#if (HAS_IR_REMOTE == 1)

#include "IRremote.h"

#define IR_RECV_CS     18

// IR Raw Key Codes for SparkFun remote
#define IRCODE_HOME  0x10EFD827   

IRrecv irReceiver(IR_RECV_CS);

bool isOff = false;

#endif

SmartMatrix matrix;

const rgb24 COLOR_BLACK = { 0, 0, 0 };

// Matrix dimensions

const uint8_t WIDTH = 32;
const uint8_t HEIGHT = 32;

// LED stuff 
const int DEFAULT_BRIGHTNESS = 255;
#define NUM_LEDS (WIDTH * HEIGHT)
CRGB *leds;

// Timer stuff (Oszillators)

struct timer {
    unsigned long takt;
    unsigned long lastMillis;
    unsigned long count;
    int delta;
    byte up;
    byte down;
};
timer multiTimer[5];

int timers = sizeof(multiTimer) / sizeof(multiTimer[0]);

void setup() {
    Serial.begin(9600);

    // Initialize 32x32 LED Matrix
    Serial.print("Initializing matrix...");
    matrix.begin();
    Serial.println(" done");

    matrix.setBrightness(DEFAULT_BRIGHTNESS);
    matrix.setColorCorrection(cc24);

    // Clear screen
    matrix.fillScreen(COLOR_BLACK);
    matrix.swapBuffers();

#if (HAS_IR_REMOTE == 1)

    // Initialize IR receiver
    irReceiver.enableIRIn();

#endif

    // set all counting directions positive for the beginning
    for (int i = 0; i < timers; i++) multiTimer[i].delta = 1;

    // set range (up/down), speed (takt=ms between steps) and starting point of all oszillators

    multiTimer[0].takt = 42;     //x1
    multiTimer[0].up = WIDTH - 1;
    multiTimer[0].down = 0;
    multiTimer[0].count = 0;

    multiTimer[1].takt = 55;     //y1
    multiTimer[1].up = HEIGHT - 1;
    multiTimer[1].down = 0;
    multiTimer[1].count = 0;

    multiTimer[2].takt = 3;      //color
    multiTimer[2].up = 255;
    multiTimer[2].down = 0;
    multiTimer[2].count = 0;

    multiTimer[3].takt = 71;     //x2  
    multiTimer[3].up = WIDTH - 1;
    multiTimer[3].down = 0;
    multiTimer[3].count = 0;

    multiTimer[4].takt = 89;     //y2
    multiTimer[4].up = HEIGHT - 1;
    multiTimer[4].down = 0;
    multiTimer[4].count = 0;
    
    Serial.println("Setup done");
}

// translates from x, y into an index into the LED array
int XY(int x, int y) {
    if (y >= HEIGHT) { y = HEIGHT - 1; }
    if (y < 0) { y = 0; }
    if (x >= WIDTH) { x = WIDTH - 1; }
    if (x < 0) { x = 0; }

    return (y * WIDTH) + x;
}

// counts all variables with different speeds linear up and down
void UpdateTimers()
{
    unsigned long now = millis();
    for (int i = 0; i < timers; i++)
    {
        while (now - multiTimer[i].lastMillis >= multiTimer[i].takt)
        {
            multiTimer[i].lastMillis += multiTimer[i].takt;
            multiTimer[i].count = multiTimer[i].count + multiTimer[i].delta;
            if ((multiTimer[i].count == multiTimer[i].up) || (multiTimer[i].count == multiTimer[i].down))
            {
                multiTimer[i].delta = -multiTimer[i].delta;
            }
        }
    }
}

// fade the image buffer arround
// x, y: center   r: radius   dimm: fade down
void Spiral(int x, int y, int r, byte dimm) {
    for (int d = r; d >= 0; d--) {
        for (int i = x - d; i <= x + d; i++) {
            leds[XY(i, y - d)] += leds[XY(i + 1, y - d)];
            leds[XY(i, y - d)].nscale8(dimm);
        }
        for (int i = y - d; i <= y + d; i++) {
            leds[XY(x + d, i)] += leds[XY(x + d, i + 1)];
            leds[XY(x + d, i)].nscale8(dimm);
        }
        for (int i = x + d; i >= x - d; i--) {
            leds[XY(i, y + d)] += leds[XY(i - 1, y + d)];
            leds[XY(i, y + d)].nscale8(dimm);
        }
        for (int i = y + d; i >= y - d; i--) {
            leds[XY(x - d, i)] += leds[XY(x - d, i - 1)];
            leds[XY(x - d, i)].nscale8(dimm);
        }
    }
}

// Bresenham line
void Line(int x0, int y0, int x1, int y1, byte color)
{
    int dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
    int dy = -abs(y1 - y0), sy = y0<y1 ? 1 : -1;
    int err = dx + dy, e2;
    for (;;){
        leds[XY(x0, y0)] += CHSV(color, 255, 255);
        if (x0 == x1 && y0 == y1) break;
        e2 = 2 * err;
        if (e2 > dy) { err += dy; x0 += sx; }
        if (e2 < dx) { err += dx; y0 += sy; }
    }
}

void DimmAll(byte value)
{
    for (int i = 0; i < NUM_LEDS; i++)
    {
        leds[i].nscale8(value);
    }
}

#if (HAS_IR_REMOTE == 1)

unsigned long handleInput() {
    unsigned long input = 0;

    decode_results results;

    results.value = 0;

    // Attempt to read an IR code ?
    if (irReceiver.decode(&results)) {
        input = results.value;

        // Prepare to receive the next IR code
        irReceiver.resume();
    }

    if (input == IRCODE_HOME) {
        isOff = !isOff;

        if (isOff){
            matrix.fillScreen(COLOR_BLACK);
            matrix.swapBuffers();
        }

        return input;
    }

    return input;
}

#endif

void loop()
{
#if (HAS_IR_REMOTE == 1)
    handleInput();

    if (isOff) {
        delay(100);
        return;
    }
#endif
    
	leds = (CRGB*) matrix.backBuffer();

    // manage the Oszillators
    UpdateTimers();

    // draw just a line defined by 5 oszillators
    Line(
        multiTimer[3].count,  // x1
        multiTimer[4].count,  // y1
        multiTimer[0].count,  // x2
        multiTimer[1].count,  // y2
        multiTimer[2].count); // color

    // manipulate the screen buffer
    // with fixed parameters (could be oszillators too)
    // center x, y, radius, scale color down
    // --> affects always a square with an odd length
    Spiral(15, 15, 16, 128);

    // why not several times?!
    Spiral(16, 6, 6, 128);
    Spiral(10, 24, 10, 128);

    // increase the contrast
    DimmAll(250);

    // done.
    //FastLED.show();
    matrix.swapBuffers();
}
