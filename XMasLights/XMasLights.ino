#include <FastLED.h>

// For debugging, uncomment this line
//#define GOFAST

// LED OBJECTS
#define NUM_LEDS 300
#define DATA_PIN 3

#ifdef GOFAST
#define DELAY_TIME 2
#else
#define DELAY_TIME_SLOW 500
#define DELAY_TIME_MEDIUM 200
#define DELAY_TIME_FAST 30
#endif

enum LED_CHANGE_TYPE
{
	LCT_IMMEDIATE = 0,
	LCT_PERCENT = 1
};

CRGB leds[NUM_LEDS];
CRGB ledTargets[NUM_LEDS];
CRGB ledDeltas[NUM_LEDS];
LED_CHANGE_TYPE ledChangeTypes[NUM_LEDS];

// UTILITY OBJECTS
char debugString[1024];
int loop1, loop2, loop3, temp1, temp2, temp3;

// FOR ROUTINE-SPECIFIC STUFF
// SNOW & other radial stuff
int lightSpot = 0;
// ringCutoffs is the critical part. Starting with light 0, you need to identify the lights above
// one another in a straight line. If you light these all, you get a vertical line up one side of
// the tree. You need to use the Serial.available if statement at the start to experimentally
// figure out which light numbers those are and adjust the length and contents of the array
// to match YOUR tree. 
// Controls: Send '=' to the serial port to increment to the next light, '-' to go back a light
// You'll need to change your NUM_OF_RINGS to match the number of times your lights go around the tree.
#define NUM_RINGS 11
// Note that the first ring needs to start at 0!
int ringCutoffs[NUM_RINGS] = { 0, 54, 95, 135, 167, 196, 218, 240, 262, 281, 295 };
int ringRatios[NUM_RINGS - 1];
char cBuff;
int snowTemp;
int fireTemp;
float rfac, gfac, bfac;
#define DELAY_TIME_FIRE 60
float snowPct;
#define SNOW_PWR 120
#define SNOW_MELT_HEAT 1
#define SNOW_MELT_HOW_MANY_RINGS_HIGH 2
#define SNOW_FULL_CUTOFF 200
#define NUM_FLAKES_TO_FALL_PER_FRAME 4
#define DELAY_TIME_SNOW 160
int snowFlakes[NUM_LEDS];
int ring, led; // temp - for use in loops

void setup()
{
	Serial.begin(9600);
	delay(2000);
	// Add the LEDs so FastLED knows about them.
	FastLED.addLeds<WS2811, DATA_PIN>(leds, NUM_LEDS);
	FastLED.setBrightness(18);

	// The best way to really get a different random number seed every time, 
	// since Arduinos don't ship with radioactive particles...
	randomSeed(analogRead(0) && analogRead(1) && analogRead(2) && analogRead(3) && analogRead(4) && analogRead(5));
	fill_solid(leds, NUM_LEDS, CRGB::Red);
	FastLED.delay(250);
	fill_solid(leds, NUM_LEDS, CRGB::Purple);
	FastLED.delay(250);
	fill_solid(leds, NUM_LEDS, CRGB::Blue);
	FastLED.delay(250);
	fill_solid(leds, NUM_LEDS, CRGB::Green);
	FastLED.delay(250);

	// Initialization for routine-specific stuff
	for (led = 0; led < NUM_LEDS; led++) {
		snowFlakes[led] = 0;
	}
}

void loop()
{
	FastLED.clear();
	for (loop1 = 0; loop1 < 300; loop1++) {
		doFire();
	}

	doCandyCane(500);

	// Clear out the snow each time.
	for (loop1 = 0; loop1 < NUM_LEDS; loop1++) {
		snowFlakes[loop1] = 0;
	}
	for (loop1 = 0; loop1 < 300; loop1++) {
		doSnow();
	}

	redGreenFlicker(15);

	sparkle(CRGB::Gold, 50, 50);
	sparkle(CRGB::Navy, 250, 50);
	//FastLED.delay(100);
	colorRacer(CRGB(200, 50, 50)); // colorRacer can take a color you define (R, G, B)
	colorRacer(CRGB::White); // Or use a whole bunch of built-in colors
	colorRacer(CRGB::Blue);
	colorRacer(CRGB::Yellow);
	colorRacer(CRGB::Black);
	for (int i = 0; i < 10; i++) {
		colorChaseDown(CRGB::Red, 10, 20, 30);
		colorChaseUp(CRGB::Red, 10, 20, 30);
	}
	colorChaseDown(CRGB::Pink, 15, 45, 100);
	colorChaseUp(CRGB::Pink, 15, 45, 100);

}

