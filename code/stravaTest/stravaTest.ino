#include <ArduinoHttpClient.h>
#include <ArduinoJson.h>
#include "arduino_secrets.h"
#include <SPI.h>
#include <WiFiNINA.h>

//motor variables
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
long int fieldDistance = 4500;

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

//field
//480*9
int field_length = 4320;

//wifi settings
char ssid[] = SECRET_SSID; 
char pass[] = SECRET_PASS;
int status = WL_IDLE_STATUS;

//strava settings
char hostname[] = "www.strava.com";
//unix timestamp for 1st April 2023 = 1680307200
const char path[] = "/api/v3/athlete/activities?per_page=30&after=1680307200&access_token=";
//String runners_codes[] = {runner1Code, runner2Code, runner3Code, runner4Code};
String access_codes[4];

StaticJsonDocument<500> doc;
StaticJsonDocument<2000> doc2;

void setup() 
{  
  Serial.begin(115200);
  
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

  //initialize the encoders
  EncoderInit();  
  
  // attempt to connect to WiFi network:
  while (status != WL_CONNECTED) 
  {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
    status = WiFi.begin(ssid, pass);
  
    // wait 10 seconds for connection:
    //OR let's gooooo
    delay(1000);
    
  }
  Serial.println("Connected to WiFi");
  Serial.println("\nStarting connection to server...");
  WiFiSSLClient wifi;

  //go to the start
  goToStartLine();
  
  //get the access token using the refresh token for each runner
  String contentType = "application/x-www-form-urlencoded";

  Serial.println("Get access tokens");
  //post data is broken down into different variables:
  String s1 = "client_id=";
  String s2 = CLIENT_ID;
  String s3 = "&client_secret=";
  String s4 = CLIENT_SECRET;
  String s5 = "&grant_type=refresh_token&refresh_token=";

  //at the end, add each runner's refresh code and call the post
  for (int r=0; r<4;r++)
  {
    String postData = s1+s2+s3+s4+s5;   
    //a new http call 
    HttpClient client = HttpClient(wifi, hostname, 443);

    //add each runner's unique refresh code
    switch (r)
    {
      case 0:
        postData = postData + RUNNER1_REFRESH;
        break;
      case 1:
        postData = postData + RUNNER2_REFRESH;
        break;
      case 2:
        postData = postData + RUNNER3_REFRESH;
        break;
      case 3:
        postData = postData + RUNNER4_REFRESH;
        break;        
    }

    //TEST Serial.print("Runner: ");
    //TEST Serial.println(r+1);
    //TEST Serial.println(postData);
    
    
    client.post("/oauth/token", contentType, postData);

    // read the status code and body of the response
    int statusCode = client.responseStatusCode();
    String response = client.responseBody();

    //TEST Serial.print("Status code: ");
    //TEST Serial.println(statusCode);
    //TEST Serial.print("Response: ");
    //TEST Serial.println(response);

    //deserialise the json from client into doc
    DeserializationError err = deserializeJson(doc, response);    

    //get the access code
    access_codes[r] = doc["access_token"].as<String>();
  }

  Serial.println("Get running data");
  //keep track of each runner's total 
  float running_total;
  float runners[4];

  //get the data for each runner
  for (int r=0; r<4;r++)
  {
    //a new http call 
    HttpClient client = HttpClient(wifi, hostname, 443);
    
    running_total = 0;
    Serial.print("Runner ");
    Serial.print(r+1);
  
    //get their data
    client.get(path+access_codes[r]);
    //TEST Serial.print("path: ");
    //TEST Serial.println(path+access_codes[r]);
    
    // read the status code and body of the response
    int statusCode = client.responseStatusCode();
    //TEST Serial.print("Status code: ");
    //TEST Serial.println(statusCode);
    
    int bodyLen = client.contentLength();
  
    //create a filter, cause we don't have room for all the data
    StaticJsonDocument<64> filter;
    filter[0]["distance"] = true;
    filter[0]["type"] = true;
    filter[0]["start_date_local"] = true;
  
    //deserialise the json from client into doc
    DeserializationError err = deserializeJson(doc2, client, DeserializationOption::Filter(filter));
  
    //let's get the jsonArray out of doc
    JsonArray arrays = doc2.as<JsonArray>();

    //go through the array
    for(JsonObject v : arrays) 
    {
      float distance = v["distance"];
      String type = v["type"];
      long start_date  = v["start_date_local"];
      if (type == "Run")
      {
        running_total += distance;
      }
    } 
    
    runners[r] = running_total; 
    Serial.print(" ran ");
    Serial.println((runners[r]/1000));    
  }


   for (int r=0; r<4;r++)
   {
      if (runners[r] > 0)
        runRunners(r, runners[r]);
   }

   
  
}


void runRunners(int r, float total)
{
  float move_runner = 0;
  
  //convert total in our scale
  
  //first it's in metres
  total = total/1000;  
  
  //convert meters into field distance
  move_runner = (total*fieldDistance)/goalDistance;
 
   
  Serial.print("Runner: ");
  Serial.print(r+1);
  Serial.print(" ran ");
  Serial.print(total);
  Serial.print("kms, move them ");
  Serial.println(move_runner);
  
  
  long int motor_direction = 0;
  long int motor_speed = 0;
  long int encoderDistance = 0;
  
  switch (r)
  {
      case 0:        
        motor_direction = MOTOR1_DIRECTION;
        motor_speed = MOTOR1_SPEED;
        encoderDistance = encoder1Distance;
        break;
      case 1:
        motor_direction = MOTOR2_DIRECTION;
        motor_speed = MOTOR2_SPEED;
        encoderDistance = encoder2Distance;
        break;
      case 2:
        motor_direction = MOTOR3_DIRECTION;
        motor_speed = MOTOR3_SPEED;
        encoderDistance = encoder3Distance;;
        break;
      case 3:
        motor_direction = MOTOR4_DIRECTION;
        motor_speed = MOTOR4_SPEED;
        encoderDistance = encoder4Distance;
        break;        
    }
  

  //did we move?
  if (move_runner > 0)
  {
    //let's gooo
    digitalWrite(motor_direction,LOW);
    analogWrite(motor_speed,70);

      //run!
      while (encoderDistance < move_runner)
      {
        delay(10);        
        if(r == 0)
          encoderDistance = encoder1Distance;
        if(r == 1)
          encoderDistance = encoder2Distance;
        if(r == 2)
          encoderDistance = encoder3Distance;
        if(r == 3)
          encoderDistance = encoder4Distance;
      }
    digitalWrite(motor_speed,LOW);  
  }
  
}

void loop() 
{
  
}



void dance(long int motor_speed, long int motor_direction, long int encoderDistance)
{
    long int count = 0;
    for (int i =0; i<4;i++)
    {
      //back 
      digitalWrite(motor_direction,HIGH);
      analogWrite(motor_speed,90);
      count = 0;
      while (count < 100)
      {
      delay(10);
      }      
  
      digitalWrite(motor_direction,LOW);
      analogWrite(motor_speed,90);
      count = 0;
      while (count < 100)
      {
      delay(10);
      }      
    }

    digitalWrite(motor_speed,LOW);  
}

void goToStartLine()
{
  Serial.println("BACK TO START");
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
  }
}
