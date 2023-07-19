#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <stdlib.h>
#include <string>
#include<time.h>

#define blindsName "living_room_middle"
#define blindsName1 "living_room_left"

//lenght of step
#define step  3000

//period from open to close
#define period 1900
//default 1500, bedroom 1900
#define period1 1500

//total lenght
#define topPosition 55000
#define topPosition1 55000
//bedroom: 60000

//loop delay
#define loopDelaymSec 10

//moving up is slower, coefficient
#define UpCoefficient 0.975



void MQTTcallback(char* topic, byte* payload, unsigned int length);
void reportPosition();

//define IO PINs
#define UP  5
//UP bedroom 4 default 15
#define DOWN  4 
//DOWN bedroom 5 default 16

#define addGND 2
#define addGND1 13

#define ENDSWITCH 14
//EnDSWITCH bedroom 14 default 14
#define DOWNSWITCH 12
//DOWNSWITCH bedroom 12 default 12
#define UPSWITCH 3
//UPSWITCH bedroom 13 default 13

#define UP1  15 //2
#define DOWN1  16 //4 
#define ENDSWITCH1 0 //0 //2
#define DOWNSWITCH1 3 //5
#define UPSWITCH1 3 //3


//driving variables
int movingUp = 0;
int movingDown = 0;
int position = 0;
int tilt = 0;
int movingForMsec = 0;
int moveToBottom = 0;
timeval  previousTime, currentTime;

int movingUp1 = 0;
int movingDown1 = 0;
int position1 = 0;
int tilt1 = 0;
int movingForMsec1 = 0;
int moveToBottom1 = 0;

int remainingStep = -2000;

int remainingStep1 = -2000;


int prevUpSwitchValue = HIGH;
int prevDownSwitchValue = HIGH;
 
int prevUpSwitchValue1 = HIGH;
int prevDownSwitchValue1 = HIGH;

WiFiClient espClient;
PubSubClient client(espClient);
// the setup function runs once when you press reset or power the board
void setup() {

  //initialize needed pins
  pinMode(UP, OUTPUT); //UP
  pinMode(DOWN, OUTPUT); //DOWN

  pinMode(UP1, OUTPUT); //UP1
  pinMode(DOWN1, OUTPUT); //DOWN1
  pinMode(addGND, OUTPUT);
  pinMode(addGND1, OUTPUT);
  //pinMode(PWM, OUTPUT);
  //pinMode(TEST, OUTPUT);
  pinMode(ENDSWITCH, INPUT_PULLUP);
  pinMode(UPSWITCH, INPUT_PULLUP);
  pinMode(DOWNSWITCH, INPUT_PULLUP);

  pinMode(ENDSWITCH1, INPUT_PULLUP);
  pinMode(UPSWITCH1, INPUT_PULLUP);
  pinMode(DOWNSWITCH1, INPUT_PULLUP);


  Serial.begin(9600,SERIAL_8N1,SERIAL_TX_ONLY);
  Serial.println();

 
  WiFi_setup();
  MQTT_setup();
 
  /*client.publish("homeassistant/switch/blinds_up/config","");
    client.publish("homeassistant/switch/blinds_down/config","");
    client.publish("homeassistant/switch/blinds_up_step/config","");
    client.publish("homeassistant/switch/blinds_down_step/config","");
  */
  /*client.publish("homeassistant/switch/blinds_up/config", "{\"name\": \"blinds_up\",  \"state_topic\": \"blinds/out/up\", \"command_topic\": \"blinds/up\"}",true );
    client.publish("homeassistant/switch/blinds_down/config", "{\"name\": \"blinds_down\",  \"state_topic\": \"blinds/out/down\", \"command_topic\": \"blinds/down\"}",true );
    client.publish("homeassistant/switch/blinds_up_step/config", "{\"name\": \"blinds_down_step\",  \"state_topic\": \"blinds/out/down\", \"command_topic\": \"blinds/down_step\"}",true );
    client.publish("homeassistant/switch/blinds_down_step/config", "{\"name\": \"blinds_up_step\",  \"state_topic\": \"blinds/out/up\", \"command_topic\": \"blinds/up_step\"}",true );*/
  /*client.publish((String("homeassistant/switch/blinds_")+blindsName+"_up_force/config").c_str(), (String("{\"name\": \"blinds_")+blindsName+"_up_force\\" ).c_str(),  \"state_topic\": \"blinds/out/up\", \"command_topic\": \"blinds/up_force\"}", true );
  client.publish((String("homeassistant/switch/blinds_")+blindsName+"_down_force/config").c_str(), (String("{\"name\": \"blinds_")+blindsName+"_down_force\\").c_str(),  \"state_topic\": \"blinds/out/down\", \"command_topic\": \"blinds/down_force\"}", true );
*/
  digitalWrite(addGND , LOW);
  digitalWrite(addGND1 , LOW);
  
//  digitalWrite(PWM , LOW);
//  digitalWrite(addGND  , HIGH);

  gettimeofday(&previousTime, NULL);
  

}
void MQTT_setup(){
   const char* mqtt_server = "CHANGE_ME_MQTT_SERVER";
  const int mqtt_port = 1883;

  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = blindsName;
    client.setServer(mqtt_server, mqtt_port);
    client.setCallback(MQTTcallback);
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
       Serial.println("connected to mqtt");
      client.subscribe((String("blinds/") + blindsName + "/set").c_str());
      client.subscribe((String("blinds/") + blindsName + "/set_position").c_str());
      client.subscribe((String("blinds/") + blindsName + "/set_tilt").c_str());

      client.subscribe((String("blinds/") + blindsName1 + "/set").c_str());
      client.subscribe((String("blinds/") + blindsName1 + "/set_position").c_str());
      client.subscribe((String("blinds/") + blindsName1 + "/set_tilt").c_str());

    }  
    else
    {
      Serial.print("failed with state ");
      Serial.println(client.state());
      delay(2000);
    }
  }
}  



