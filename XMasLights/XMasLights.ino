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

CRGB leds[NUM_LEDS];

// UTILITY OBJECTS
char debugString[1024];
int loop1, loop2, loop3, temp1, temp2, temp3, patternloop;
float ftemp1;
CRGB ctemp1;
bool btemp;

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
#define NUM_RINGS 12
// Note that the first ring needs to start at 0!
int ringCutoffs[NUM_RINGS] = {0, 54, 95, 135, 167, 196, 218, 240, 262, 281, 295, NUM_LEDS};
char cBuff;

int snowTemp;
float snowPct;
#define SNOW_PWR 120
#define SNOW_MELT_HEAT 1
#define SNOW_MELT_HOW_MANY_RINGS_HIGH 2
#define SNOW_FULL_CUTOFF 200
#define NUM_FLAKES_TO_FALL_PER_FRAME 4
#define DELAY_TIME_SNOW 160
byte snowFlakes[NUM_LEDS];

int fireTemp;
#define DELAY_TIME_FIRE 90

int ring, led; // temp - for use in loops

enum PATTERN
{
	FIRE,
	COLORRACE,
	RINGBOUNCE,
	SNOW,
	CANDYCANE,
	RAINBOW,
	SPARKLE,
	CHASEDOWN,
	CHASEUP,
	REDGREEN,
	OLDSCHOOL,
	NUM_OF_PATTERNS // must always be last
};
int patterns[NUM_OF_PATTERNS];
// Each pattern runs for about 3 seconds * DURATION_SCALE.
// If we add more LEDs, the duration will go up.
#define DURATION_SCALE 10

void setup()
{
	Serial.begin(9600);
	delay(2000);
	// Add the LEDs so FastLED knows about them.
	FastLED.addLeds<WS2811, DATA_PIN>(leds, NUM_LEDS);

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

	CreateNewPatternList(patterns);
}

void loop()
{
	shuffle(patterns, 5);
	for (patternloop = 0; patternloop < NUM_OF_PATTERNS; patternloop++)
	{
		fill_solid(leds, NUM_LEDS, CRGB::Black);
		FastLED.clear();
		switch (patterns[patternloop])
		{
		case RINGBOUNCE:
			FastLED.setBrightness(255);
			doRingBounce(80 * DURATION_SCALE);
			break;
		case FIRE:
			FastLED.setBrightness(40);
			doFire(25 * DURATION_SCALE);
			break;
		case RAINBOW:
			FastLED.setBrightness(28);
			doRainbow(200 * DURATION_SCALE);
			break;
		case CANDYCANE:
			FastLED.setBrightness(20);
			doCandyCane(80 * DURATION_SCALE);
			break;
		case SNOW:
			FastLED.setBrightness(80);
			doSnow(20 * DURATION_SCALE);
			break;
		case SPARKLE:
			FastLED.setBrightness(32);
			doSparkle(CHSV(random8(), 255, 255), 2, 70 * DURATION_SCALE);
			break;
		case CHASEDOWN:
			FastLED.setBrightness(48);
			temp1 = (random8() % 20) + 10;
			doChaseDown(CHSV(random8(), 255, 255), temp1, temp1, 80 * DURATION_SCALE);
			break;
		case CHASEUP:
			FastLED.setBrightness(48);
			temp1 = (random8() % 20) + 10;
			doChaseUp(CHSV(random8(), 255, 255), temp1, temp1, 80 * DURATION_SCALE);
			break;
		case REDGREEN:
			FastLED.setBrightness(64);
			doRedGreenFlicker(4 * DURATION_SCALE);
			break;
		case COLORRACE:
			FastLED.setBrightness(32);
			// This defines one 'unit' of time, since it's a function of NUM_LEDS.
			temp1 = DURATION_SCALE;
			for (temp2 = 0; temp2 < temp1; temp2++)
			{
				doColorRacer(CHSV(random8(), 255, 255));
			}
			break;
		case OLDSCHOOL:
			FastLED.setBrightness(32);
			doOldSchool(6 * DURATION_SCALE);
			break;
		}
	}
}

