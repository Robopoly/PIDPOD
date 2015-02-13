/*
 * Name:        PIDPOD
 * Date:        2015-01-30
 * Version:     1.0
 * Description: Segway-type self-balancing robot sketch based on the CC3200 development board.
 */

/* --------------- TIMER SETTINGS ----------------  */
/* TIMER0 --> motors                                */
/* TIMER1 --> imu_controller                        */
/* TIMER2 --> odometry                              */
/* TIMER3 --> odometry_controller                   */
/* ------------------------------------------------ */


#include <WiFi.h>
#include <WiFiClient.h>
#include <WiFiServer.h>

char ssid[] = "Hotspotname";
char password[] = "password";


// Don't forget to change the WiFi.begin function as well

WiFiServer server(80);

#include <Wire.h>
#include <MPU9150.h>
#include <Motors.h>
#include <imu_control.h>
#include <odometer.h>

float kp = 10;    // 10 is ok
float ki = 30;    // 25/30 is ok. Maybe even more
float kd = 0.5;   // 0.4/0.5 is ok, 1 is stability limit with others ok values

// Pin definitions
#define LED       RED_LED
#define SWAG_LED  5
#define DIP4      15
#define DIP2      18
#define DIP3      17
#define DIP1      19  

void setup()
{
  Serial.begin(115200);
  pinMode(DIP4, INPUT);
  pinMode(DIP1, INPUT);
  pinMode(14, INPUT);    // odo1 et 2
  pinMode(6, INPUT);
  pinMode(LED, OUTPUT);
  pinMode(SWAG_LED, OUTPUT);

  digitalWrite(SWAG_LED, HIGH);
  delay(1000);
  
  
  // --------- START WIFI
  if(digitalRead(DIP1))
  {
    startWifi(ssid, password);
  }
  // --------- END WIFI
  
  motorSetup();

  /* Initialize serial communication */
  Serial.begin(115200);

  /* Initialize the 'Wire' class for the I2C-bus needed for IMU */
  Wire.begin();

  /* Setup odometer */
  odometer_setup();

  /* Setup IMU and IMU parameters */
  imu_setup();

  /* Set controller parameters */
  set_controller_parameters(kp, ki, kd);
  controller_setup();
  odometer_controller_setup();      // MUST be the last one called!


  digitalWrite(LED, HIGH);
  delay(100);
  digitalWrite(LED, LOW);
  digitalWrite(SWAG_LED, LOW);

}




boolean startWifi(char ssid[], char password[])
{
  uint8_t retries = 3;
  uint8_t tries;

  while(retries)
  {
    if(strlen(password) == 0)
    {
      WiFi.begin(ssid);
    }
    else
    {
      WiFi.begin(ssid, password);
    }

    for(tries = 0; tries < 10 && WiFi.status() != WL_CONNECTED; tries++)
    {
      Serial.print(".");
      delay(200);
    }

    // Try again
    if(retries == 0)
    {
      retries--;
      continue;
    }

    Serial.println("\nYou're connected to the network");
    Serial.println("Waiting for an ip address");

    while(WiFi.localIP() == INADDR_NONE)
    {
      // print dots while we wait for an ip addresss
      Serial.print(".");
      delay(300);
    }

    // you're connected now, so print out the status  
    printWifiStatus();

    Serial.println("Starting webserver on port 80");
    server.begin();                           // start the web server on port 80
    Serial.println("Webserver started!");

    return true;
  }

  return false;
}



void loop()
{
  /* Wifi section is managed "best effort" */

  if(digitalRead(DIP1))
  {
    wifi();
    set_controller_parameters(kp, ki, kd);
  }  

  delay(100);
}



