const byte MOTOR1_encoderA = 13; //SDA
const byte MOTOR1_encoderB = 20; 
const byte MOTOR2_encoderA = 18; //A4
const byte MOTOR2_encoderB = 19; //A5
const byte MOTOR3_encoderA = 14; //A0
const byte MOTOR3_encoderB = 15; //A1
const byte MOTOR4_encoderA = 16; //A2
const byte MOTOR4_encoderB = 17; //A3

long int encoder1Distance = 0;
long int encoder2Distance = 0;
long int encoder3Distance = 0;
long int encoder4Distance = 0;

int runners[4] = {10,20,50,100};
int raceTrackDistance = 480;
int goalDistance = 100;

int MOTOR1_DIRECTION = 4;
int MOTOR1_SPEED = 3;

int MOTOR2_SPEED = 10;
int MOTOR2_DIRECTION = 12;

int MOTOR3_SPEED = 5;
int MOTOR3_DIRECTION = 8;

int MOTOR4_SPEED = 6;
int MOTOR4_DIRECTION = 7;

//switches
int MOTOR1_SWITCH = 21; //SCL
int MOTOR2_SWITCH = 1; 
int MOTOR3_SWITCH = 9; 
int MOTOR4_SWITCH = 2; 


//switch checks
int MOTOR1_STOP = 0;
int MOTOR2_STOP = 0;
int MOTOR3_STOP = 0;
int MOTOR4_STOP = 0;

void setup()
{
  
  //Create pins
  pinMode(MOTOR1_SPEED,OUTPUT);
  pinMode(MOTOR2_SPEED,OUTPUT);
  pinMode(MOTOR3_SPEED,OUTPUT);
  pinMode(MOTOR4_SPEED,OUTPUT);
  pinMode(MOTOR1_DIRECTION,OUTPUT);
  pinMode(MOTOR2_DIRECTION,OUTPUT);
  pinMode(MOTOR3_DIRECTION,OUTPUT);
  pinMode(MOTOR4_DIRECTION,OUTPUT);
  
  //Switches
  pinMode(MOTOR1_SWITCH,INPUT_PULLUP);  
  pinMode(MOTOR2_SWITCH,INPUT_PULLUP);  
  pinMode(MOTOR3_SWITCH,INPUT_PULLUP);
  pinMode(MOTOR4_SWITCH,INPUT_PULLUP);  
  
  
  //turn it all off
  digitalWrite(MOTOR1_DIRECTION,LOW);
  digitalWrite(MOTOR1_SPEED,LOW);
  digitalWrite(MOTOR2_DIRECTION,LOW);
  digitalWrite(MOTOR2_SPEED,LOW);
  digitalWrite(MOTOR3_DIRECTION,LOW);
  digitalWrite(MOTOR3_SPEED,LOW);
  digitalWrite(MOTOR4_DIRECTION,LOW);
  digitalWrite(MOTOR4_SPEED,LOW);  
  
  Serial.begin(57600);//Initialize the serial port


  EncoderInit();//Initialize the module
  
  goToStartLine();
  
}

void goToStartLine()
{
  analogWrite(MOTOR1_SPEED,70);
  analogWrite(MOTOR2_SPEED,70);
  analogWrite(MOTOR3_SPEED,70);
  analogWrite(MOTOR4_SPEED,70);
  
  digitalWrite(MOTOR1_DIRECTION,HIGH);
  digitalWrite(MOTOR2_DIRECTION,HIGH);
  digitalWrite(MOTOR3_DIRECTION,HIGH);
  digitalWrite(MOTOR4_DIRECTION,HIGH);  
  
 
  MOTOR1_STOP = digitalRead(MOTOR1_SWITCH);
  MOTOR2_STOP = digitalRead(MOTOR2_SWITCH);
  MOTOR3_STOP = digitalRead(MOTOR3_SWITCH);
  MOTOR4_STOP = digitalRead(MOTOR4_SWITCH);

  //keep going while the switches haven't been hit  
  while (MOTOR1_STOP || MOTOR2_STOP || MOTOR3_STOP || MOTOR4_STOP)
  {
    MOTOR1_STOP = digitalRead(MOTOR1_SWITCH);
    MOTOR2_STOP = digitalRead(MOTOR2_SWITCH);
    MOTOR3_STOP = digitalRead(MOTOR3_SWITCH);
    MOTOR4_STOP = digitalRead(MOTOR4_SWITCH);

    //if the switch was hit, turn off the motor
    if (MOTOR1_STOP == 0)
      digitalWrite(MOTOR1_SPEED,LOW);
    if (MOTOR2_STOP == 0)
      digitalWrite(MOTOR2_SPEED,LOW);    
    if (MOTOR3_STOP == 0)
      digitalWrite(MOTOR3_SPEED,LOW);
    if (MOTOR4_STOP == 0)
      digitalWrite(MOTOR4_SPEED,LOW);       
  }

   encoder1Distance = 0;
   encoder2Distance = 0;
   encoder3Distance = 0;
   encoder4Distance = 0;
  
}

