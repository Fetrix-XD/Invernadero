#include <ESP8266WiFi.h>                                                //libreria WiFi   
#include <Ubidots.h>                                                    //libreria Ubidots
#include <DHT.h>                                                        //libreria DHT
#define DHTTYPE DHT22

#define DHTPIN 5                                                        //Define pin de sensor D1
#define tierra 4                                                        // d2

const char* UBIDOTS_TOKEN = "BBFF-SyxCGwE9oF8VZO1W1hhDUC8BpW3LSP";      //TOKEN de ubidots
String ssid     = "paloma1";                                            //red wifi
String password = "1223334444";                                         //constraseña wifi
Ubidots ubidots(UBIDOTS_TOKEN, UBI_HTTP);                          
DHT dht(DHTPIN, DHTTYPE, 22);                                           //se declara el sensor
//LDR                                                               
byte conts = 0;                                                         //conteo de intentos de conexion
byte max_int = 50;                                                      // maximos intentos

//CUBIERTA
int Paso [ 8 ][ 4 ] =                                                  //pasos
   {  {1, 0, 0, 0},
      {1, 1, 0, 0},
      {0, 1, 0, 0},
      {0, 1, 1, 0},
      {0, 0, 1, 0},
      {0, 0, 1, 1},
      {0, 0, 0, 1},
      {1, 0, 0, 1}
   };       
//pines PaP
#define IN1  0                                                       //declaracion de pines de motor
#define IN2  2
#define IN3  13
#define IN4  15


boolean Direction = true;                                            //se declara la variable de direccion
int Steps = 0;                                                        
int steps_left = 0;                                                 //los pasos a dar  

                                                        
void setup() {                                                                                                                              
  Serial.begin(9600);                                                 //se inicia comunicacion serial                                 
  dht.begin();
  pinMode(14, OUTPUT);                                                 //pin de la bomba
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);  
  pinMode(2 , OUTPUT);                                                        //pin del on/off de motor
  WiFi.begin(ssid ,password);
  while (WiFi.status() !=WL_CONNECTED and conts <max_int){          //bucle de conexion wifi
    conts++;
    delay(500);
    Serial.print(".");
    Serial.print("");
    if(conts < max_int){
      Serial.println("*************************************************");
      Serial.print(  "Conectado a la red: ");
      Serial.println(WiFi.SSID());
      Serial.print("IP: ");
      Serial.println(WiFi.localIP());
      Serial.print("macAdress: ");
      Serial.println(WiFi.macAddress());
      Serial.println("*************************************************");
    }
    else{
      Serial.println("*************************************************");
      Serial.println("Error de conexion");
      Serial.println("*************************************************");
    }
  }
}
int cont = 3; 
void loop() {
  
  //LDR
  int sensorValue = analogRead(A0);                             // entrada analogica
  float voltage = sensorValue * (3.3 / 1023.0);                 //ecuacion para transformar el valor a voltaje entre 0 a 3.3
  
  //DHT 22
  float h = dht.readHumidity();                                 //se determina variable de humedad
  float t = dht.readTemperature();                              //se determina variable de temperatura
  
  //se escribe en el monitor en serie
  Serial.println("Humedad: ");
  Serial.println(h);
  Serial.println("Temperatura: ");
  Serial.println(t);
  //tierra
  int valor = digitalRead(tierra);                            //se lee estado logico de sensor de humedad
  if ((voltage) < 0.10) {
    Serial.println("Luz: ");
    Serial.println("Oscuro");                                 //informa tipo de iluminacion que recibe el cultivo
    if ((valor) == 1) {
      Serial.println("Humedad de suelo: ");
      Serial.println("Seco");                                 //informa de humedad de suelo
      digitalWrite(14, HIGH);                                  //Se activa bomba de riego D0
    }
    else {
      Serial.println("Humedad de suelo: ");
      Serial.println("Humedo");                               //informa de humedad de suelo
      digitalWrite(14, LOW);                                   //Se desactiva la bomba de riego D0
    }
  }
  else {
    Serial.println("Luz: ");
    Serial.println("Iluminado");                              //informa tipo de iluminacion que recibe el cultivo
    digitalWrite(14, LOW);
    if ((valor) == 1) {
      Serial.println("Humedad de suelo: ");
      Serial.println("Seco");                                 //informa de humedad de suelo
                                                              //Se activa bomba de riego D0
    }
    else {
      Serial.println("Humedad de suelo: ");
      Serial.println("Humedo");                               //informa de humedad de suelo
                                                             //Se desactiva la bomba de riego D0
    }
  }

  //CUBIERTA
  if( t < 2.00 or t > 30.00){
    if(Direction){
      cont = 1;
    }
  }
  
  if (cont == 1){
    if (t < 2.00 or t > 30.00){
      steps_left = 25000;
      Direction = false;
      Giro();
      cont = 2;
    }else{
      steps_left = 0;
      Giro();
    }
  }else if( cont == 2){
    if( t > 2.00 and t < 30.00){
      steps_left = 26000;
      Direction = true;
      Giro();
      cont = 3;
    }
    else{
      steps_left = 0;
      Giro();
    }
  }

  //SE ENVIA DATOS A UBIDOTS
  ubidots.add("Humedad",h);
  ubidots.add("Temperatura",t);
  ubidots.add("Humedad de suelo", valor);

  //Se verifica envio de datos
  bool bufferSent = false;
  bufferSent = ubidots.send();
  if (bufferSent){
    Serial.println("Valores enviados correctamente");
  }else{
    Serial.println("Error valores no enviados");
  }
  delay(5000);
}

void Giro(){
  while(steps_left >0){
      stepper() ;     // Avanza un paso
      
      steps_left-- ;  // Un paso menos
      delay(1);}
     
}
void stepper()            //Avanza un paso
   {  digitalWrite( IN1, Paso[Steps][ 0] );
      digitalWrite( IN2, Paso[Steps][ 1] );
      digitalWrite( IN3, Paso[Steps][ 2] );
      digitalWrite( IN4, Paso[Steps][ 3] );

      SetDirection();
   }
void SetDirection()
  {  
  if(Direction)
      Steps++;
  else
      Steps--;
      Steps = ( Steps + 8 ) % 8 ;
   }
