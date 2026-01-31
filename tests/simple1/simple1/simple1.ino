// defines pins numbers

const int stepX = 2;

const int dirX  = 5;



const int stepY = 3;

const int dirY  = 6;



const int stepZ = 4;

const int dirZ  = 7;



const int enPin = 8;



void setup() {



  // Sets the two pins as Outputs

  pinMode(stepX,OUTPUT);

  pinMode(dirX,OUTPUT);



  pinMode(stepY,OUTPUT);

  pinMode(dirY,OUTPUT);



   pinMode(stepZ,OUTPUT);

  pinMode(dirZ,OUTPUT);



  pinMode(enPin,OUTPUT);

  digitalWrite(enPin,LOW);



  digitalWrite(dirX,HIGH);

  digitalWrite(dirY,LOW);

  digitalWrite(dirZ,HIGH);



}

void loop() {


  // Enables the motor to move in a particular direction

  // Makes 200 pulses for making one full cycle rotation

  int n ;

  n = random(1, 5) *200;
  digitalWrite(dirX, random(2)); 

  for(int x = 0; x < n; x++) {
    digitalWrite(stepX,HIGH);

    delayMicroseconds(1000);

    digitalWrite(stepX,LOW);

    delayMicroseconds(1000);

  };
  delay(1000); // One second delay

digitalWrite(dirY, random(2)); 
  n = random(1,4) *200;
   for(int x = 0; x < n; x++) {

    digitalWrite(stepY,HIGH);

    delayMicroseconds(1000);

    digitalWrite(stepY,LOW);

    delayMicroseconds(1000);

  }
   delay(1000); // One second delay


digitalWrite(dirZ, random(2)); 
  n = random(1,3) *200;
  for(int x = 0; x < n; x++) {

    digitalWrite(stepZ,HIGH);

    delayMicroseconds(1000);

    digitalWrite(stepZ,LOW);

    delayMicroseconds(1000);

  }
   delay(1000); // One second delay
}