void WiFi_setup(){
  WiFi.begin("CHANGE_ME_WIFI_SSID", "CHANGE_ME_WIFI_PASSWORD");
  
  Serial.print("Connecting");
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  
  Serial.print("Connected, IP address: ");
  Serial.println(WiFi.localIP());
}


void moveUp(int timespanmSec,int side)
{
  if (side ==0 ){
    moveToBottom = 0;
    if (position < topPosition) {
      
      //turn off moving down (if any)
      digitalWrite(DOWN  , LOW);
      digitalWrite(UP  , HIGH);
  
      movingDown = 0;
      movingUp = 1;
      if (timespanmSec == 0){
        remainingStep  = topPosition-position;  
      }
      else {
        remainingStep   = timespanmSec;
      }
  
    }
  }
  else {
    moveToBottom1 = 0;
    if (position < topPosition) {
      
      //turn off moving down (if any)
      digitalWrite(DOWN1  , LOW);
      digitalWrite(UP1  , HIGH);
  
      movingDown1 = 0;
      movingUp1 = 1;
      if (timespanmSec == 0){
        remainingStep1  = topPosition1-position1;  
      }
      else {
        remainingStep1   = timespanmSec;
      }
  
    }
  }

}

void stop(int side)
{
  //turn off moving (if any)
  if (side==0) {
    digitalWrite(DOWN  , LOW);
    digitalWrite(UP  , LOW);
    movingUp = 0;
    movingDown = 0;
    moveToBottom = 0;
  }
  else{
    digitalWrite(DOWN1  , LOW);
    digitalWrite(UP1  , LOW);
    movingUp1 = 0;
    movingDown1 = 0;
    moveToBottom1 = 0;
  }
  reportPosition();
}

void moveDown(int timespanmSec,int side)
{
   if (side==0) {
    //move in case end switch is not pressed 
    int endSwitchValue = digitalRead(ENDSWITCH);
    //at bottom possition 
    if (endSwitchValue == HIGH){
      //Serial.println(timespanmSec);
      //turn off moving up (if any)
  
      digitalWrite(UP  , LOW);
      movingUp = 0;
  
      digitalWrite(DOWN  , HIGH);
      if (timespanmSec == 0){
        remainingStep = position;  
        moveToBottom = 1;
      }
      else {
        remainingStep   = timespanmSec;
        moveToBottom = 0;
      }
      movingDown = 1;
    }
    else {
      Serial.println("cannot move down side 0, at bottom possition ");  
    }
  }
  else {
    //move in case end switch is not pressed 
    int endSwitchValue = digitalRead(ENDSWITCH1);
    //at bottom possition 
    if (endSwitchValue == HIGH){
      //Serial.println(timespanmSec);
      //turn off moving up (if any)
  
      digitalWrite(UP1  , LOW);
      movingUp1 = 0;
  
      digitalWrite(DOWN1, HIGH);
      if (timespanmSec == 0){
        remainingStep1 = position1;  
        moveToBottom1 = 1;
      }
      else {
        remainingStep1 = timespanmSec;
        moveToBottom1 = 0;
      }
      movingDown1 = 1;
    }
    else {
      Serial.println("cannot move down side 1, at bottom possition ");  
    }
    

  }  
  
}





