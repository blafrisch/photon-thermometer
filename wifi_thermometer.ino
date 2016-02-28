#include <QueueList.h>

// TODO: Ability to set thermocouples up via HTTP: enable pin, set low and high, do reading
// TODO: Ability to set altitude via HTTP
// TODO: Ability to toggle C/F

bool DEBUG = false;
float AD8495_OFFSET = 1.25;
int QUEUE_MAX_LEN = 10;
// TODO: make variable based on altitude
float REFERENCE_LOW = 0.0;
float REFERENCE_HIGH = 99.6;
float REFERENCE_RANGE = REFERENCE_HIGH - REFERENCE_LOW;

// TODO: Make able to handle > 1 thermocouple
int tcouplePin = A1;
QueueList <int> tcoupleAvgQueue;
int tcoupleRawLow = -3.73;
int tcoupleRawHigh = 100.0;

void setup() {
  Serial.begin(9600);
}

void enqueueValue(QueueList<int> &queue, int value) {
  if (queue.count() == QUEUE_MAX_LEN) {
    queue.pop();
  }
  queue.push(value);
}

float averageValueInQueue(QueueList<int> &queue) {
  float total = 0;
  for (int i=0; i < queue.count(); i++) {
    int value = queue.pop();
    total += (value * 1.0);
    queue.push(value);
  }
  return total / queue.count();
}

float twoPointCalibration(float rawValue, float rawLow, float rawHigh) {
  float rawRange = rawHigh - rawLow;
  return (((rawValue - rawLow) * REFERENCE_RANGE) / rawRange) + REFERENCE_LOW;
}

float calculateTemp(float input, float rawLow, float rawHigh) {
  float voltageOut = input * (3.3 / 4095.0);
  float tempC = (voltageOut - AD8495_OFFSET) / 0.005; // 0.005 is magic?
  if (DEBUG) Serial.printf("Actual reading degrees C: %f\n", tempC);
  tempC = twoPointCalibration(tempC, rawLow, rawHigh);
  if (DEBUG) Serial.printf("Adjusted reading degrees C: %f\n", tempC);
  return tempC;
}

float degreesCtoF(float degreesC) {
  return ((degreesC * 9.0) / 5.0) + 32.0;
}

void loop() {
  int raw = analogRead(tcouplePin);
  if (DEBUG) Serial.printf("Raw value: %d\n", raw);
  enqueueValue(tcoupleAvgQueue, raw);
  float avgVal = averageValueInQueue(tcoupleAvgQueue);
  if (DEBUG) Serial.printf("Avg queue value: %f\n", avgVal);
  float temp = calculateTemp(avgVal, tcoupleRawLow, tcoupleRawHigh);
  Serial.printf("Temperature C: %f | F: %f\n", temp, degreesCtoF(temp));

  delay(500);
}
