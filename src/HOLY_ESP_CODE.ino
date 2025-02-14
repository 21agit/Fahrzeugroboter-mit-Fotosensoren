//////////////////////////////////////////////
// RemoteXY include library //
//////////////////////////////////////////////
// RemoteXY select connection mode and include library

#define REMOTEXY_MODE__ESP32CORE_BLE
#include <BLEDevice.h>
#include <RemoteXY.h>

#include <ESP32Servo.h>

// RemoteXY connection settings
#define REMOTEXY_BLUETOOTH_NAME "BMW M5 CS"
#define REMOTEXY_ACCESS_PASSWORD "uzunhortum"

// RemoteXY configurate
#pragma pack(push, 1)
uint8_t RemoteXY_CONF[] = // 94 bytes
 { 255,5,0,0,0,87,0,16,29,1,2,1,3,71,29,11,2,26,31,31,
 83,101,110,115,111,114,0,80,104,111,110,101,0,10,48,38,74,19,19,4,
 36,8,32,71,79,32,0,24,83,84,79,80,0,2,1,3,88,29,8,2,
 26,31,31,77,111,116,46,79,78,0,77,111,116,46,111,102,102,0,4,48,
 10,3,10,64,2,26,4,48,40,3,10,64,2,26 }; 

 // this structure defines all the variables and events of your control interface
struct {
  // input variables
  uint8_t switch_1; // =1 if switch ON and =0 if OFF
  uint8_t pushSwitch_Engine; // =1 if state is ON, else =0
  uint8_t switch_2; // =1 if switch ON and =0 if OFF
  int8_t slider_links; // =-100..100 slider position
  int8_t slider_rechts; // =-100..100 slider position
  // other variable
  uint8_t connect_flag; // =1 if wire connected, else =0
} RemoteXY;
#pragma pack(pop)
/////////////////////////////////////////////
// END RemoteXY include //
/////////////////////////////////////////////


int PIN_PUSHSWITCH_1 = 10;
int PIN_SWITCH_2 = 9;

int SensorLinks = 0;
int SensorRechts = 1;

//Note: Hinten und Vorne ist vertauscht
int Motor_rechts_vorne = 3; // GPIO3
int Motor_rechts_hinten = 2; // GPIO2
int Motor_links_vorne = 9; // GPIO9  
int Motor_links_hinten = 8; // GPIO8 


Servo myservo; 
const int servoPin = 19; 
int val;



void setup()
{
  RemoteXY_Init ();
  
  pinMode(SensorLinks, INPUT);
  pinMode(SensorRechts, INPUT);
  
  pinMode(Motor_rechts_vorne, OUTPUT);
  pinMode(Motor_rechts_hinten, OUTPUT);
  pinMode(Motor_links_vorne, OUTPUT);
  pinMode(Motor_links_hinten, OUTPUT);
  
  myservo.setPeriodHertz(50); // Perioden-Dauer einstellen
  myservo.attach(servoPin); 
  Serial.begin(9600);  
}

