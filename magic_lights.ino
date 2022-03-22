// readPin is reading from the middle of a voltage divider, with a 10K pull down resistor, and a pull-up resistor
// made from an open circuit (made out of conductive paint, in a fun lightbulb shape). When you place your hand on the lightbulb
// you complete the circuit, lowering the pull-up resistance (theoretically from Infinity, but practically it is much smaller), 
// and thus increasing the measured voltage.

const int readPin = A5;

// relayPin is outputting to the control pin of a relay box
const int relayPin = 8;


boolean holdingButton = false;
boolean lightState = false;

// If we detect a hand (i.e. somebody is holding the 'button'), we toggle the light
// and lock the system until we stop detecting a hand
boolean lock = false;

// Configured by experiment - the difference in reading between baseline and short-term averages required to 
// trigger the presence and absence, respectively, of a hand
const int upThreshold = 25;
const int downThreshold = 10;


bool acquiringBaseline = true;
int baselineLoops = 30;

// To mitigate noise, calculate two exponential moving averages: a slower moving one that gives a baseline average reading,
// and a fast moving one that captures the increased voltage due to application of a hand.
float ema = 0;
float emaMult = 0.06;

float baseEma = 0;
float baseEmaMult = 0.01;

int loopCounter = 0;

void setup() {
  pinMode(relayPin, OUTPUT);
  pinMode(readPin, INPUT);
  Serial.begin(9600);
}


void loop()
{
  loopCounter++;
  
  delay(30);
  float directRead = analogRead(readPin);
  float pinVoltage  = abs(directRead * 5 * 1000 / 1024);
  
  ema = (emaMult * pinVoltage) + (ema * (1 - emaMult));
  baseEma = (baseEmaMult * pinVoltage) + (baseEma * (1 - baseEmaMult));
  float diff = ema - baseEma;

  if (loopCounter % 10 == 0) {
    Serial.print("Baseline EMA: " + String(baseEma) + "mV ----- ");
    Serial.print("Current EMA: " + String(ema) + "mV ------ ");
    Serial.println("Difference: " + String(diff) + "mV");
  }

  if (loopCounter > baselineLoops) acquiringBaseline = false;

  if (!acquiringBaseline) {
    holdingButton = (diff > upThreshold);
    
    if (holdingButton && !lock) {
      lock = true;
      lightState = !lightState;
      digitalWrite(relayPin, lightState);
    } 
    if (diff < downThreshold) lock = false;
  }
}
