#include <WiFi.h>
#include <PubSubClient.h>
#include <Servo.h>
#include <math.h>

Servo motor1;
Servo motor2;

// Update these with values suitable for your network.

//const char* ssid = "¿Quién va a reprobar?";
//const char* password = "todoelgrupo";
//const char* mqtt_server = "broker.emqx.io";

const char* ssid = "INFINITUM6211_2.4";
const char* password = "1ciGnkCo4e";
//const char* mqtt_server = "127.0.0.1";
const char* mqtt_server = "broker.emqx.io";

WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE  (50)
//#define BUILTIN_LED
char msg1[MSG_BUFFER_SIZE];
char msg2[MSG_BUFFER_SIZE];
int value = 0;
int pot1 = 0;
int pot2 = 0;
int pot3 = 0;
int i;
int o;
int p;
int q;
int r;
float v1=0;
float v2=0;
float v3=0;
int angulo1=0;
int angulo2=0;
int angulo3=0;
byte angulos;

float tf=0;//tiempo final de ejecución
float x=0,y=0; //coordenadas generadas
int xi,yi,xf,yf; //coordeandas iniciales y finales
float t=0; //tiempo de ejecución
float theta1=0,theta2=0;
int posi =90;
int DEL = 10;
float deltaT=0.01;

void setup_wifi() 
{

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) 
  {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

int callback1(char* topic, byte* payload, unsigned int length) //resepción de de datos desde el broker
{  
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.println("] ");
  for (int i = 0; i < length; i++) 
  {
    Serial.println((char)payload[i]);
  }

  
  int a=payload[1]-48;
  int b=payload[2]-48;
  int c=payload[5]-48;
  int d=payload[6]-48;

  int e=payload[9]-48;
  int f=payload[10]-48;
  int g=payload[13]-48;
  int h=payload[14]-48;

  int j=payload[0];
  int k=payload[4];
  int l=payload[8];
  int m=payload[12];


  if (j==45)
  {
    o=(j-45)-1;
  }
  if (j==43)
  {
    o=(j-43)+1;
  }
  
  if (k==45)
  {
    p=(k-45)-1;
  }
  if (k==43)
  {
    p=(k-43)+1;
  }
  
  if (l==45)
  {
    q=(l-45)-1;
  }
  if (l==43)
  {
    q=(l-43)+1;
  }

  if (m==45)
  {
    r=(m-45)-1;
  }
  if (m==43)
  {
    r=(m-43)+1;
  }


  xi=((a*10)+b)*o;
  yi=((c*10)+d)*p;

  xf=((e*10)+f)*q;
  yf=((g*10)+h)*r;
  

  Serial.println(a);
  Serial.println(b);
  Serial.println(c);
  Serial.println(d);
  Serial.println(xi);
  Serial.println(yi);
  Serial.println(xf);
  Serial.println(yf);

  return xi,yi,xf,yf;
}
 


void reconnect() 
{
  // Loop until we're reconnected
  while (!client.connected()) 
  {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str())) 
    {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish("posicion1", "Enviando el primer mensaje");
      client.publish("posicion2", "Enviando el primer mensaje");
      // ... and resubscribe
      client.subscribe("coordenadas");
    } 
    else 
    {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup() 
{
  //pinMode(5, OUTPUT);     // Initialize the BUILTIN_LED pin as an output
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback1);

  motor1.attach(15);
  motor2.attach(2);
  //levar a posición de descanso al robot, cambiar según el angulo adecuado
  motor1.write(posi);
  motor2.write(posi);
  delay(2000);
  tf=1;

  for(t=0;t<=tf;t+=deltaT)
  {
    rectaGenerada(tf,t,0.16,0.0,0.12,0.0);
    inversaRR(x,y);
    motor1.write(posi+theta1);
    motor2.write(posi+theta2);
    delay(DEL);
  }
    
  for(t=0;t<=tf;t+=deltaT)
  {
    rectaGenerada(tf,t,0.12,0.0,0.16,0.0);
    inversaRR(x,y);
    motor1.write(posi+theta1);
    motor2.write(posi+theta2);
    delay(DEL);
  }
}

void loop() 
{

  if (!client.connected()) 
  {
    reconnect();
  }
  client.loop();

//Para el envio de datos
  pot1 = analogRead(36);
  pot2 = analogRead(39);
  pot3 = analogRead(34);

  v1=pot1*(3.3/4095);
  v2=pot2*(3.3/4095);
  v3=pot3*(3.3/4095);

  angulo1=(v1*180)/3.3;
  angulo2=(v2*180)/3.3;
  angulo3=(v3*180)/3.3;

  
  
  unsigned long now = millis();
  if (now - lastMsg > 2000) 
  {
    lastMsg = now;
    ++value;
    //snprintf (msg1, MSG_BUFFER_SIZE, "%ld, %ld, %ld", angulo1, angulo2, angulo3);
    snprintf (msg1, MSG_BUFFER_SIZE, "%ld", angulo1);
    snprintf (msg2, MSG_BUFFER_SIZE, "%ld", angulo2);
    
    
    client.publish("posicion1", msg1);
    client.publish("posicion2", msg2);
   
  }
    
}

float rectaGenerada(float tf,float t,int xi,int yi,int xf,int yf)
{
  float vx=(xf-xi)/tf;
  float vy=(yf-yi)/tf;
  x=xi+vx*t;
  y=yi+vy*t;
  return x,y;
}

float inversaRR(float x,float y)
{
  float L1=0.08;
  float L2=0.08;
  theta2=acos((pow(x,2)+pow(y,2)-pow(L1,2)-pow(L2,2))/(2*L1*L2));
  theta1=atan(y/x)-atan((L2*sin(theta2))/(L1+L2*cos(theta2)));
  //conversion a grados par aservos
  theta1=round(180*(theta1/3.14));
  theta2=round(180*(theta2/3.14));
  return theta1,theta2; 
}