/**********************************************
*
* doRingBounce()
*    A bouncing ring.
*
**********************************************/
void doRingBounce(int frames)
{
	ftemp1 = 0;
	// btemp is set to true if we've already changed the color this bounce
	btemp = false;
	for (loop1 = 0; loop1 < frames; loop1++)
	{
		temp2 = abs(sin(ftemp1)) * (NUM_RINGS - 1);
		if (!btemp)
		{
			if (!temp2)
			{
				ctemp1 = CHSV(random8(), 255, 255);
				btemp = true;
			}
		}
		if (temp2)
		{
			btemp = false;
		}
		FastLED.clear();
		for (led = ringCutoffs[temp2]; led < ringCutoffs[temp2 + 1]; led++)
		{
			leds[led] = ctemp1;
		}
		FastLED.show();
		ftemp1 += 0.065;
		FastLED.delay(DELAY_TIME_FAST);
	}
}

/**********************************************
*
* doRainbow()
*    A fast rainbow chase down the tree.
*
**********************************************/
void doRainbow(int frames)
{
	for (loop1 = 0; loop1 < frames; loop1++)
	{
		for (led = 0; led < NUM_LEDS; led++)
		{
			leds[led] = CHSV((led + loop1) % 255, 255, 255);
		}
		FastLED.show();
	}
}

/**********************************************
*
* doFire()
*    Where are Chet's nuts roasting?
*
**********************************************/
void doFire(int frames)
{
#define FIRE_HORIZONTAL_HEAT_LOSS 0.95
float rfac, gfac, bfac;
	for (loop1 = 0; loop1 < frames; loop1++)
	{
		// Fill the first ring with high-variance sparks to fuel the flames.
		// First we diffuse the current flames. Handle the outer ones first.
		leds[0] = CRGB(avg8(leds[0].r, leds[1].r) * FIRE_HORIZONTAL_HEAT_LOSS,
					   avg8(leds[0].g, leds[1].g) * FIRE_HORIZONTAL_HEAT_LOSS,
					   avg8(leds[0].b, leds[1].b) * FIRE_HORIZONTAL_HEAT_LOSS);
		leds[ringCutoffs[1] - 1] = CRGB(avg8(leds[ringCutoffs[1] - 1].r, leds[ringCutoffs[1] - 2].r) * FIRE_HORIZONTAL_HEAT_LOSS,
										avg8(leds[ringCutoffs[1] - 1].g, leds[ringCutoffs[1] - 2].g) * FIRE_HORIZONTAL_HEAT_LOSS,
										avg8(leds[ringCutoffs[1] - 1].b, leds[ringCutoffs[1] - 2].b) * FIRE_HORIZONTAL_HEAT_LOSS);
		// Then iterate over the middle of the ring
		for (led = 1; led < ringCutoffs[1] - 2; led++)
		{
			leds[led] = CRGB(((leds[led].r / 3) + (leds[led - 1].r / 3) + (leds[led + 1].r / 3)) * FIRE_HORIZONTAL_HEAT_LOSS,
							 ((leds[led].g / 3) + (leds[led - 1].g / 3) + (leds[led + 1].g / 3)) * FIRE_HORIZONTAL_HEAT_LOSS,
							 ((leds[led].b / 3) + (leds[led - 1].b / 3) + (leds[led + 1].b / 3)) * FIRE_HORIZONTAL_HEAT_LOSS);
		}
		// Then we randomly spark some of the LEDs to very high brightness.
		for (led = 0; led < ringCutoffs[1]; led++)
		{
			if (random8() < 25)
			{
				leds[led] = CRGB(random8(55) + 200, random8(55) + 150, random(55) + 50);
			}
		}
		for (ring = 1; ring < NUM_RINGS - 2; ring++)
		{
			for (led = ringCutoffs[ring]; led < ringCutoffs[ring + 1]; led++)
			{
				// Find the percentage aroung the current ring.
				fireTemp = (float)((float)(led - ringCutoffs[ring]) / (float)(ringCutoffs[ring + 1] - ringCutoffs[ring])) *
							   // Then multiply that by the number of leds on the ring below
							   (ringCutoffs[ring] - ringCutoffs[ring - 1]) +
						   // And add the base offset of the ring below
						   ringCutoffs[ring - 1] +
						   // And make it drift left, right, or straight down
						   (random8(3) - 1);
				rfac = ((random8(32) + 175) / 255.0f);
				gfac = ((random8(32) + 150) / 255.0f);
				bfac = ((random8(32) + 90) / 255.0f);
				leds[led] = CRGB(leds[fireTemp].r * rfac,
								 leds[fireTemp].g * gfac,
								 leds[fireTemp].b * bfac);
			}
		}
		FastLED.delay(DELAY_TIME_FIRE);
	}
}

