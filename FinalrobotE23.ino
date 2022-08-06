#include <WiFi.h> 
#include <PubSubClient.h> 
#include <Servo.h>
#include <math.h>

Servo motor1; // para trabajar con servo 1
Servo motor2; // para trabajar con servo 2

// Update these with values suitable for your network.
// Contraseña y red utilizada en el instituto donde está conectado el dispositivo
const char* ssid = "¿Quién va a reprobar?";
const char* password = "todoelgrupo";
const char* mqtt_server = "broker.emqx.io";


//const char* ssid = "INFINITUM6211_2.4";
//const char* password = "1ciGnkCo4e";
//const char* mqtt_server = "127.0.0.1";
//const char* mqtt_server = "broker.emqx.io";

WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE  (50)
//#define BUILTIN_LED
char msg1[MSG_BUFFER_SIZE]; //variable para almacenar posición 1
char msg2[MSG_BUFFER_SIZE]; //variable para almacenar posición 1
int value = 0;     //variable incremental
int pot1 = 0;     //variable para la lectura del potenciometro1
int pot2 = 0;     //variable para la lectura del potenciometro1
//int pot3 = 0;
int i;            //Variable incremental
int o;            //variable para guardar el signo positivo o negativo
int p;            //variable para guardar el signo positivo o negativo
int q;            //variable para guardar el signo positivo o negativo
int r;            //variable para guardar el signo positivo o negativo
float v1=0;       //variable para el voltaje 1
float v2=0;       //variable para el voltaje 1
float v3=0;       //variable para el voltaje 1
int angulo1=0;    //variable para el angulo 1
int angulo2=0;    //variable para el angulo 1
int angulo3=0;    //variable para el angulo 1
byte angulos;     //variable para el angulo 1

float tf=2;//tiempo final de ejecución en segundos
float x=0,y=0; //coordenadas generadas de la trayectoria recta
float xi=16,yi=0,xf=16,yf=0; //coordeandas iniciales y finales (para iniciar son las mismas)
float t=0; //tiempo de ejecución (este se recalculará en la función rectaGenerada)
float theta1=0,theta2=0; // angulos para el movimiento del servo motor
int posi =90;   //angulo de compensación inicial para que el brazo quede recto
int DEL = 10;   //variable de tiempo
float deltaT=0.01; //Incrementos de tiempo en segundos

void setup_wifi()  //función Wifi
{
  delay(10);  //pausa
  // We start by connecting to a WiFi network
  Serial.println();                  
  Serial.print("Connecting to ");   //Impresión en puerto serial de conección
  Serial.println(ssid);            //Impresión en puerto serial del ssis

  WiFi.mode(WIFI_STA);            //Libreria Wifi
  WiFi.begin(ssid, password);     // Configuración de wifi

  while (WiFi.status() != WL_CONNECTED)   //Estado de conección
  {
    delay(500);             //Pausa
    Serial.print(".");      //impresión en puerto serial
  }

  randomSeed(micros());

  Serial.println("");                //impresión en puerto serial
  Serial.println("WiFi connected");  //impresión en puerto serial
  Serial.println("IP address: ");    //impresión en puerto serial
  Serial.println(WiFi.localIP());    //impresión en puerto serial
}

float callback1(char* topic, byte* payload, unsigned int length) //resepción de de datos desde el broker
{  
  Serial.print("Message arrived ["); //impresión en puerto serial
  Serial.print(topic);               //impresión en puerto serial
  Serial.println("] ");
  for (int i = 0; i < length; i++)       //Lectura de las coordenadas
  {
    Serial.println((char)payload[i]);
  }

  int a=payload[1]-48;               //Conversión del primer digito de la primera coordenada de codigo ascii a decimal
  int b=payload[2]-48;               //Conversión del segundo digito de la primera coordenada de codigo ascii a decimal
  int c=payload[5]-48;               //Conversión del primer digito de la segunda coordenada de codigo ascii a decimal
  int d=payload[6]-48;               //Conversión del segundo digito de la segunda coordenada de codigo ascii a decimal

  int e=payload[9]-48;               //Conversión del primer digito de la tercera coordenada de codigo ascii a decimal
  int f=payload[10]-48;              //Conversión del segundo digito de la tercera coordenada de codigo ascii a decimal
  int g=payload[13]-48;              //Conversión del primer digito de la cuarta coordenada de codigo ascii a decimal
  int h=payload[14]-48;              //Conversión del segundo digito de la cuarta coordenada de codigo ascii a decimal

  int j=payload[0];                  //Conversión de signo de la primera coordenada de codigo ascii a decimal
  int k=payload[4];                  //Conversión de signo de la segunda coordenada de codigo ascii a decimal
  int l=payload[8];                  //Conversión de signo de la tercera coordenada de codigo ascii a decimal
  int m=payload[12];                 //Conversión de signo de la cuarta coordenada de codigo ascii a decimal


  if (j==45)                      //Verificación del signo negativo de la primera coordenada
  {
    o=(j-45)-1;
  }
  if (j==43)                     //Verificación del signo positivo de la primera coordenada
  {
    o=(j-43)+1;
  }
  
  if (k==45)                     //Verificación del signo negativo de la segunda coordenada
  {
    p=(k-45)-1;
  }
  if (k==43)                    //Verificación del signo positivo de la segunda coordenada
  {
    p=(k-43)+1;
  }
  
  if (l==45)                  //Verificación del signo negativo de la tercera coordenada
  {
    q=(l-45)-1;
  }
  if (l==43)                //Verificación del signo positivo de la tercera coordenada
  {
    q=(l-43)+1;
  }

  if (m==45)               //Verificación del signo negativo de la cuarta coordenada
  {
    r=(m-45)-1;
  }
  if (m==43)              //Verificación del signo positivo de la cuarta coordenada
  {
    r=(m-43)+1;
  }


  xi=(((a*10)+b)*o);   //Obtención de la primera derivada
  yi=(((c*10)+d)*p);   //Obtención de la segunda derivada

  xf=(((e*10)+f)*q);   //Obtención de la tercera derivada
  yf=(((g*10)+h)*r);  //Obtención de la cuarta derivada
  

  Serial.println(a);
  Serial.println(b);
  Serial.println(c);
  Serial.println(d);
  Serial.println(xi);
  Serial.println(yi);
  Serial.println(xf);
  Serial.println(yf);

  return xi,yi,xf,yf;   //Retorno de coordenadas procesadas que vienen de mensaje externo por MQTT
}
 


