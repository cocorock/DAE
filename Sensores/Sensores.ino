#include <Wire.h>
#include <BMA222.h>
#include <Adafruit_TMP006.h>
#include <WiFi.h>

// your network name also called SSID
char ssid[] = "galileo";
// your network password
char password[] = "elec2012";
// initialize the library instance:
WiFiClient client;
IPAddress server(52,8,0,240); //  exosite IP Address
#define cik "52a19b371125949dbc77035334388e06dc7aebb5" // exosite identifier

String accXalias= "accX=";
String accYalias= "accY=";
String accZalias= "accZ=";
String tempAlias= "temp=";
String alarmAlias= "alarm=";
String alarmAccAlias= "alarmAcc=";
String alarmMovAlias= "alarmMov=";
String alarmTempAlias= "alarmTemp=";
String armedAlias = "armado=";
char buffer[256]={0};


BMA222 mySensor;
//Adafruit_TMP006 tmp006;
Adafruit_TMP006 tmp006(0x41);

int8_t armed = 0;
int8_t muchacho = 0;
int8_t alarm = 0;
int8_t alarmacc = 0;
int8_t alarmtemp = 0;
int8_t accX = 0;
int8_t accY = 0;
int8_t accZ = 0;
float sensortemp = 0.0f;

void setup() {
  // initialize the LED pin as an output:
  pinMode(RED_LED, OUTPUT); 
  pinMode(GREEN_LED, OUTPUT);
  pinMode(8, OUTPUT); 
  digitalWrite(GREEN_LED,LOW);
  // initialize the pin as an input:
  pinMode(2, INPUT);
  pinMode(PUSH2, INPUT_PULLUP);  
  //serial 
  Serial.begin(115200);
  //accelerometer
  mySensor.begin();
  //temperature
  tmp006.begin();
  
  // attempt to connect to Wifi network:
  Serial.print("Attempting to connect to Network named: ");
  // print the network name (SSID);
  Serial.println(ssid); 
  // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
  WiFi.begin(ssid);
  while ( WiFi.status() != WL_CONNECTED) {
    // print dots while we wait to connect
    Serial.print(".");
    delay(300);
  }
  Serial.println("\nYou're connected to the network");
  Serial.println("Waiting for an ip address");
  while (WiFi.localIP() == INADDR_NONE) {
    // print dots while we wait for an ip addresss
    Serial.print(".");
    delay(300);
  }
  Serial.println("\nIP Address obtained");
  printWifiStatus();
  //5 seconds delay
  delay(5000);
}

void loop(){
  //armar alarma
  
  if (armed) digitalWrite(8, HIGH);
  else digitalWrite(8, LOW);
  
  //movimiento
  muchacho = digitalRead(2);
  if (muchacho == HIGH){
    digitalWrite(RED_LED, LOW);
    alarm = 0;
  }
  else{
    digitalWrite(RED_LED, HIGH);
    alarm = (armed)?1:0;
  }
  
  //aceleeracion
  accX = mySensor.readXData();
  //Serial.print("X: ");
  //Serial.print(accX);

  accY = mySensor.readYData();
  //Serial.print(" Y: ");
  //Serial.print(accY);

  accZ = mySensor.readZData();
  //Serial.print(" Z: ");
  //Serial.println(accZ);
  
  if ((accX>32)||(accX<-32)||(accY>32)||(accY<-32)||(accZ>96)||(accZ<32)) alarmacc = (armed)?1:0;
  else alarmacc = 0;
  
  //temperatura
  sensortemp = tmp006.readObjTempC();
  //Serial.print("Temp: "); 
  //Serial.print(sensortemp); 
  //Serial.println("*C");
  
  if (sensortemp > 50.0f) alarmtemp = (armed)?1:0;
  else alarmtemp= 0;
  
  
  //=================================================================
  //Recived data from Server
  readServer("armado");   //check armado switch //alarmAcc&alarmMov&alarmTemp&
  delay(1000);
  int idx = 0;
  while (client.available()) {
    char c = client.read();
    //Serial.print(c);
    buffer[idx++]=c;
  }
    Serial.print("buffer idx= ");
    Serial.println(idx);
    idx = 90;
    while(buffer[idx]){
      Serial.print(buffer[idx++]);
    }
    Serial.print("\nInput Buffer: ");Serial.println(buffer[97]);
    armed = (buffer[97]=='1')?1:0;
    
    //convert data to string
    String  ax = String(accX);
    String  ay = String(accY);
    String  az = String(accZ);
    String  t = String((int)sensortemp);
    String  alarmStr = String(alarm);
    String  armedStr = String(armed);
    String  alarmaccStr = String(alarmacc);
    String  alarmtempStr = String(alarmtemp);
    //make string 
    String datos;
    
   datos = accXalias+ax
        +"&"+accYalias+ay
        +"&"+accZalias+az
        +"&"+tempAlias+t
        +"&"+alarmAlias+alarmStr
        +"&"+alarmAccAlias+alarmaccStr
        +"&"+alarmTempAlias+alarmtempStr;
    
    Serial.println(" datos: "+datos);
    writeServer(datos);
    
    
  
}

int readServer(String alias){
  client.stop();
  if (client.connect(server, 80)) {
     Serial.print("\nRead connecting...");
     Serial.println(server);
     //======= READ
    client.print("GET /onep:v1/stack/alias?");
    client.print(alias);
    client.println(" HTTP/1.1");
    client.println("Host: m2.exosite.com");
    client.print("X-Exosite-CIK: ");
    client.println(cik);
    client.println("Accept: application/x-www-form-urlencoded; charset=utf-8");
    client.println("");
  }else{
      Serial.println("connection failed while reading form server");
      Serial.println();
      Serial.println("disconnecting.");
      client.stop();
      return -1;
   }
   return 1;
}

int writeServer(String aliasData){
  client.stop();
  if (client.connect(server, 80)) {
     Serial.print("\nWrite connecting...");
     Serial.println(server);
      Serial.print("String data lenght = ");
      Serial.println(aliasData.length());
      //======= WRITE
      client.println("POST /onep:v1/stack/alias HTTP/1.1");
      client.println("Host: m2.exosite.com");
      client.print("X-Exosite-CIK: ");
      client.println(cik);
      client.println("Content-Type: application/x-www-form-urlencoded; charset=utf-8");
      client.print("Content-Length: ");
      client.println(aliasData.length());
      client.println("");
      client.println(aliasData);
  }else{
      Serial.println("connection failed while writing to server");
      Serial.println();
      Serial.println("disconnecting.");
      return -1;
   }
   client.stop();
   return 1;
}

void printWifiStatus() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
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