/**********************************************
*
* doCandyCaneSpin()
*
**********************************************/
void doCandyCane(int frames)
{
	for (loop1 = frames; loop1 >= 0; loop1--)
	{
		for (loop2 = 0; loop2 < NUM_RINGS - 2; loop2++)
		{
			temp1 = ringCutoffs[loop2];		// start of current ring
			temp2 = ringCutoffs[loop2 + 1]; // end of current ring / start of next ring
			temp3 = temp2 - temp1;			// 1/2 of current ring
			for (loop3 = 0; loop3 < temp3; loop3++)
			{
				if ((loop1 + loop3) % (temp3 / 2) > (temp3 / 4))
				{
					leds[temp1 + loop3] = CRGB::Red;
				}
				else
				{
					leds[temp1 + loop3] = CRGB::White;
				}
			}
		}
		FastLED.delay(DELAY_TIME_FAST);
	}
}

/**********************************************
*
* doSnow()
*    Let it snow, let it snow!
*
**********************************************/
void doSnow(int frames)
{
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
	for (loop1 = 0; loop1 < frames; loop1++)
	{
		for (ring = 0; ring < NUM_RINGS - 2; ring++)
		{
			// Iterate over the LEDs in the ring.
			for (led = ringCutoffs[ring]; led < ringCutoffs[ring + 1]; led++)
			{
				// If there's a snowflake there
				if (snowFlakes[led] > 0)
				{
					// And we're not on the bottom ring
					if (ring > 0)
					{
						// Try to find a place for this snowflake to fall.
						// First find the current LEDs percentage around its own ring
						snowTemp = ((float)((float)(led - ringCutoffs[ring]) / (float)(ringCutoffs[ring + 1] - ringCutoffs[ring])) *
									// Then multiply that by the number of leds on the ring below
									(ringCutoffs[ring] - ringCutoffs[ring - 1])) +
								   // And add the base offset of the ring below
								   ringCutoffs[ring - 1] +
								   // And make it drift left, right, or straight down
								   (random8(3) - 1);
						if (snowFlakes[snowTemp] < SNOW_FULL_CUTOFF)
						{
							snowFlakes[snowTemp] = max(snowFlakes[led], min(250, snowFlakes[snowTemp] + SNOW_PWR));
							snowFlakes[led] = 0;
						}
					}
				}
			}
		}
		for (led = ringCutoffs[NUM_RINGS - 2]; led < NUM_LEDS; led++)
		{
			if (snowFlakes[led] > 0)
			{
				snowTemp = ((float)((float)(led - ringCutoffs[NUM_RINGS - 2]) / (float)(NUM_LEDS - ringCutoffs[NUM_RINGS - 2])) *
							(ringCutoffs[NUM_RINGS - 2] - ringCutoffs[NUM_RINGS - 3])) +
						   ringCutoffs[NUM_RINGS - 3] +
						   (random8(3) - 1);
				if (snowFlakes[snowTemp] < SNOW_FULL_CUTOFF)
				{
					snowFlakes[snowTemp] = min(250, snowFlakes[snowTemp] + SNOW_PWR);
					snowFlakes[led] = 0;
				}
			}
		}
		for (led = 0; led < ringCutoffs[SNOW_MELT_HOW_MANY_RINGS_HIGH]; led++)
		{
			snowFlakes[led] = max(0, snowFlakes[led] - SNOW_MELT_HEAT);
		}
		for (snowTemp = 0; snowTemp < NUM_FLAKES_TO_FALL_PER_FRAME; snowTemp++)
		{
			led = ringCutoffs[NUM_RINGS - 2] + (random8((NUM_LEDS - ringCutoffs[NUM_RINGS - 2])));
			snowFlakes[led] = min(250, snowFlakes[led] + SNOW_PWR);
		}
		for (led = 0; led < NUM_LEDS; led++)
		{
			leds[led] = CRGB(snowFlakes[led], snowFlakes[led], snowFlakes[led]);
		}
		FastLED.delay(DELAY_TIME_SNOW);
	}
}