/**********************************************
*
* doFire()
*    Where are Chet's nuts roasting?
*
**********************************************/
void doFire() {
	for (led = 0; led < ringCutoffs[1]; led ++) {
		leds[led] = CRGB(random8(55) + 200, random8(55) + 200, random(55) + 50);
	}
	for (ring = 1; ring < NUM_RINGS - 2; ring++) {
		for (led = ringCutoffs[ring]; led < ringCutoffs[ring + 1]; led++) {
			// Find the percentage aroung the current ring.
			fireTemp = (float)((float)(led - ringCutoffs[ring]) / (float)(ringCutoffs[ring + 1] - ringCutoffs[ring])) *
						   // Then multiply that by the number of leds on the ring below
						   (ringCutoffs[ring] - ringCutoffs[ring - 1]) +
					   // And add the base offset of the ring below
					   ringCutoffs[ring - 1] +
					   // And make it drift left, right, or straight down
					   (random8(3) - 1);
			rfac = ((random8(32) + 190) / 255.0f);
			gfac = ((random8(32) + 160) / 255.0f);
			bfac = ((random8(32) + 120) / 255.0f);
			leds[led] = CRGB(leds[fireTemp].r * rfac, 
				leds[fireTemp].g * gfac, 
				leds[fireTemp].b * bfac);
		}
	}
	FastLED.delay(DELAY_TIME_FIRE);
}

/**********************************************
*
* doCandyCaneSpin()
*
**********************************************/
void doCandyCane(int frames) {
	for (loop2 = frames; loop2 >= 0; loop2--)
	{
		for (loop1 = 0; loop1 < NUM_LEDS; loop1++) {
			if ((loop1 + loop2) % 20 > 10) {
				leds[loop1] = CRGB::Red;
			} else {
				leds[loop1] = CRGB::White;
			}
		}
		FastLED.delay(DELAY_TIME_FAST);
	}

	// Honestly I have no idea what this used to do...
	// // FastLED.delay(5000);
	// fill_solid(leds, NUM_LEDS, CRGB::Black);
	// int numOfLightsThisRing;
	// float tempSin;
	// for (loop1 = 0; loop1 < 250; loop1++) {
	// 	//Serial.println("In loop1");
	// 	//FastLED.delay(100);
	// 	for (ring = 0; ring < 1; ring++) {
	// 		//Serial.println("in Ring");
	// 		//Serial.println(ring);
	// 		//FastLED.delay(100);
	// 		numOfLightsThisRing = ringCutoffs[ring + 1] - ringCutoffs[ring];
	// 		for (led = 0; led < numOfLightsThisRing - 1; led++) {
	// 			leds[led] = CRGB(loop1, loop1, loop1);
	// 		}
	// 	}
	// 	leds[0] = CRGB::Purple;
	// 	FastLED.delay(DELAY_TIME_FAST);
	// }
}