void goRunners()
{
  
  for (int r=0; r<3; r++)
  {
    int distance = runners[r];
    int moveMe = (raceTrackDistance/goalDistance)*r;
    
  }
}

void loop()
{ 
  Serial.print("encoder4 START");
  Serial.println(encoder4Distance);
  digitalWrite(MOTOR4_DIRECTION,LOW);
  analogWrite(MOTOR4_SPEED,90);
  
  while (encoder4Distance < 1000)
  {
    delay(10);
    Serial.print("encoder4 ");
    Serial.println(encoder4Distance);
  }
  digitalWrite(MOTOR4_SPEED,LOW);


  Serial.print("encoder1 START");
  Serial.println(encoder1Distance);
  digitalWrite(MOTOR1_DIRECTION,LOW);
  analogWrite(MOTOR1_SPEED,90);
  
  while (encoder1Distance < 3000)
  {
    delay(10);
    Serial.print("encoder1 ");
    Serial.println(encoder1Distance);
  }
  digitalWrite(MOTOR1_SPEED,LOW);

  Serial.print("encoder2 START ********************************************************************************************");
  Serial.print("encoder2 START");
  Serial.println(encoder2Distance);
  digitalWrite(MOTOR2_DIRECTION,LOW);
  analogWrite(MOTOR2_SPEED,90);
  
  while (encoder2Distance < 2000)
  {
    delay(10);
    Serial.print("encoder2 ");
    Serial.println(encoder2Distance);
  }
  digitalWrite(MOTOR2_SPEED,LOW);


  Serial.print("encoder3 START");
  Serial.println(encoder3Distance);
  digitalWrite(MOTOR3_DIRECTION,LOW);
  analogWrite(MOTOR3_SPEED,90);
  
  while (encoder3Distance < 4600)
  {
    delay(10);
    Serial.print("encoder3 ");
    Serial.println(encoder3Distance);
  }
  digitalWrite(MOTOR3_SPEED,LOW);

void EncoderInit()
{
  //setup the A and B pins
  pinMode(MOTOR1_encoderA,INPUT);
  pinMode(MOTOR1_encoderB,INPUT);
  pinMode(MOTOR2_encoderA,INPUT);
  pinMode(MOTOR2_encoderB,INPUT);
  pinMode(MOTOR3_encoderA,INPUT);
  pinMode(MOTOR3_encoderB,INPUT);
  pinMode(MOTOR4_encoderA,INPUT);
  pinMode(MOTOR4_encoderB,INPUT);


  //setting it up so whenever A goes from high to low, call wheelspeed
  attachInterrupt(digitalPinToInterrupt(MOTOR1_encoderA), wheelSpeed1, FALLING);
  attachInterrupt(digitalPinToInterrupt(MOTOR2_encoderA), wheelSpeed2, FALLING);
  attachInterrupt(digitalPinToInterrupt(MOTOR3_encoderA), wheelSpeed3, FALLING);
  attachInterrupt(digitalPinToInterrupt(MOTOR4_encoderA), wheelSpeed4, FALLING);
}

void wheelSpeed1()
{
  //if B is high, we're going forwards. our distance goes up
  if (digitalRead(MOTOR1_encoderB)) 
  {
    encoder1Distance++;
  } 
  else 
  {
    encoder1Distance--;
  }
}

void wheelSpeed2()
{
  //if B is high, we're going forwards. our distance goes up
  if (digitalRead(MOTOR2_encoderB)) 
  {
    encoder2Distance++;
  } 
  else 
  {
    encoder2Distance--;
  }
}


void wheelSpeed3()
{
  //if B is high, we're going forwards. our distance goes up
  if (digitalRead(MOTOR3_encoderB)) 
  {
    encoder3Distance++;
  } 
  else 
  {
    encoder3Distance--;
  }
}

void wheelSpeed4()
{
  //if B is high, we're going forwards. our distance goes up
  if (digitalRead(MOTOR4_encoderB)) 
  {
    encoder4Distance++;
  } 
  else 
  {
    encoder4Distance--;
r  }
}