/**********************************************
*
* doSparkle
*    Take a base color and flicker it, like shiny garland
*    Note - flickerOdds (2nd parameter) is a number that says how many chances in 1000 each light has each frame to sparkle
*
**********************************************/
void doSparkle(CRGB baseColor, int flickerOdds, int frames)
{
	for (loop2 = 0; loop2 < frames; loop2++)
	{
		for (loop1 = 0; loop1 < NUM_LEDS; loop1++)
		{
			if (rand() % 1000 < flickerOdds)
			{
				leds[loop1] = CRGB::White;
			}
			else
			{
				leds[loop1] = baseColor;
			}
		}
		FastLED.delay(DELAY_TIME_FAST);
	}
}

/**********************************************
*
* doChaseDown
*    Race a couple of dashes down the tree
*
**********************************************/
void doChaseDown(CRGB theColor, int onsize, int offsize, int frames)
{
	FastLED.clear();
	temp1 = 0;
	temp2 = onsize + offsize;
	for (loop2 = 0; loop2 < frames; loop2++)
	{
		for (loop1 = 0; loop1 < NUM_LEDS; loop1++)
		{
			if ((loop1 + temp1) % temp2 < onsize)
			{
				leds[loop1] = theColor;
			}
			else
			{
				leds[loop1] = CRGB::Black;
			}
		}
		FastLED.delay(DELAY_TIME_FAST);
		temp1++;
	}
}

/**********************************************
*
* doChaseUp
*    Race a couple of dashes up the tree
*
**********************************************/
void doChaseUp(CRGB theColor, int onsize, int offsize, int frames)
{
	FastLED.clear();
	temp1 = NUM_LEDS * 30;
	temp2 = onsize + offsize;
	for (loop2 = 0; loop2 < frames; loop2++)
	{
		for (loop1 = 0; loop1 < NUM_LEDS; loop1++)
		{
			if ((loop1 + temp1) % temp2 < onsize)
			{
				leds[loop1] = theColor;
			}
			else
			{
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
void doRedGreenFlicker(int numberOfTimesToFlicker)
{
	Serial.println("RG Flickering");
	for (loop2 = 0; loop2 < numberOfTimesToFlicker; loop2++)
	{
		for (loop1 = 0; loop1 < NUM_LEDS; loop1++)
		{
			if (loop1 % 4 == 0)
			{
				leds[loop1] = CRGB::Red;
			}
			else if (loop1 % 4 == 2)
				leds[loop1] = CRGB::Green;
		}
		FastLED.show();
		FastLED.delay(DELAY_TIME_SLOW);
		FastLED.clear();
		for (loop1 = 0; loop1 < NUM_LEDS; loop1++)
		{
			if (loop1 % 4 == 1)
			{
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
void doColorRacer(CRGB colorToRace)
{
	Serial.println("Racing");
	for (loop1 = 0; loop1 < NUM_LEDS; loop1++)
	{
		leds[loop1] = colorToRace;
		FastLED.show();
	}
}

/**********************************************
*
* Old School
*    Good old fashioned, sparkley colors.
*
**********************************************/
void doOldSchool(int frames)
{
	CHSV temphsv;
	for (loop1 = 0; loop1 < frames; loop1++)
	{
		for (loop2 = 0; loop2 < NUM_LEDS; loop2++)
		{
			// Default to very colorful...
			leds[loop2] = CHSV(random8(), 255, 255);
			// Every once in a while, make one white
			if (random8() < 5)
			{
				leds[loop2] = CRGB::White;
			}
		}
		FastLED.show();
		FastLED.delay(DELAY_TIME_SLOW);
	}
}

/**********************************************
*
* UTIL: Shuffle
*    Shuffles an array of ints, which are used to determine which pattern to play.
*
**********************************************/
void shuffle(int *array, size_t n)
{
	for (loop1 = 0; loop1 < n - 1; loop1++)
	{
		temp1 = rand() % NUM_OF_PATTERNS;
		temp2 = rand() % NUM_OF_PATTERNS;
		temp3 = array[temp1];
		array[temp1] = array[temp2];
		array[temp2] = temp3;
	}
}

/**********************************************
*
* UTIL: CreateNewPatternList
*    Create an array of ints we'll use (after shuffling) to decide which pattern to play.
*
**********************************************/
void CreateNewPatternList(int *array)
{
	for (loop1 = 0; loop1 < NUM_OF_PATTERNS; loop1++)
	{
		array[loop1] = loop1;
	}
}