void reconnect()                              //Función de reconección
{
  // Loop until we're reconnected
  while (!client.connected())                     //Verificación de conección
  {
    Serial.print("Attempting MQTT connection...");    // Create a random client ID
    
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);        // Attempt to connect
   
    if (client.connect(clientId.c_str()))      //Verificación de conección
    {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish("posicion1", "Enviando el primer mensaje");        //Enviando el mensaje de posición1 
      client.publish("posicion2", "Enviando el primer mensaje");        //Enviando el mensaje de posición2 
      // ... and resubscribe
      client.subscribe("coordenadas");       //Subcripción
    } 
    else 
    {
      Serial.print("failed, rc=");                     //Impresión en puerto serial
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}


void setup() 
{
  Serial.begin(115200); //velocidad de comunicación serial
  setup_wifi();                                 //Inicialización de Wifi 
  client.setServer(mqtt_server, 1883);          //Inicialización de MQTT
  client.setCallback(callback1);

  motor1.attach(15);     //Inicialización en pin del servo 1
  motor2.attach(2);      //Inicialización en pin del servo 1
  //llevar a posición de descanso al robot, cambiar según el angulo adecuado
  //Donde posi=90 grados hace que el brazo empieze estirado
  motor1.write(posi); 
  motor2.write(posi);
  delay(2000); //Tiempo de espera de 2 segundos
  tf=2; //Tiempo de ejecución en caso de cambiarlo respecto del default
}

void loop() 
{
//Reconexión
  if (!client.connected()) 
  {
    reconnect();
  }
  client.loop(); 


  pot1 = analogRead(36);      //Lee la posición 1
  pot2 = analogRead(39);      //Lee la posición 2
 
  v1=pot1*(3.3/4095);        //Voltaje de la entrada posición1
  v2=pot2*(3.3/4095);        //Voltaje de la entrada posición1

  angulo1=(v1*180)/3.3;      //Calculo de angulo de la posición1
  angulo2=(v2*180)/3.3;      //Calculo de angulo de la posición2


/*En esta sección usamos la función rectaGenerada que recibe los mensajes
desde internet para cambiar las trayectorias que realize el robot, 
esta función se usa dentro de un ciclo for para hacer el recorrido según 
las especificaciones diseñadas de coordenadas iniciales-finales así como el tiempo de ejecución*/
for(t=0;t<=tf;t+=deltaT)
{
  rectaGenerada(tf,t,xi,yi,xf,yf); //cada iteración crea nuevo par coordenada a seguir
  inversaRR(x,y); //por cada par generado calcula cinemática inversa para ángulos
  motor1.write(posi+theta1); //envía ángulos adecuados a servomotores 1 y 2
  motor2.write(posi+theta2);
  delay(DEL);
}
  
  
  unsigned long now = millis();       //Envio de datos con MQTT
  if (now - lastMsg > 2000) 
  {
    lastMsg = now;
    ++value;
    //snprintf (msg1, MSG_BUFFER_SIZE, "%ld, %ld, %ld", angulo1, angulo2, angulo3);
    snprintf (msg1, MSG_BUFFER_SIZE, "%ld", angulo1);
    snprintf (msg2, MSG_BUFFER_SIZE, "%ld", angulo2);
    
    
    client.publish("posicion1", msg1);      //Envio de posición 1
    client.publish("posicion2", msg2);      //Envio de posición 2
   
  }
    
}

//Función de generación de trayectoria recta MRU (movimiento rectilineo uniforme)
float rectaGenerada(float tf,float t,float xi,float yi,float xf,float yf)
{
//se divide para trabajar en metros, ya que se recibe en centímetros
xi=xi/100; 
yi=yi/100;
xf=xf/100;
yf=yf/100;
//Cálculo de velocidades cartesianas
  float vx=(xf-xi)/tf; 
  float vy=(yf-yi)/tf;
//Generación de puntos en cada instante de tiempo discreto "t"
  x=xi+vx*t;
  y=yi+vy*t;
  return x,y;
}

//Función de cinemática inversa
float inversaRR(float x,float y)
{
  float L1=0.08; //largo de eslabones 1 y 2
  float L2=0.08;
  theta2=acos((pow(x,2)+pow(y,2)-pow(L1,2)-pow(L2,2))/(2*L1*L2)); //calculo de ángulos
  theta1=atan(y/x)-atan((L2*sin(theta2))/(L1+L2*cos(theta2)));
  //conversion a grados para servos
  theta1=round(180*(theta1/3.14));
  theta2=round(180*(theta2/3.14));
  return theta1,theta2; 
}