void reportPosition() {
  Serial.print("position side 0 percentage: ");
  float positionPercentage = (float)position / topPosition * 100;
  if (positionPercentage <= 1) positionPercentage = 1;
  if (positionPercentage > 100) positionPercentage = 100;
  Serial.println(positionPercentage );

  Serial.print("position side 1 percentage: ");
  float positionPercentage1 = (float)position1 / topPosition1 * 100;
  if (positionPercentage1 <= 1) positionPercentage1 = 1;
  if (positionPercentage1 > 100) positionPercentage1 = 100;
  Serial.println(positionPercentage1 );
  
  String state;
  if (positionPercentage < 50) state = "closed";
  else state = "open";
  client.publish( (String("blinds/") + blindsName + "/position").c_str(), String((int)positionPercentage).c_str(), true);
  client.publish( (String("blinds/") + blindsName + "/state").c_str(), state.c_str(), true);
  Serial.print("tilt percentage: ");
  int tiltPercentage = (int)((float)(position % period) / period * 100);
  Serial.println(tiltPercentage);
  client.publish( (String("blinds/") + blindsName + "/tilt_state").c_str(), String((int)tiltPercentage).c_str(), true);


  
  if (positionPercentage1 < 50) state = "closed";
  else state = "open";
  client.publish( (String("blinds/") + blindsName1 + "/position").c_str(), String((int)positionPercentage1).c_str(), true);
  client.publish( (String("blinds/") + blindsName1 + "/state").c_str(), state.c_str(), true);
  Serial.print("tilt side 1 percentage: ");
  tiltPercentage = (int)((float)(position1 % period1) / period1 * 100);
  Serial.println(tiltPercentage);
  client.publish( (String("blinds/") + blindsName1 + "/tilt_state").c_str(), String((int)tiltPercentage).c_str(), true);
}



