#include <ArduinoHttpClient.h>
#include <ArduinoJson.h>
#include "arduino_secrets.h"
#include <SPI.h>
#include <WiFiNINA.h>


//wifi settings
char ssid[] = SECRET_SSID; 
char pass[] = SECRET_PASS;
int status = WL_IDLE_STATUS;

//strava settings
char hostname[] = "www.strava.com";
const char path[] = "/api/v3/activities?access_token=";
String refresh_codes[3];

StaticJsonDocument<500> doc;

void setup() 
{  
  Serial.begin(115200);
    
  // attempt to connect to WiFi network:
  while (status != WL_CONNECTED) 
  {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
    status = WiFi.begin(ssid, pass);
    // wait 10 seconds for connection:
    delay(10000);
  }
  Serial.println("Connected to WiFi");
  Serial.println("\nStarting connection to server...");
  WiFiSSLClient wifi;
  HttpClient client = HttpClient(wifi, hostname, 443);

  //get the refresh token
  String contentType = "application/x-www-form-urlencoded";

  //post data is broken down into different variables:
  String s1 = "client_id=";
  String s2 = CLIENT_ID;
  String s3 = "&client_secret=";
  String s4 = CLIENT_SECRET;
  String s5 = "&grant_type=authorization_code&code=";
  
  String postData = s1+s2+s3+s4+s5; 

  //at the end, add each runner's authorization code and call the post
  for (int r=0; r<4;r++)
  {
    switch (r)
    {
      case 0:
        postData = postData + RUNNER1_CODE;
        break;
      case 1:
        postData = postData + RUNNER2_CODE;
        break;
      case 2:
        postData = postData + RUNNER3_CODE;
        break;
      case 3:
        postData = postData + RUNNER4_CODE;
        break;        
    }
  
    client.post("/oauth/token", contentType, postData);

    // read the status code and body of the response
    int statusCode = client.responseStatusCode();
    String response = client.responseBody();

    Serial.print("Status code: ");
    Serial.println(statusCode);
    Serial.print("Response: ");
    Serial.println(response);

    //deserialise the json from client into doc
    DeserializationError err = deserializeJson(doc, response);
    
    refresh_codes[r] = doc["refresh_token"].as<String>();
    Serial.print("Runner ");
    Serial.print(r+1);
    Serial.print("'s refresh code: ");
    Serial.println(doc["refresh_token"].as<String>());
    Serial.println("********************************************");
       
  }

}


void loop() 
{
  
}
