#define TRIG 7
#define ECHO 6

void setup(){
  pinMode(TRIG, OUTPUT);
  pinMode(ECHO, INPUT);
  Serial.begin(9600);
}
void loop(){
  digitalWrite(TRIG, LOW); delayMicroseconds(2);
  digitalWrite(TRIG, HIGH); delayMicroseconds(10);
  digitalWrite(TRIG, LOW); 

  long dur = pulseIn(ECHO, HIGH, 30000);
  int dist = dur * 0.017;

  Serial.print("Distance: "); Serial.print(dist); Serial.println("cm");
  delay(100); 
}