void MQTTcallback(char* topic, byte* payload, unsigned int length)
{
  Serial.print("Message received in topic: ");
  Serial.println(topic);
  Serial.print("Message:");
  String message;
  for (int i = 0; i < length; i++)
  {
    message = message + (char)payload[i];
  }
  Serial.print(message);
  if (message == "UP" && strcmp (topic, (String("blinds/") + blindsName + "/set").c_str()) == 0)
  {
    Serial.println("pulling UP");
    moveUp(0,0);
  }
  else if (message == "UP" && strcmp (topic, (String("blinds/") + blindsName1 + "/set").c_str()) == 0)
  {
    Serial.println("pulling UP side 1");
    moveUp(0,1);
  }
  else if (message == "DOWN" && strcmp (topic, (String("blinds/") + blindsName + "/set").c_str())  == 0 )
  {
    Serial.println("pulling down");
    moveDown(0,0);
  }
  else if (message == "DOWN" && strcmp (topic, (String("blinds/") + blindsName1 + "/set").c_str())  == 0 )
  {
    Serial.println("pulling down side 1");
    moveDown(0,1);
  }
  
  else if (message != "" && strcmp (topic, (String("blinds/") + blindsName + "/set_position").c_str()) == 0)
  {
    int desiredPosition = atoi(message.c_str());
    Serial.print("target percentage: ");
    Serial.print(desiredPosition);
    int positionPercentage = (int)((float)position / topPosition * 100);
    Serial.print(" current percentage: ");
    Serial.print(positionPercentage);
    if (desiredPosition > positionPercentage) {
      float targetStep = (desiredPosition - positionPercentage ) * topPosition / 100;
      Serial.print(" step to target up: ");
      Serial.println(targetStep);
      moveUp(targetStep,0)  ;
    }
    if (desiredPosition < positionPercentage) {
      float targetStep = (positionPercentage - desiredPosition) * topPosition / 100;
      Serial.print("step to target down: ");
      Serial.println(targetStep);
      moveDown(targetStep,0)  ;
    }
  }


else if (message != "" && strcmp (topic, (String("blinds/") + blindsName1 + "/set_position").c_str()) == 0)
  {
    int desiredPosition = atoi(message.c_str());
    Serial.print("target percentage 1: ");
    Serial.print(desiredPosition);
    int positionPercentage = (int)((float)position1 / topPosition1 * 100);
    Serial.print(" current percentage 1: ");
    Serial.print(positionPercentage);
    if (desiredPosition > positionPercentage) {
      float targetStep = (desiredPosition - positionPercentage ) * topPosition / 100;
      Serial.print(" step to target up 1: ");
      Serial.println(targetStep);
      moveUp(targetStep,1)  ;
    }
    if (desiredPosition < positionPercentage) {
      float targetStep = (positionPercentage - desiredPosition) * topPosition / 100;
      Serial.print("step to target down 1: ");
      Serial.println(targetStep);
      moveDown(targetStep,1)  ;
    }
  }

  
  else if (message != "" && strcmp (topic, (String("blinds/") + blindsName + "/set_tilt").c_str()) == 0)
  {
    int desiredTilt = atoi(message.c_str());
    Serial.print("target tilt: ");
    Serial.print(desiredTilt);
    int tiltPercentage = (int)((float)(position % period) / period * 100);
    Serial.print(" current tilt percentage: ");
    Serial.print(tiltPercentage);
    if (desiredTilt > tiltPercentage) {
      float targetStep = ((desiredTilt - tiltPercentage ) * period / 100);
      Serial.print(" step to target tilt up: ");
      Serial.println(targetStep);
      moveUp(targetStep,1)  ;
    }
    
    if (desiredTilt < tiltPercentage) {
      float targetStep = (tiltPercentage - desiredTilt) * period / 100;
      Serial.print("step to target tilt down: ");
      Serial.println(targetStep);
      moveDown(targetStep,0)  ;
    }
  }
  else if (message != "" && strcmp (topic, (String("blinds/") + blindsName1 + "/set_tilt").c_str()) == 0)
  {
    int desiredTilt = atoi(message.c_str());
    Serial.print("target tilt 1: ");
    Serial.print(desiredTilt);
    int tiltPercentage = (int)((float)(position1 % period1) / period1 * 100);
    Serial.print(" current tilt percentage 1: ");
    Serial.print(tiltPercentage);
    if (desiredTilt > tiltPercentage) {
      float targetStep = ((desiredTilt - tiltPercentage ) * period1 / 100);
      Serial.print(" step to target tilt up 1: ");
      Serial.println(targetStep);
      moveUp(targetStep,1)  ;
    }
    
    if (desiredTilt < tiltPercentage) {
      float targetStep = (tiltPercentage - desiredTilt) * period1 / 100;
      Serial.print("step to target tilt down 1: ");
      Serial.println(targetStep);
      moveDown(targetStep,0)  ;
    }
  }

  

  else if (message == "OFF" && strcmp (topic, (String("blinds/") + blindsName + "/set").c_str())  == 0 )
  {
    Serial.println("turning off");
    stop(0);
  }
  
  else if (message == "OFF" && strcmp (topic, (String("blinds/") + blindsName1 + "/set").c_str())  == 0 )
  {
    Serial.println("turning off");
    stop(1);
  }
  else if (message == "OFF" && strcmp (topic, (String("blinds/") + blindsName + "/set_tilt").c_str())  == 0 )
  {
    Serial.println("turning off tilt");
    stop(0);
  }
  else if (message == "OFF" && strcmp (topic, (String("blinds/") + blindsName1 + "/set_tilt").c_str())  == 0 )
  {
    Serial.println("turning off tilt 1");
    stop(1);
  }
  else {
    Serial.print("unknown command received: ");
    Serial.println(message);
  }
}


void buttonHandle(){

    int upSwitchValue = digitalRead(UPSWITCH);
    int downSwitchValue = digitalRead(DOWNSWITCH);

    int upSwitchValue1 = digitalRead(UPSWITCH1);
    int downSwitchValue1 = digitalRead(DOWNSWITCH1);

  if ((upSwitchValue == LOW and prevUpSwitchValue == LOW ) || (prevUpSwitchValue1 ==LOW and upSwitchValue == LOW)){
      Serial.println("button up still pushed");
  }
  if (upSwitchValue == LOW and prevUpSwitchValue == HIGH){
       Serial.println("button up pushed");
       if (movingUp == 1 || movingDown == 1) {
         stop(0);
       }
       else {
        moveUp(0,0);
       }
       prevUpSwitchValue = LOW;
  }
  if (upSwitchValue1 == LOW and prevUpSwitchValue1 == HIGH ){
       Serial.println("button up 1 pushed");
        if (movingUp1 == 1 || movingDown1 == 1) {
           stop(1);
         }
         else {
          moveUp(0,1);
         }
         prevUpSwitchValue1 = LOW;
  }
  
  if ((downSwitchValue == LOW and prevDownSwitchValue == LOW ) || (prevDownSwitchValue1 ==LOW and downSwitchValue1 == LOW )){
      Serial.println("some button down still pushed");
  }
  
  if (downSwitchValue == LOW and prevDownSwitchValue == HIGH){
     Serial.println("pushed button down"); 
     if (movingUp == 1 || movingDown == 1) {
       stop(0);
     }
     else {
      moveDown(0,0);
     }
     prevDownSwitchValue = LOW;
  }
  if (downSwitchValue1 == LOW and prevDownSwitchValue1 == HIGH){
     Serial.println("pushed button 1 down"); 
     if (movingUp1 == 1 || movingDown1 == 1) {
       stop(1);
     }
     else {
      moveDown(0,1);
     }
     prevDownSwitchValue1 = LOW;
  }
  
  if (downSwitchValue == HIGH and prevDownSwitchValue == LOW){
    prevDownSwitchValue = HIGH;
    Serial.println("reset down button");
  }
  if (upSwitchValue == HIGH and prevUpSwitchValue == LOW){
    prevUpSwitchValue =HIGH;
    Serial.println("reset up button");
  }

  if (downSwitchValue1 == HIGH and prevDownSwitchValue1 == LOW){
    prevDownSwitchValue1 = HIGH;
    Serial.println("reset down button 1");
  }
  if (upSwitchValue1 == HIGH and prevUpSwitchValue1 == LOW){
    prevUpSwitchValue1 =HIGH;
    Serial.println("reset up button 1");
  }
  
  
  
  
  
  }