/**********************************************
*
* doSnow()
*    Let it snow, let it snow!
*
**********************************************/
void doSnow() {
	//Serial.println("Let it snow!");
	// STEP 1 - SET UP YOUR TREE
	// THIS IS THE PART YOU USE THE FIRST TIME TO SET UP YOUR TREE
	// Uncomment this block (and comment the rest). Use '=' and '-' to
	// move your 'cursor' light around and experimentally get the 
	// values you need to put in the ringCutoffs array.
	//if (Serial.available() > 0) {
	//	cBuff = Serial.read();
	//	switch (cBuff) {
	//		case '=':
	//			lightSpot++;
	//			break;
	//		case '-':
	//			lightSpot--;
	//			break;
	//	}
	//	Serial.println(lightSpot);
	//}
	//fill_solid(leds, NUM_LEDS, CRGB::Black);
	//leds[lightSpot] = CRGB::White;
	//FastLED.delay(DELAY_TIME_FAST);

	// STEP 2 - DOUBLE-CHECK
	// When you're done filling in ringCutoffs, uncomment this block and check your line. See if it goes 
	// fairly straight up the tree.
	//fill_solid(leds, NUM_LEDS, CRGB::Black);
	//for (ringCounter = 0; ringCounter < NUM_OF_RINGS; ringCounter++) {
	//	leds[ringCutoffs[i]] = CRGB::White;
	//}
	//FastLED.delay(DELAY_TIME_FAST);

	// STEP 3 - ENJOY THE SNOW
	// Uncomment this code block and it will do the rest!
	// Algorithm:
	// Generate a random flake
	// Start at the bottom ring, and float them all down

	// Start at the bototm and go up.
	for (ring = 0; ring < NUM_RINGS - 1; ring++) {
		// Iterate over the LEDs in the ring.
		for (led = ringCutoffs[ring]; led < ringCutoffs[ring + 1]; led++) {
			// If there's a snowflake there
			if (snowFlakes[led] > 0) {
				// And we're not on the bottom ring
				if (ring > 0) {
					// Try to find a place for this snowflake to fall. 
					// First find the current LEDs percentage around its own ring
					snowTemp = ((float)((float)(led - ringCutoffs[ring]) / (float)(ringCutoffs[ring + 1] - ringCutoffs[ring])) *
						// Then multiply that by the number of leds on the ring below
						(ringCutoffs[ring] - ringCutoffs[ring - 1])) +
						// And add the base offset of the ring below
						ringCutoffs[ring - 1] +
						// And make it drift left, right, or straight down
						(random8(3) - 1);
					if (snowFlakes[snowTemp] < SNOW_FULL_CUTOFF) {
						//Serial.print("Going from ");
						//Serial.print(led);
						//Serial.print(" to ");
						//Serial.println(snowTemp);
						snowFlakes[snowTemp] = max (snowFlakes[led], min (250, snowFlakes[snowTemp] + SNOW_PWR));
						//Serial.print("Snow ");
						//Serial.print(snowTemp);
						//Serial.print(" is now ");
						//Serial.print(snowFlakes[snowTemp]);
						//Serial.print(", and sf[");
						//Serial.print(led);
						//Serial.print("] was ");
						//Serial.println(snowFlakes[led]);
						snowFlakes[led] = 0;
					}
					//Serial.print("ON: ");
				}
			}
			//Serial.println(led);
		}
	}
	for (led = ringCutoffs[NUM_RINGS - 1]; led < NUM_LEDS; led++) {
		if (snowFlakes[led] > 0) {
			snowTemp = ((float)((float)(led - ringCutoffs[NUM_RINGS - 1]) / (float)(NUM_LEDS - ringCutoffs[NUM_RINGS - 1])) * 
				(ringCutoffs[NUM_RINGS - 1] - ringCutoffs[NUM_RINGS - 2])) + 
				ringCutoffs[NUM_RINGS - 2] + 
				(random8(3) - 1);
			//Serial.println(led - ringCutoffs[NUM_RINGS - 1]);
			//Serial.println(NUM_LEDS - ringCutoffs[NUM_RINGS - 1]);
			//Serial.println((float)((float)(led - ringCutoffs[NUM_RINGS - 1]) / (float)(NUM_LEDS - ringCutoffs[NUM_RINGS - 1])));
			//Serial.println(ringCutoffs[NUM_RINGS - 1] - ringCutoffs[NUM_RINGS - 2]);
			//Serial.println((float)((float)(led - ringCutoffs[NUM_RINGS - 1]) / (float)(NUM_LEDS - ringCutoffs[NUM_RINGS - 1])) * (ringCutoffs[NUM_RINGS - 1] - ringCutoffs[NUM_RINGS - 2]));
			//Serial.println(((float)((float)(led - ringCutoffs[NUM_RINGS - 1]) / (float)(NUM_LEDS - ringCutoffs[NUM_RINGS - 1])) * (ringCutoffs[NUM_RINGS - 1] - ringCutoffs[NUM_RINGS - 2])) + ringCutoffs[NUM_RINGS - 2]);
			if (snowFlakes[snowTemp] < SNOW_FULL_CUTOFF) {
				//Serial.print("Going from ");
				//Serial.print(led);
				//Serial.print(" to ");
				//Serial.println(snowTemp);
				snowFlakes[snowTemp] = min(250, snowFlakes[snowTemp] + SNOW_PWR);
				snowFlakes[led] = 0;
			}
			//Serial.print("ON: ");
		}
			//Serial.println(led);
	}
	for (led = 0; led < ringCutoffs[SNOW_MELT_HOW_MANY_RINGS_HIGH]; led++) {
		snowFlakes[led] = max(0, snowFlakes[led] - SNOW_MELT_HEAT);
	}
	for (snowTemp = 0; snowTemp < NUM_FLAKES_TO_FALL_PER_FRAME; snowTemp++) {
		led = ringCutoffs[NUM_RINGS - 1] + (random8((NUM_LEDS - ringCutoffs[NUM_RINGS - 1])));
		snowFlakes[led] = min (250, snowFlakes[led] + SNOW_PWR);
	}
	//fill_solid(leds, NUM_LEDS, CRGB::Black);
	for (led = 0; led < NUM_LEDS; led++) {
		//if (snowFlakes[led] > 0) {
			leds[led] = CRGB(snowFlakes[led], snowFlakes[led], snowFlakes[led]);
		//}
	}
	// leds[0] = CRGB::Yellow;
	FastLED.delay(DELAY_TIME_SNOW);
}

