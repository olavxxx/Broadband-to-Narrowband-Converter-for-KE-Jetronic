#include <Arduino.h>
#include <ArduinoGraphics.h>
#include <Arduino_LED_Matrix.h>

// --- Configuration values ---
const float TARGET_AFR = 14.3;      // Here you can target 14.3, 14.1, etc. (lower value => richer mixture)
const float STOICH_AFR = 14.7;      // Standard for petrol
const float SLOPE_STEEPNESS = 5.0;  // How steep slope the narrowband simulation should have (higher = more binary-like output)
// ---------------------------

const int aemInputPin = A1;   
const int kjetOutputPin = A0; 

ArduinoLEDMatrix matrix;
float targetLambda; // Calculated from TARGET_AFR and STOICH_AFR in setup()

// (Frame-data and plotVoltageFrame)
byte frame[8][12] = {
  { 0,0,0,0,0,0,0,0,0,0,0,0 }, { 0,0,0,0,0,0,0,0,0,0,0,0 },
  { 0,0,0,0,0,0,0,0,0,0,0,0 }, { 0,0,0,0,0,0,0,0,0,0,0,0 },
  { 0,0,0,0,0,0,0,0,0,0,0,0 }, { 0,0,0,0,0,0,0,0,0,0,0,0 }, 
  { 0,0,0,0,0,0,0,0,0,0,0,0 }, { 1,1,1,1,1,1,1,1,1,1,1,1 }
};

void plotVoltageFrame(float voltage) {
  
  int pixelRow = 7 - round((voltage - 0.1) / 0.8 * 7.0);
  pixelRow = constrain(pixelRow, 0, 7);
  static int lastPixelRow = -1;
  
  for (int row = 0; row < 8; row++) {
    for (int col = 0; col < 11; col++) { frame[row][col] = frame[row][col + 1]; }
  }
  
  int minRow = (lastPixelRow == -1) ? pixelRow : min(lastPixelRow, pixelRow);
  int maxRow = (lastPixelRow == -1) ? pixelRow : max(lastPixelRow, pixelRow);
  
  for (int row = 0; row < 8; row++) { frame[row][11] = (row >= minRow && row <= maxRow) ? 1 : 0; }
  
  lastPixelRow = pixelRow;
  
  matrix.renderBitmap(frame, 8, 12);
}

void setup() {
  analogReadResolution(12);
  analogWriteResolution(12);
  
  // Calculate targetLambda based on the desired target AFR and the stoichiometric AFR
  targetLambda = TARGET_AFR / STOICH_AFR; // F.eks 14.3 / 14.7 = 0.972

  matrix.loadSequence(LEDMATRIX_ANIMATION_STARTUP);
  matrix.begin();
  matrix.play(true);
  Serial.begin(9600);
  
  Serial.print("System Ready. Target AFR: "); Serial.println(TARGET_AFR);
  Serial.print("Target Lambda: "); Serial.println(targetLambda, 3);
}

void loop() {
  int aemRaw = analogRead(aemInputPin);
  float aemVoltage = aemRaw * (5.0 / 4095.0);
  float lambda = (0.1621 * aemVoltage) + 0.4990;
  
  float outputVoltage;
  String status;

  if (aemVoltage < 0.5) {
    outputVoltage = 0.5; 
    status = "WARMING";
  } else if (aemVoltage > 4.5) {
    outputVoltage = 0.5;
    status = "ERROR";
  } else {
    // Dynamic simulation of narrowband sensor
    outputVoltage = 0.5 - ((lambda - targetLambda) * SLOPE_STEEPNESS);

    // Hard-clamping emulates small band output 0.1V - 0.9V
    outputVoltage = constrain(outputVoltage, 0.1, 0.9);
    status = "RUNNING";
  }

  int dacValue = round(outputVoltage / 5.0 * 4095.0);
  analogWrite(kjetOutputPin, dacValue);

  // Graph updates and serial prints throttled to avoid overwhelming the system
  static unsigned long lastPrint = 0;
  if (millis() - lastPrint > 1000) {
    Serial.print("AFR: "); Serial.print(lambda * STOICH_AFR, 1);
    Serial.print(" | Out: "); Serial.print(outputVoltage, 3);
    Serial.print("V | Status: "); Serial.println(status);
    lastPrint = millis();
  }

  static unsigned long lastChart = 0;

  if (millis() - lastChart > 50) {
    plotVoltageFrame(outputVoltage);
    lastChart = millis();
  }

}