void wifi()
{
  char input[20];    
  char type, c, testchar;    
  uint8_t count = 0;
  int kint = 0;
  float kfloat = 0;
  uint8_t found = 0;
  char digit[4];
  boolean currentLineIsBlank = true;
  WiFiClient client;

  // listen for incoming clients
  client = server.available();

  if (client) {
    //Serial.println("new client");
    // an http request ends with a blank line
    currentLineIsBlank = true;
    count = 0;
    kint = 0;
    kfloat = 0;
    found = 0;

    while(client.connected())
    {
      if(client.available())
      {
        c = client.read();
        //Serial.write(c);

        // Catch the first 20 lines
        if(count < 20)
        {
          count++,
          input[count] = c;
        }

        //Serial.print(c);

        // if you've gotten to the end of the line (received a newline
        // character) and the line is blank, the http request has ended,
        // so you can send a reply
        if (c == '\n' && currentLineIsBlank) {
          found = 0;
          count = 0;
          // send a standard http response header
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: text/html");
          client.println("Connection: close");  // the connection will be closed after completion of the response
          client.println("Refresh: 8");
          client.println();
          client.print("<!DOCTYPE HTML><html><script>");
          client.print("function drag(field,value){document.getElementById(field).innerHTML=value/100;}function sendValues(){var fields=['JP','JI','JD'];for(i=0;i<3;i++){set(fields[i],document.getElementById(fields[i]).value);}}function set(field,value){var xmlhttp = new XMLHttpRequest();xmlhttp.open(\"GET\",\"?\"+field+\"=\"+(value<1000?'0':'')+(value<100?'0':'')+(value<10?'0':'')+value,true);xmlhttp.send(null);}</script>");
          client.print("Upright position: ");
          client.print(get_accelerometer_default_offset());
          client.print("    Corrected upright position: ");
          client.print(get_accelerometer_offset());
          client.print("<br />Set values: <input type=\"button\" value=\"Set\" onclick=\"sendValues();\" /><br />");
          client.print("KP: <span id=\"KP\">10</span> <input style=\"width:100%\" type=\"range\" name=\"kp\" min=\"0\" max=\"2000\" value=\"1000\" step=\"1\" id=\"JP\" oninput=\"drag('KP',this.value)\" /><br />KI: <span id=\"KI\">10</span> <input style=\"width:100%\" type=\"range\" name=\"ki\" min=\"0\" max=\"2000\" value=\"1000\" step=\"1\" id=\"JI\" oninput=\"drag('KI',this.value)\" /><br />KD: <span id=\"KD\">0.5</span> <input style=\"width:100%\" type=\"range\" name=\"kd\" min=\"0\" max=\"2000\" value=\"50\" step=\"1\" id=\"JD\" oninput=\"drag('KD',this.value)\" />");
          client.print("Kp: ");
          client.print(kp);
          client.print("  Ki: ");
          client.print(ki);
          client.print("  Kd: ");
          client.print(kd);
          client.println("</html>");
          break;
        }
        if (c == '\n') {
          // you're starting a new line
          if(!found)
          {
            type = input[8];
            digit[0] = input[10];
            digit[1] = input[11];
            digit[2] = input[12];
            digit[3] = input[13];

            // parse
            kint = (((uint8_t)digit[0])-48)*1000 + (((uint8_t)digit[1])-48)*100 + (((uint8_t)digit[2])-48)*10 + ((uint8_t)digit[3])-48;
            kfloat = (float)kint/100.;

            if(kint <= 2000)
            {
              switch(type)
              {
              case 'P':
                kp = kfloat;
                break;
              case 'I':
                ki = kfloat;
                break;
              case 'D':
                kd = kfloat;
                break;
              }
              Serial.println(kint);
            }

            found = 1;
          }

          currentLineIsBlank = true;
        }
        else if (c != '\r') {
          // you've gotten a character on the current line
          currentLineIsBlank = false;
        }
      }
    }
    // give the web browser time to receive the data
    delay(1);

    // close the connection:
    client.stop();
    //Serial.println("client disonnected");
  }
}




void printWifiStatus() {
  // print the SSID of the network you're attached to:
  Serial.print("Network Name: ");
  Serial.println(WiFi.SSID());

  // print your WiFi shield's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}