// the loop function runs over and over again forever
void loop() {

  //Serial.println(remainingStep);
  //if(digitalRead(GPIO03)==0)
  /*{
    client.publish("esp/test1", "Hello from ESP8266");
    delay(1000);
    }
    else;*/
  gettimeofday(&currentTime, NULL);
  int timeDiffmSec = (currentTime.tv_sec - previousTime.tv_sec) * 1000 + (currentTime.tv_usec - previousTime.tv_usec) / 1000;
  gettimeofday(&previousTime, NULL);

  if (WiFi.status() != WL_CONNECTED)
    {
      Serial.println("WiFi was not connected");
      WiFi_setup();
    }
  if (!client.connected()) {
    Serial.println("MQTT was not connected");
    MQTT_setup();
  }

  int endSwitchValue = digitalRead(ENDSWITCH);
  //at bottom possition 
  if (endSwitchValue == LOW){
    stop(0);
    position = 0;
    Serial.print("at bottom possition ");  
    //to not have the button pressed all the time
    moveUp(0,0);
    delay(100);
    stop(0);
  }
  endSwitchValue = digitalRead(ENDSWITCH1);
  //at bottom possition 
  if (endSwitchValue == LOW){
    stop(1);
    position1 = 0;
    Serial.print("at bottom possition side 1");  
    //to not have the button pressed all the time
    moveUp(0,1);
    delay(100);
    stop(1);
  }
  buttonHandle();
  client.loop();
  if (movingUp == 1) {
    position += timeDiffmSec * UpCoefficient;
    remainingStep-=timeDiffmSec * UpCoefficient;
    Serial.print("moving up, position: ");
    Serial.print(position);
    Serial.print(" remaining step:  ");
    Serial.println(remainingStep);
    reportPosition();
  }
  
  if (movingUp1 == 1) {
    position1 += timeDiffmSec * UpCoefficient;
    remainingStep1-=timeDiffmSec * UpCoefficient;
    Serial.print("moving up side 1, position: ");
    Serial.print(position1);
    Serial.print(" remaining step side 1:  ");
    Serial.println(remainingStep1);
    reportPosition();
  }
  if (movingDown == 1) {
    position -= timeDiffmSec;
    remainingStep-=timeDiffmSec;
    Serial.print("moving down, position: ");
    Serial.print(position);
    Serial.print(" remaining step:  ");
    Serial.println(remainingStep);
    
    reportPosition();
  }
  if (movingDown1 == 1) {
    position1 -= timeDiffmSec;
    remainingStep1-=timeDiffmSec;
    Serial.print("moving down 1, position: ");
    Serial.print(position1);
    Serial.print(" remaining step 1:  ");
    Serial.println(remainingStep1);
    
    reportPosition();
  }
  if (movingUp || movingDown) {
    if (remainingStep <= -10000 || position >= topPosition || (remainingStep <= 0 && moveToBottom !=1)) {
      stop(0);
      if (position <= 0) {
        position = 0;
      }
      if (position >= topPosition) {
        position = topPosition;
      }

    }
  }
  if (movingUp1 || movingDown1) {
    if (remainingStep1 <= -10000 || position1 >= topPosition1 || (remainingStep1 <= 0 && moveToBottom1 !=1)) {
      stop(1);
      if (position1 <= 0) {
        position1 = 0;
      }
      if (position >= topPosition) {
        position = topPosition;
      }

    }
  }

  delay(loopDelaymSec);
  //loop();
}