void loop()
{
  RemoteXY_Handler ();
  
  //Alles in den seriellen Monitor drucken
  Serial.println("---------------------------------");
  Serial.print("Slider-Links = ");
  Serial.print(RemoteXY.slider_links);
  Serial.print("Slider-Rechts = ");
  Serial.println(RemoteXY.slider_rechts);
  
  Serial.print("Sensor-Rechts: ");
  Serial.println(analogRead(SensorRechts));
  
  Serial.print("Sensor-Links: ");
  Serial.println(analogRead(SensorLinks));
  
  //Ternary Operator
  //Condition ? if true : else
  Serial.println(RemoteXY.switch_1==0 ? "Phone":"Sensor");
  
  Serial.println(RemoteXY.switch_2==0 ? "Motor aus":"Motor an");
  
  Serial.println(RemoteXY.pushSwitch_Engine==0 ? "Stop":"Go");
  
  //In jeder Iteration testen ob der Stop/Go Button gedrückt wurde
  if(RemoteXY.pushSwitch_Engine==0)
  { val = 3500; } else
  {  val = 1000; }
  
  // Abbilden des Wertes „val“ aus dem Bereich 0 – 4096 auf 0 - 180
  val = map(val,0,4096, 0, 180); 
  myservo.write(val); // “val” an den servo ausgeben.
  
  
  //Wenn Motor aus dann setzt alle Pins auf 0 und return aus der Funktion
  if(!RemoteXY.switch_2)
  {
    analogWrite(Motor_links_vorne, 0);  
    
    analogWrite(Motor_links_hinten, 0);  
    analogWrite(Motor_rechts_vorne, 0);  
    
    analogWrite(Motor_rechts_hinten, 0); 
    return;
  } 
  
  
  //Ab diesem Wert soll der ESP zwischen Schwarz und weiß unterscheiden
  const int schwellenwert = 1500;
  
  
  
  
  
  //Sensormodus
  if(RemoteXY.switch_1)
  {
    //Wenn Rechts schwarz und links weiß
    while(analogRead(SensorRechts) > schwellenwert && analogRead(SensorLinks) < schwellenwert)
    {
      analogWrite(Motor_links_vorne, 0);  
      analogWrite(Motor_links_hinten, 200);  
      analogWrite(Motor_rechts_vorne, 0);  
      analogWrite(Motor_rechts_hinten, 0);  
    }
    
    //Wenn Rechts weiß und links schwarz
    while(analogRead(SensorRechts) < schwellenwert && analogRead(SensorLinks) > schwellenwert)
    {
      analogWrite(Motor_rechts_vorne, 0);  
      analogWrite(Motor_rechts_hinten, 200);  
      analogWrite(Motor_links_vorne, 0);  
      analogWrite(Motor_links_hinten, 0);
    }
    
    //Wenn Rechts weiß und links weiß
    while(analogRead(SensorRechts) < schwellenwert && analogRead(SensorLinks) < schwellenwert)
    {
      analogWrite(Motor_rechts_vorne, 0);  
      analogWrite(Motor_rechts_hinten, 200);  
      analogWrite(Motor_links_vorne, 0);  
      analogWrite(Motor_links_hinten, 200); 
    }
    
    //Wenn Rechts schwarz und links schwarz
    while(analogRead(SensorRechts) > schwellenwert && analogRead(SensorLinks) > schwellenwert)
    {
      analogWrite(Motor_rechts_vorne, 0);      
      analogWrite(Motor_rechts_hinten, 0);  
      analogWrite(Motor_links_vorne, 0);  
      analogWrite(Motor_links_hinten, 0); 
    }
  } 
  
  
  
  
  
  
  //Handymodus
  //NOTE: x und y müssen mit 2.55 multipliziert werden weil analogWrite() Werte von 0 bis 255 nimmt
  else if(!RemoteXY.switch_1)
  {
    
    int x = 2.55 * RemoteXY.slider_links;
  
    
    if(x == 0)
    {  
      digitalWrite(Motor_links_vorne, LOW);
      digitalWrite(Motor_links_hinten, LOW);
      
      digitalWrite(Motor_rechts_vorne, LOW);
      digitalWrite(Motor_rechts_hinten, LOW);
    }
    
    if(x > 0)
    {
      analogWrite(Motor_rechts_hinten, x);
      analogWrite(Motor_links_hinten, x);
      
      digitalWrite(Motor_rechts_vorne, LOW);
      digitalWrite(Motor_links_vorne, LOW);
    } 
    else if(x < 0) 
    {
      analogWrite(Motor_rechts_vorne, abs(x));
      analogWrite(Motor_links_vorne, abs(x));
      
      digitalWrite(Motor_rechts_hinten, LOW);
      digitalWrite(Motor_links_hinten, LOW);
    
    }
    
    int y = 2.55 * RemoteXY.slider_rechts;
    
    if(y > 0)
    {
    analogWrite(Motor_rechts_hinten, 0);
    analogWrite(Motor_rechts_vorne, 0);
    } else if(y < 0) {
    
    analogWrite(Motor_links_hinten, 0);
    analogWrite(Motor_links_vorne, 0);
    }
  
  
  
  }

}