/**********************************************
*
* Update()
*    This is where all the LEDs are "rendered", using whatever rules are set up for them.
*
**********************************************/
void update() {
	for (loop1 = 0; loop1 < NUM_LEDS; loop1++) {
		switch (ledChangeTypes[loop1]) {
		case LCT_IMMEDIATE:
			leds[loop1] = ledTargets[loop1];
			break;
		}
	}
}

/**********************************************
*
* Sparkle
*    Take a base color and flicker it, like shiny garland
*    Note - flickerOdds (2nd parameter) is a number that says how many chances in 1000 each light has each frame to sparkle
*
**********************************************/
void sparkle(CRGB baseColor, int flickerOdds, int frames) {
	for (loop2 = 0; loop2 < frames; loop2++) {
		for (loop1 = 0; loop1 < NUM_LEDS; loop1++) {
			if (rand() % 1000 < flickerOdds) {
				leds[loop1] = CRGB(avg8(baseColor.r, 255), avg8(baseColor.g, 255), avg8(baseColor.b, 255));
			}
			else {
				leds[loop1] = baseColor;
			}
		}
		FastLED.delay(DELAY_TIME_SLOW);
	}
}


/**********************************************
*
* ColorChaseDown
*    Race a couple of dashes down the tree
*
**********************************************/
void colorChaseDown(CRGB theColor, int onsize, int offsize, int frames) {
	FastLED.clear();
	temp1 = 0;
	temp2 = onsize + offsize;
	for (loop2 = 0; loop2 < frames; loop2++) {
		for (loop1 = 0; loop1 < NUM_LEDS; loop1++) {
			if ((loop1 + temp1) % temp2 < onsize) {
				leds[loop1] = theColor;
			}
			else {
				leds[loop1] = CRGB::Black;
			}
		}
		FastLED.delay(DELAY_TIME_FAST);
		temp1++;
	}
}

/**********************************************
*
* ColorChaseUp
*    Race a couple of dashes up the tree
*
**********************************************/
void colorChaseUp(CRGB theColor, int onsize, int offsize, int frames) {
	FastLED.clear();
	temp1 = NUM_LEDS * 30;
	temp2 = onsize + offsize;
	for (loop2 = 0; loop2 < frames; loop2++) {
		for (loop1 = 0; loop1 < NUM_LEDS; loop1++) {
			if ((loop1 + temp1) % temp2 < onsize) {
				leds[loop1] = theColor;
			}
			else {
				leds[loop1] = CRGB::Black;
			}
		}
		FastLED.delay(DELAY_TIME_FAST);
		temp1--;
	}
}

/**********************************************
*
* Red Green Flicker
*    Flicker between red/green alternating lights
*
**********************************************/
void redGreenFlicker(int numberOfTimesToFlicker) {
	Serial.println("RG Flickering");
	for (loop2 = 0; loop2 < numberOfTimesToFlicker; loop2++) {
		for (loop1 = 0; loop1 < NUM_LEDS; loop1++) {
			if (loop1 % 4 == 0) {
				leds[loop1] = CRGB::Red;
			}
			else if (loop1 % 4 == 2)
				leds[loop1] = CRGB::Green;
		}
		FastLED.show();
		FastLED.delay(DELAY_TIME_SLOW);
		FastLED.clear();
		for (loop1 = 0; loop1 < NUM_LEDS; loop1++) {
			if (loop1 % 4 == 1) {
				leds[loop1] = CRGB::Green;
			}
			else if (loop1 % 4 == 3)
				leds[loop1] = CRGB::Red;
		}
		FastLED.show();
		FastLED.delay(DELAY_TIME_SLOW);
		FastLED.clear();
	}
}

/**********************************************
*
* Color Racer
*    Race a color up the string
*
**********************************************/
void colorRacer(CRGB colorToRace) {
	Serial.println("Racing");
	for (loop1 = 0; loop1 < NUM_LEDS; loop1++) {
			leds[loop1] = colorToRace;
			FastLED.show();
			FastLED.delay(DELAY_TIME_FAST);
		}
}