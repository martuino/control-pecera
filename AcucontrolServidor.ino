/*
Acucontrol

 */

#include <WiFi.h>
#include <WiFiClient.h>
#include <WiFiServer.h>
#include <OneWire.h>
#include <wire.h>
//#include <RTClib.h>
#include <DS1307RTC.h>
#include <SLFS.h>
#include <inc/hw_types.h>
#include <driverlib/prcm.h>
#include <Timelib.h>


// El nombre de tu red (SSID) y la contraseña
char ssid[] = "MiFibra-72D6";
char password[] = "hheaCR7w";
char IPluces[] = "192.168.1.106";
int keyIndex = 0;
const int analogInPin1 = 16; //Sonda PH
const int analogInPin2 = A1; 
const int analogInPin3 = A2; 
int sensorReading = 0;
float celsius = 0; //Temperatura recogida por 18B20
String fecha = "";
const int releco2 = 9; //JP1-1 Rele CO2
const int esterilizador = 10;//Jp1-2 Rele estelirizador UV
const int jp1rele3 = 2; //JP1-3 Rele 3
const int jp1rele4 = 3; //JP1-4 Rele 4
const int calentador = 4; //JP2-1 Rele calentador
const int comedero = 5; //JP2-2 Rele 2
const int dosificadora = 6; //JP2-3 Rele dosificadora +PH
const int jp2rele4 = 7; //JP2-4 Rele 4
const int LimitPH = 8;
int valors1 = 0;
int valors2 = 0;
int valors3 = 0;
float tempcale = 0;
int phmin = 0;
int phdosi = 0;
int Memo[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
//******GUARDAR***************
int Memominu[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
int Memohora[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
int Memovalor1[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
int Memovalor2[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
int Memovalor3[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
boolean hubocambio = false;
int Nummemo = 0;
int cont = 0;
int Numactual = 10;
int misminutos = 0;
int misminutos2 = 0;
int difvalor1 = 0;
int difvalor2 = 0;
int difvalor3 = 0;
int difMinuto1 = 0;
int difMinuto2 = 0;
int difMinuto3 = 0;
boolean difneg1 = false;
boolean difneg2 = false;
boolean difneg3 = false;
int cntminu1 = 0;
int cntminu2 = 0;
int cntminu3 = 0;
int cntsel = 0;
int pos = 0;
int minutoAnterior = 0;
uint8_t flash_buffer[32]; //Se guarda la hora en la flash
uint8_t flash_buffer2[32]; //Se guarda los minutos en la flash
uint8_t flash_buffer3[32]; //Se guarda los valores en la flash
uint8_t flash_buffer4[32]; //Se guarda los valores en la flash
uint8_t flash_buffer5[32]; //Se guarda los valores en la flash
uint8_t flash_buffer7[8]; //Se guarda los valores actuales en flash
int Horaactual = 0;
int Minutoactual = 0;
String titulo = "Control Acuario Martuino 1.01";
float valorPH = 0;
float valorPH2 = 0;
float pHValue = 0;
//Medidor de flujo de agua
volatile int  flow_frequency;  // Measures flow meter pulses                   
int Alarflow = 0; //A "1" fujode agua sobrepaso el valor maximo
int Day = 1;
int Month = 1;
int Year = 2020;
int Hour = 0;
int Minute = 0; 
float divisor = 0.0003581; //1,467/4096
int colorph = 0x2d572c;
int colortemp = 0x2d572c;
int colorcaudal = 0x2d572c;
float factor_conversion=7.5; //para convertir de frecuencia a caudal
float caudal_L_m = 0;
unsigned long startMillis;
unsigned long currentMillis;

// devices
OneWire  ds(10);  // on pin 10 (a 4.7K resistor is necessary)
WiFiServer server(80); //Pagina web estado y configuracion
WiFiClient luces; //Conexion con el wifi de las luces
tmElements_t tm;

void setup() {
  Serial.begin(19200);      // Inicializa puerto serie
  Wire.begin();  
  //Configura salidas
  pinMode(calentador,OUTPUT);
  pinMode(dosificadora,OUTPUT);
  pinMode(releco2,OUTPUT);
  pinMode(esterilizador,OUTPUT);
  pinMode(jp1rele3,OUTPUT);
  pinMode(jp1rele4,OUTPUT);
  pinMode(comedero,OUTPUT);
  pinMode(jp2rele4,OUTPUT);
  pinMode(LimitPH,INPUT);
  pinMode(11,INPUT); //
  digitalWrite(releco2,HIGH);//JP1-1
  digitalWrite(esterilizador,HIGH);//JP1-2
  digitalWrite(jp1rele3,HIGH);//JP1-3
  digitalWrite(jp1rele4,HIGH);//JP1-4
  digitalWrite(calentador,HIGH);//JP2-1
  digitalWrite(dosificadora,HIGH);//JP2-2
  digitalWrite(comedero,HIGH);//JP2-3
  digitalWrite(jp2rele4,HIGH);//JP2-4
  attachInterrupt(11, caudal, RISING);
  startMillis = millis();
  hubocambio = true;
  // Esperando a conectar con tu wifi:
  Serial.print("Esperando a conectar con: ");
  // muestra el nombre de tu red(SSID);
  Serial.println(ssid); 
  // Conectando a red WPA/WPA2. Cambia esta l�nea si usas una red abierta o WEP :
  WiFi.begin(ssid, password);
  while ( WiFi.status() != WL_CONNECTED) {
    // muestra puntos hata que conecte
    Serial.print(".");
    delay(300);
  }
  Serial.println("\nEstas conectado con tu wifi");
  Serial.println("Esperando una IP");
  while (WiFi.localIP() == INADDR_NONE) {
    // muestra puntos hasta obtener la IP
    Serial.print(".");
    delay(300);
  }
  // Ya esta conectado , muestra el estado  
  printWifiStatus();
  server.begin(); //Comienza servidor en puerto 80
  Serial.println("Servidor web en marcha version 1.01!");
  //Extrae las memorias
  SerFlash.begin();  
  //Lee de la memoria las horas
  SerFlash.open("horas.txt", FS_MODE_OPEN_READ);
  for (int i=0; i<20;i++){
    flash_buffer[i] = SerFlash.read(); 
  }
  SerFlash.close();
  for (int a=0;a<20;a++){
    if (flash_buffer[a] != 0){
     Memohora[a] = char(flash_buffer[a]);
    }
  }
  //Lee de la memoria los minutos
  SerFlash.open("minu.txt", FS_MODE_OPEN_READ);
  for (int i=0; i<20;i++){
    flash_buffer2[i] = SerFlash.read(); 
  }
  SerFlash.close();
  for (int a=0;a<20;a++){
    if (flash_buffer2[a] != 0){
     Memominu[a] = char(flash_buffer2[a]);
    }
  }
  //Lee de la memoria los valores salida 1
  SerFlash.open("valor1.txt", FS_MODE_OPEN_READ);
  for (int i=0; i<20;i++){
    flash_buffer3[i] = SerFlash.read(); 
  }
  SerFlash.close();
  for (int a=0;a<20;a++){
    if (flash_buffer3[a] != 0){
     Memovalor1[a] = char(flash_buffer3[a]);
    }
  }
  //Lee de la memoria los valores de la salida 2
  SerFlash.open("valor2.txt", FS_MODE_OPEN_READ);
  for (int i=0; i<20;i++){
    flash_buffer4[i] = SerFlash.read(); 
  }
  SerFlash.close();
  for (int a=0;a<20;a++){
    if (flash_buffer4[a] != 0){
     Memovalor2[a] = char(flash_buffer4[a]);
    }
  }
  //Lee de la memoria los valores del calentador
  SerFlash.open("valor3.txt", FS_MODE_OPEN_READ);
  for (int i=0; i<20;i++){
    flash_buffer5[i] = SerFlash.read(); 
  }
  SerFlash.close();
  for (int a=0;a<20;a++){
    if (flash_buffer5[a] != 0){
     Memovalor3[a] = char(flash_buffer5[a]);
    }
  }
  //Lee de la memoria la última configuracion
  SerFlash.open("conf.txt", FS_MODE_OPEN_READ);
  for (int i=0; i<8;i++){
    flash_buffer7[i] = SerFlash.read(); 
  }
  SerFlash.close();
  valors1 = flash_buffer7[0];
  valors2 = flash_buffer7[1];
  valors3 = flash_buffer7[2];
  Numactual = flash_buffer7[3];
  Alarflow = flash_buffer7[4];
  tempcale = flash_buffer7[5];
  phmin = flash_buffer7[6];
  phdosi = flash_buffer7[7];
}


void loop() { 
  if (RTC.read(tm)) {
    Year = tmYearToCalendar(tm.Year);
    Month = tm.Month;
    Day = tm.Day;
    Hour = tm.Hour;
    Minute = tm.Minute;
  }else{
    Serial.println("No se pudo leer la hora"); 
  }
  leeTemperatura();
  leePH();
  float frecuencia=ObtenerFrecuencia(); //obtenemos la Frecuencia de los pulsos en Hz
  caudal_L_m=frecuencia/factor_conversion; //calculamos el caudal en L/m
  //Alarmas y condiciones extremas
  if (caudal_L_m < Alarflow){
    colorcaudal = 0xff0000; //Rojo
  }else{
    colorcaudal = 0x2d572c; //Verde
  }
  if (valorPH < phmin){
    colorph = 0xff0000; //Rojo
    digitalWrite(releco2,HIGH); //Se para el rele
  }else{
    colorph = 0x2d572c; //Verde
  }
  if (celsius < tempcale){
    colortemp = 0xff0000; //Rojo
    digitalWrite(calentador, LOW); //Se pone en marcha rele
  }else{
    if (celsius > tempcale){
      colortemp = 0x2d572c; //Verde
      digitalWrite(calentador, HIGH); //Se para el rele
    }
  }
  if (valors3 == 0){
    digitalWrite(comedero,HIGH);
  }else{
    digitalWrite(comedero,LOW);
  }
  // Escucha para conexiones entrantes
  WiFiClient client = server.available();
  if (client) {
    Serial.println("nuevo cliente");
    // el http request finaliza con una linea en blanco
    boolean currentLineIsBlank = true;
    String cadena=""; //Creamos una cadena de caracteres vacía
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        //Serial.write(c);
        cadena.concat(c);//Unimos el String 'cadena' con la petición HTTP (c). De esta manera convertimos la petición HTTP a un String
        //Puesta en Hora
        int posicion2 = cadena.indexOf("horafin=0");
        if (posicion2>0){
          int horaini = cadena.indexOf("horaini=");  
          int horafin = cadena.indexOf("&",horaini);
          String horanueva =cadena.substring(horaini+8,horaini+10);
          String minunuevo = cadena.substring(horaini+13,horafin);
          int fechaini = cadena.indexOf("fechaini=");  
          int fechafin = cadena.indexOf("&",fechaini);
          String anonuevo = cadena.substring(fechaini+9,fechaini+13);
          String mesnuevo = cadena.substring(fechaini+14,fechaini+16);
          String dianuevo = cadena.substring(fechaini+17,fechafin);
          Day = dianuevo.toInt();
          Month = mesnuevo.toInt();
          Year = anonuevo.toInt();
          Hour = horanueva.toInt();
          Minute = minunuevo.toInt();
          tm.Year = CalendarYrToTm(Year);
          tm.Month = Month;
          tm.Day = Day;
          tm.Hour = Hour;
          tm.Minute = Minute;
          tm.Second = 0;
          RTC.write(tm);
          fecha = String(Day) + '/' + String(Month) + '/' + String(Year) + ' ' + String(Hour) + ':' + String(Minute); 
        }
        //Guardar programas
        int posfin = cadena.indexOf("programador=0");
        if (posfin>0){
          int posprogra1 = 0;
          int numemo = 0;
          String horaini = "00";
          String minuini = "00";
          String val1 = "00";
          String val2 = "00";
          String val3 = "00";
          char mese[3]; //Buffer para convertir un string to uint_8 con atoi()
          char meso[4];
          if (cadena.indexOf("horamemo1=")>0){
            posprogra1 = cadena.indexOf("horamemo1=");
            horaini = cadena.substring(posprogra1+10,posprogra1+12);
            minuini = cadena.substring(posprogra1+15,posprogra1+17);
            numemo = 0;
          }
          if (cadena.indexOf("horamemo2=")>0){
            posprogra1 = cadena.indexOf("horamemo2=");
            horaini = cadena.substring(posprogra1+10,posprogra1+12);
            minuini = cadena.substring(posprogra1+15,posprogra1+17);
            numemo = 1;
          }
          if (cadena.indexOf("horamemo3=")>0){
            posprogra1 = cadena.indexOf("horamemo3=");
            horaini = cadena.substring(posprogra1+10,posprogra1+12);
            minuini = cadena.substring(posprogra1+15,posprogra1+17);
            numemo = 2;
          }
          if (cadena.indexOf("horamemo4=")>0){
            posprogra1 = cadena.indexOf("horamemo4=");
            horaini = cadena.substring(posprogra1+10,posprogra1+12);
            minuini = cadena.substring(posprogra1+15,posprogra1+17);
            numemo = 3;
          }
          if (cadena.indexOf("horamemo5=")>0){
            posprogra1 = cadena.indexOf("horamemo5=");
            horaini = cadena.substring(posprogra1+10,posprogra1+12);
            minuini = cadena.substring(posprogra1+15,posprogra1+17);
            numemo = 4;
          }
          if (cadena.indexOf("horamemo6=")>0){
            posprogra1 = cadena.indexOf("horamemo6=");
            horaini = cadena.substring(posprogra1+10,posprogra1+12);
            minuini = cadena.substring(posprogra1+15,posprogra1+17);
            numemo = 5;
          }
          if (cadena.indexOf("horamemo7=")>0){
            posprogra1 = cadena.indexOf("horamemo7=");
            horaini = cadena.substring(posprogra1+10,posprogra1+12);
            minuini = cadena.substring(posprogra1+15,posprogra1+17);
            numemo = 6;
          }
          if (cadena.indexOf("horamemo8=")>0){
            posprogra1 = cadena.indexOf("horamemo8=");
            horaini = cadena.substring(posprogra1+10,posprogra1+12);
            minuini = cadena.substring(posprogra1+15,posprogra1+17);
            numemo = 7;
          }
          if (cadena.indexOf("horamemo9=")>0){
            posprogra1 = cadena.indexOf("horamemo9=");
            horaini = cadena.substring(posprogra1+10,posprogra1+12);
            minuini = cadena.substring(posprogra1+15,posprogra1+17);
            numemo = 8;
          }
          if (cadena.indexOf("horamemo10=")>0){
            posprogra1 = cadena.indexOf("horamemo10=");
            horaini = cadena.substring(posprogra1+11,posprogra1+13);
            minuini = cadena.substring(posprogra1+16,posprogra1+18);
            numemo = 9;
          }          
          int valini = cadena.indexOf("valor1=");
          int valfin = cadena.indexOf("&",valini);
          val1 = cadena.substring( valini+7,valfin);
          long med = val1.toInt();
          val1 = String(med);
          int valini2 = cadena.indexOf("valor2=");
          int valfin2 = cadena.indexOf("&",valini2);
          val2 = cadena.substring( valini2+7,valfin2);
          long med2 = val2.toInt();
          val2 = String(med2);
          int valini3 = cadena.indexOf("valor3=");
          int valfin3 = cadena.indexOf("&",valini3);
          val3 = cadena.substring( valini3+7,valfin3); 
          if (val3 == "desactivar"){val3 = "0";}
          if (val3 == "activar"){val3 = "1";}
          horaini.toCharArray(mese,3);          
          Memohora[numemo] = atoi(mese);
          minuini.toCharArray(mese,3);
          Memominu[numemo] = atoi(mese);
          val1.toCharArray(meso,4);
          Memovalor1[numemo] = atoi(meso);
          val2.toCharArray(meso,4);
          Memovalor2[numemo] = atoi(meso);
          val3.toCharArray(meso,4);
          Memovalor3[numemo] = atoi(meso);
          hubocambio = true;
        }
        //temp
        int postemp = cadena.indexOf("tempefin=0");
        if (postemp>0){
          String valtemp = "0";
          int valinitemp = cadena.indexOf("tempmin=");
          int valfintemp = cadena.indexOf("&",valinitemp);
          valtemp = cadena.substring(valinitemp+8 ,valfintemp);
          tempcale = valtemp.toInt();
          hubocambio = true; //Para guardar el valor de alarma del caudalimetro
        }
        //PH
        int posph = cadena.indexOf("phfin=0");
        if (posph>0){
          String valph = "0";
          int valiniph = cadena.indexOf("phmin=");
          int valfinph = cadena.indexOf("&",valiniph);
          valph = cadena.substring(valiniph+6 ,valfinph);
          phmin = valph.toInt();
          hubocambio = true; //Para guardar el valor de alarma del caudalimetro
        }
        //Caudal
        int poscaudal = cadena.indexOf("caudalfin=0");
        if (poscaudal>0){
          String valca = "0";
          int valinicau = cadena.indexOf("caudalmax=");
          int valfincau = cadena.indexOf("&",valinicau);
          valca = cadena.substring(valinicau+10 ,valfincau);
          Alarflow = valca.toInt();
          hubocambio = true; //Para guardar el valor de alarma del caudalimetro
        }
        //Valor S1
        int posval1 = cadena.indexOf("valorfin1=0");
        if (posval1>0){
          String vals1 = "0";
          int vals1ini = cadena.indexOf("valor1=");
          int vals1fin = cadena.indexOf("&",vals1ini);
          vals1 = cadena.substring(vals1ini+7,vals1fin);
          valors1 = vals1.toInt();
          hubocambio = true; //Para guardar el valor
          Numactual = 10; //Al activar algo manual se para secuencia
          posval1 = 0;
        }
        //Valor S2
        int posval2 = cadena.indexOf("valorfin2=0");
        if (posval2>0){
          String vals2 = "0";
          int vals2ini = cadena.indexOf("valor2=");
          int vals2fin = cadena.indexOf("&",vals2ini);
          vals2 = cadena.substring(vals2ini+7,vals2fin);
          valors2 = vals2.toInt();
          hubocambio = true; //Para guardar el valor
          Numactual = 10; //Al activar algo manual se para secuencia
          posval2 = 0;
        }
        //Valor S3 -> Comedero
        int posval3 = cadena.indexOf("valorfin3=0");
        if (posval3>0){
          String vals3 = "desactivar";
          int vals3ini = cadena.indexOf("valor3=");
          int vals3fin = cadena.indexOf("&",vals3ini);
          vals3 = cadena.substring(vals3ini+7,vals3fin);
          if (vals3 == "activar"){
            valors3 = 1;
            digitalWrite(comedero,LOW);
          }
          if (vals3 == "desactivar"){
            valors3 = 0;
            digitalWrite(comedero,HIGH);
          }
          hubocambio = true; //Para guardar el valor
          posval3 = 0;
        }       
        //Valor Rele 1 jp1 -> rele CO2
        int posre1jp1 = cadena.indexOf("rele1jp1fin=0");
        if (posre1jp1>0){
          String valre1 = "desactivar";
          int valre1ini = cadena.indexOf("rele1jp1=");
          int valre1fin = cadena.indexOf("&",valre1ini);
          valre1 = cadena.substring(valre1ini+9,valre1fin);
          if (valre1 == "activar"){digitalWrite(releco2,LOW);}
          if (valre1 == "desactivar"){digitalWrite(releco2,HIGH);}
          posre1jp1 = 0;
        }       
        //Valor Rele 2 jp1 -> rele esterilizador
        int posre2jp1 = cadena.indexOf("rele2jp1fin=0");
        if (posre2jp1>0){
          String valre2 = "desactivar";
          int valre2ini = cadena.indexOf("rele2jp1=");
          int valre2fin = cadena.indexOf("&",valre2ini);
          valre2 = cadena.substring(valre2ini+9,valre2fin);
          if (valre2 == "activar"){digitalWrite(esterilizador,LOW);}
          if (valre2 == "desactivar"){digitalWrite(esterilizador,HIGH);}
          posre2jp1 = 0;
        }       
        //Valor Rele 3 jp1 -> rele 3
        int posre3jp1 = cadena.indexOf("rele3jp1fin=0");
        if (posre3jp1>0){
          String valre3 = "desactivar";
          int valre3ini = cadena.indexOf("rele3jp1=");
          int valre3fin = cadena.indexOf("&",valre3ini);
          valre3 = cadena.substring(valre3ini+9,valre3fin);
          if (valre3 == "activar"){digitalWrite(jp1rele3,LOW);}
          if (valre3 == "desactivar"){digitalWrite(jp1rele3,HIGH);}
          posre3jp1 = 0;
        }       
        //Valor Rele 4 jp1 -> rele 4
        int posre4jp1 = cadena.indexOf("rele4jp1fin=0");
        if (posre4jp1>0){
          String valre4 = "desactivar";
          int valre4ini = cadena.indexOf("rele4jp1=");
          int valre4fin = cadena.indexOf("&",valre4ini);
          valre4 = cadena.substring(valre4ini+9,valre4fin);
          if (valre4 == "activar"){digitalWrite(jp1rele4,LOW);}
          if (valre4 == "desactivar"){digitalWrite(jp1rele4,HIGH);}
          posre4jp1 = 0;
        }       
        //Valor Rele 1 jp2 -> rele calentador
        int posre1jp2 = cadena.indexOf("rele1jp2fin=0");
        if (posre1jp2>0){
          String valre5 = "desactivar";
          int valre5ini = cadena.indexOf("rele1jp2=");
          int valre5fin = cadena.indexOf("&",valre5ini);
          valre5 = cadena.substring(valre5ini+9,valre5fin);
          if (valre5 == "activar"){digitalWrite(calentador,LOW);}
          if (valre5 == "desactivar"){digitalWrite(calentador,HIGH);}
          posre1jp2 = 0;
        }       
        //Valor Rele 2 jp2 -> rele 2
        int posre2jp2 = cadena.indexOf("rele2jp2fin=0");
        if (posre2jp2>0){
          String valre6 = "desactivar";
          int valre6ini = cadena.indexOf("rele2jp2=");
          int valre6fin = cadena.indexOf("&",valre6ini);
          valre6 = cadena.substring(valre6ini+9,valre6fin);
          if (valre6 == "activar"){
            digitalWrite(comedero,LOW);
            valors3 = 1;
          }
          if (valre6 == "desactivar"){
            digitalWrite(comedero,HIGH);
            valors3 = 0;
          }
          hubocambio = true; //Para guardar el valor
          posre2jp2 = 0;
        }       
        //Valor Rele 3 jp2 -> rele dosificadora
        int posre3jp2 = cadena.indexOf("rele3jp2fin=0");
        if (posre3jp2>0){
          String valre7 = "desactivar";
          int valre7ini = cadena.indexOf("rele3jp2=");
          int valre7fin = cadena.indexOf("&",valre7ini);
          valre7 = cadena.substring(valre7ini+9,valre7fin);
          if (valre7 == "activar"){digitalWrite(dosificadora,LOW);}
          if (valre7 == "desactivar"){digitalWrite(dosificadora,HIGH);}
          posre3jp2 = 0;
        }       
        //Valor Rele 4 jp2 -> rele 4
        int posre4jp2 = cadena.indexOf("rele4jp2fin=0");
        if (posre4jp2>0){
          String valre8 = "desactivar";
          int valre8ini = cadena.indexOf("rele4jp2=");
          int valre8fin = cadena.indexOf("&",valre8ini);
          valre8 = cadena.substring(valre8ini+9,valre8fin);
          if (valre8 == "activar"){digitalWrite(jp2rele4,LOW);}
          if (valre8 == "desactivar"){digitalWrite(jp2rele4,HIGH);}
          posre4jp2 = 0;
        }       

        //Enviamos una replica
        if (c == '\n' && currentLineIsBlank) {
          // envia la cabecera standard http 
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: text/html");
          client.println("Connection: close");  
          //client.println("Refresh: 5");  //Refresca cada 5 seg.
          client.println();
          //COMIENZA LA PAGINA WEB
          page_web(client);
          break;
        }
        if (c == '\n') {
          // Comienza una nueva linea
          currentLineIsBlank = true;
        }else if (c != '\r') {           
          currentLineIsBlank = false;
        }
      }
    }
    // Da tiempo al navegador para recibir los datos
    delay(3000); //antes delay(400)
    // cerramos la conexion:
    client.stop();
    Serial.println("cliente desconectado");
  }
  //cada minuto mira los valores
  Horaactual = String(Hour,DEC).toInt();
  Minutoactual = String(Minute,DEC).toInt();
  //cada minuto mira los valores
  if (Minutoactual != minutoAnterior){
      minutoAnterior = Minutoactual;
      //Comparamos cosas cada minuto
      
      //Ahora vemos la progresión entre memorias
      misminutos = Memohora[Numactual] * 60; //Horas a minutos memoria 0
      misminutos += Memominu[Numactual];
      if (Numactual == 9){   //La última memoria 
          misminutos2 = Memohora[0] * 60; //Horas a minutos memoria 0
          misminutos2 += Memominu[0];
         //valores
         if (Memovalor1[Numactual] > Memovalor1[0]){
           difvalor1 = Memovalor1[Numactual] - Memovalor1[0];
           difneg1 = true; //descenso de valor 1
         }else{
           difvalor1 = Memovalor1[0] - Memovalor1[Numactual];
           difneg1 = false; //Ascenso valor 1
         }
         if (Memovalor2[Numactual]> Memovalor2[0]){
           difvalor2 = Memovalor2[Numactual] - Memovalor2[0];
           difneg2 = true; //Descenso valor 2
         }else{
           difvalor2 = Memovalor2[0] - Memovalor2[Numactual];
           difneg2 = false; //Ascenso valor 2
         }
      }else if (Numactual == 10){
         //No hagas nada secuencia parada   
      }else if (Numactual > 10){
        Numactual = 0;
      }else{
          misminutos2 = Memohora[Numactual+1] * 60; //Horas a minutos memoria 0
          misminutos2 += Memominu[Numactual+1];
         //valores
         if (Memovalor1[Numactual] > Memovalor1[Numactual+1]){
           difvalor1 = Memovalor1[Numactual] - Memovalor1[Numactual+1];
           difneg1 = true; //Descenso
         }else{
           difvalor1 = Memovalor1[Numactual+1] - Memovalor1[Numactual]; 
           difneg1 = false; //Ascenso
         }
         if (Memovalor2[Numactual] > Memovalor2[Numactual+1]){
           difvalor2 = Memovalor2[Numactual] - Memovalor2[Numactual+1];
           difneg2 = true; //Descenso
         }else{
           difvalor2 = Memovalor2[Numactual+1] - Memovalor2[Numactual];
           difneg2 = false; //Ascenso
         } 
      }
      //diferencia de minutos
      if (misminutos > misminutos2){
        pos = misminutos - misminutos2;
      }else{
        pos = misminutos2 - misminutos;
      }
      //Cada minuto hay que realizar un cambio de x en los valores de las salidas
      if (difvalor1 > pos){
        difMinuto1 = difvalor1 / pos; //Aumenta difMinuto1 cada minuto
      }else{
        difMinuto1 = pos / difvalor1; //Aumenta 1 cada difMinuto1 minutos
        difMinuto1 = difMinuto1 * (-1); //Lo hacemos negativo
        cntminu1++;
      }
      if (difvalor2 > pos){      
        difMinuto2 = difvalor2 / pos;
      }else{
        difMinuto2 = pos / difvalor2;
        difMinuto2 = difMinuto2 * (-1); //Lo hacemos negativo
        cntminu2++;
      }
      //Cada minuto sumamos el valor diferencia a cada salida
      if (difMinuto1 >= 0 ){
        if (difneg1){
          valors1 -= difMinuto1; 
        }else{
          valors1 += difMinuto1;
        }
        hubocambio = true;
      }else{
        cntsel = difMinuto1 * (-1); //Lo hacemos positivo
        cntsel = cntminu1 % cntsel;
        if (cntsel == 0){
          cntminu1=0;
          if (difneg1){
            valors1 -= 1; 
          }else{
            valors1 += 1;
          }          
        }
      }
      if (difMinuto2 >= 0 ){
        if (difneg2){
          valors2 -= difMinuto2; 
        }else{
          valors2 += difMinuto2;
        }
        hubocambio = true;
      }else{
        cntsel = difMinuto2 * (-1); //Lo hacemos positivo
        cntsel =  cntminu2 % cntsel;
        if (cntsel == 0){
          cntminu2=0;
          if (difneg2){
            valors2 -= 1; 
          }else{
            valors2 += 1;
          }          
        }
      }
      for (int g=0; g<10;g++){
         if (Memohora[g] == Horaactual){
          if (Memominu[g] == Minutoactual){
            Numactual = g;
            if (valors1 != Memovalor1[g]){
             valors1 = Memovalor1[g];
             hubocambio = true;
            }
            if (valors2 != Memovalor2[g]){
             valors2 = Memovalor2[g];
             hubocambio = true;
            }
            if (valors3 != Memovalor3[g]){
             valors3 = Memovalor3[g];
             hubocambio = true;
            }
          }
        }
      }
  }  
  if (valors1 < 0){
    valors1 = 0;
  }
  if (valors2 < 0 ){
    valors2 = 0;
  }
  if (valors3 <0){
    valors3 = 0;
  }
  if (valors1 > 100){
    valors1 = 100;
  }
  if (valors2 > 100 ){
    valors2 = 100;
  }
  if (hubocambio){
    hubocambio = false;
    //Guardamos los nuevos valores
    flash_buffer7[0] = valors1;
    flash_buffer7[1] = valors2;
    flash_buffer7[2] = valors3;
    flash_buffer7[3] = Numactual;
    flash_buffer7[4] = Alarflow;
    flash_buffer7[5] = tempcale;
    flash_buffer7[6] = phmin;
    flash_buffer7[7] = phdosi;
    Serial.println("Nivel Salida1: " + String(valors1));
    Serial.println("Nivel Salida2: " + String(valors2));
    Serial.println("Nivel Salida3: " + String(valors3));
    Serial.println("Secuencia: " + String(Numactual));
    int s1nivel = map(valors1,0,100,0,1024);
    int s2nivel = map(valors2,0,100,0,1024);
    String valoresp1 = String(s1nivel);
    String valoresp2 = String(s2nivel);
    //Los valores a enviar tienen que tener 4 caracteres
    if (valoresp1.length() == 1){
      valoresp1 = "000" + valoresp1;
    }else if (valoresp1.length() == 2){
      valoresp1 = "00" + valoresp1;
    }else if (valoresp1.length() == 3){
      valoresp1 = "0" + valoresp1;
    }
    if (valoresp2.length() == 1){
      valoresp2 = "000" + valoresp2;
    }else if (valoresp2.length() == 2){
      valoresp2 = "00" + valoresp2;
    }else if (valoresp2.length() == 3){
      valoresp2 = "0" + valoresp2;
    }
    //Enviar via wifi el estado de las salidas
    if (luces.connect(IPluces, 23)) {
      luces.stop();
    }
    Serial.println("Conectando con luces para luz una");
    if (luces.connect(IPluces, 23)) {
      delay(200);
      Serial.println("conectado a luces");
      luces.println("#881D" + valoresp1 + "#\n");
      Serial.println("Enviado comando 88");  
      delay(500);
      luces.stop();
      Serial.println("Desconectado de luces");
    }else{
      Serial.println("No se pudo conectar con luces"); 
    }
    delay(1000);
    Serial.println("Conectando con luces para luz dos");
    if (luces.connect(IPluces, 23)) {
      delay(200);
      Serial.println("conectado a luces");
      luces.println("#891D" + valoresp2 + "#\n"); 
      Serial.println("Enviado comando 89");
      delay(500);
      luces.stop();
      Serial.println("Desconectado de luces");
    }else{
      Serial.println("No se pudo conectar con luces"); 
    }
    //Grabamos los valores en flash
    SerFlash.open("conf.txt",FS_MODE_OPEN_CREATE(8, _FS_FILE_OPEN_FLAG_COMMIT));
    SerFlash.write(flash_buffer7,8);  // x characters
    SerFlash.close();
    //Guardamos la hora en flash
    for (int d=0;d<20;d++){
      flash_buffer[d] = (uint8_t)Memohora[d];  
    }
    SerFlash.open("horas.txt",FS_MODE_OPEN_CREATE(20, _FS_FILE_OPEN_FLAG_COMMIT));
    SerFlash.write(flash_buffer,20);  // x characters
    SerFlash.close();
    //Guardamos el minuto en flash
    for (int d=0;d<20;d++){
      flash_buffer2[d] = (uint8_t)Memominu[d];  
    }
    SerFlash.open("minu.txt",FS_MODE_OPEN_CREATE(20, _FS_FILE_OPEN_FLAG_COMMIT));
    SerFlash.write(flash_buffer2,20);  // x characters
    SerFlash.close();
    //Guardamos el valor 1 en flash
    for (int d=0;d<20;d++){
      flash_buffer3[d] = (uint8_t)Memovalor1[d];  
    }
    SerFlash.open("valor1.txt",FS_MODE_OPEN_CREATE(20, _FS_FILE_OPEN_FLAG_COMMIT));
    SerFlash.write(flash_buffer3,20);  // x characters
    SerFlash.close();
    //Guardamos el valor 2 en flash
    for (int d=0;d<20;d++){
      flash_buffer4[d] = (uint8_t)Memovalor2[d];  
    }
    SerFlash.open("valor2.txt",FS_MODE_OPEN_CREATE(20, _FS_FILE_OPEN_FLAG_COMMIT));
    SerFlash.write(flash_buffer4,20);  // x characters
    SerFlash.close();
    //Guardamos el valor 3 en flash
    for (int d=0;d<20;d++){
      flash_buffer5[d] = (uint8_t)Memovalor3[d];
    }  
    SerFlash.open("valor3.txt",FS_MODE_OPEN_CREATE(20, _FS_FILE_OPEN_FLAG_COMMIT));
    SerFlash.write(flash_buffer5,20);  // x characters
    SerFlash.close();
  }
} 

//*******************************WIFI STATUS**********************************
void printWifiStatus() {
  // envia el SSID de la red:
  Serial.print("Nombre d la red: ");
  Serial.println(WiFi.SSID());

  // envia la IP :
  IPAddress ip = WiFi.localIP();
  Serial.print("IP: ");
  Serial.println(ip);

  // Envia el nivel de se�al:
  long rssi = WiFi.RSSI();
  Serial.print("Nivel de senyal (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}
//*************SENSOR TEMPERATURA DS18B20****************
void leeTemperatura(){
  byte i;
  byte present = 0;
  byte type_s;
  byte data[12];
  byte addr[8];
  if ( !ds.search(addr)) {
    //Serial.println("No more addresses.");
    //Serial.println();
    ds.reset_search();
    delay(250);
    return;
  }
  if (OneWire::crc8(addr, 7) != addr[7]) {
      Serial.println("CRC is not valid!");
      return;
  }
  // the first ROM byte indicates which chip
  switch (addr[0]) {
    case 0x10:
      //Serial.println("  Chip = DS18S20");  // or old DS1820
      type_s = 1;
      break;
    case 0x28:
      //Serial.println("  Chip = DS18B20");
      type_s = 0;
      break;
    case 0x22:
      //Serial.println("  Chip = DS1822");
      type_s = 0;
      break;
    default:
      //Serial.println("Device is not a DS18x20 family device.");
      return;
  } 
  ds.reset();
  ds.select(addr);
  ds.write(0x44, 1);        // start conversion, with parasite power on at the end
  delay(1000);     // maybe 750ms is enough, maybe not
  // we might do a ds.depower() here, but the reset will take care of it.
  present = ds.reset();
  ds.select(addr);    
  ds.write(0xBE);         // Read Scratchpad
  for ( i = 0; i < 9; i++) {           // we need 9 bytes
    data[i] = ds.read();
  }
  int16_t raw = (data[1] << 8) | data[0];
  if (type_s) {
    raw = raw << 3; // 9 bit resolution default
    if (data[7] == 0x10) {
      // "count remain" gives full 12 bit resolution
      raw = (raw & 0xFFF0) + 12 - data[6];
    }
  } else {
    byte cfg = (data[4] & 0x60);
    // at lower res, the low bits are undefined, so let's zero them
    if (cfg == 0x00) raw = raw & ~7;  // 9 bit resolution, 93.75 ms
    else if (cfg == 0x20) raw = raw & ~3; // 10 bit res, 187.5 ms
    else if (cfg == 0x40) raw = raw & ~1; // 11 bit res, 375 ms
    //// default is 12 bit resolution, 750 ms conversion time
  }
  celsius = (float)raw / 16.0;
  //Serial.print("Sonda Temperatura: ");
  //Serial.println(celsius);
}

//************SENSOR PH**************************************
void leePH(){
  unsigned long int avgValue;
  float b;
  float  Offset = 0.0;
  int buf[10],temp; 
  for(int i=0;i<10;i++){
    buf[i]=analogRead(analogInPin1);
    delay(10);
  }
  for(int i=0;i<9;i++){
    for(int j=i+1;j<10;j++){
      if(buf[i]>buf[j]){
        temp=buf[i];
        buf[i]=buf[j];
        buf[j]=temp;
      }
    }
  }
  avgValue=0;
  for(int i=2;i<8;i++){
    avgValue+=buf[i];
  }
  int pHVol = analogRead(analogInPin1);
  float valorVph = (float) pHVol * divisor;
  valorVph = valorVph * 3.15; //3.15 divisor resistivo
  float totPH = (float) avgValue * 5.0/4096/6;
  valorPH = (-5.70 * totPH)  + 21.34; 
  //Serial.print("Sonda PH: ");
  //Serial.println(valorPH);
}
//*************CAUDALIMETRO***************************************
void caudal(){
  flow_frequency++;
}
int ObtenerFrecuencia() 
{
  int frecuencia;
  currentMillis = millis();
  double tiem_transcurrido = currentMillis - startMillis;
  startMillis = currentMillis;
  if (tiem_transcurrido > 0){
    tiem_transcurrido = tiem_transcurrido / 1000; //Lo pasamos a segundos
  }
  frecuencia = 0;
  frecuencia=flow_frequency/tiem_transcurrido; //Hz(pulsos por segundo)
  flow_frequency = 0;
  return frecuencia;
}
//*****************PAGINA WEB*************************************
void page_web(WiFiClient client){
  client.println("<!DOCTYPE HTML>");
  client.println("<html>");
  client.print("<head><title>");
  client.print(titulo);
  client.print("</title><style>table {font-family: arial, sans-serif;border-collapse: collapse;width: 50%;}");
  client.print("td, th {border: 1px solid #dddddd;text-align: left;padding: 8px;}");
  client.print("tr:nth-child(even) { background-color: #dddddd;}.buttontemp{background-color: #");
  client.print(colortemp,HEX);
  client.print(";}.buttonph{background-color: #");
  client.print(colorph,HEX);
  client.print(";}.buttonflow{background-color: #");
  client.print(colorcaudal,HEX);
  client.println(";}</style></head>");
  client.println("<body><font size='+3'>Control de acuario Martuino</font><p></p><br/>");
  client.println("<table style='width:50%'><tr><th>Variable</th><th>Valor</th><th>Valor minimo</th></tr>");
  client.print("<tr><td><font size='+2'>Fecha:");
  String dia = String(Day,DEC);
  if (dia.length()==1){
    dia = '0' + dia; 
  }
  String mes = String(Month,DEC);
  if (mes.length() == 1){
    mes = '0' + mes; 
  }
  String ano = String(Year,DEC);
  if (ano.length() == 1){
    ano = '000' + ano;
  }else if (ano.length() == 2){
    ano = '00' + ano;
  }else if (ano.length() == 2){
    ano = '0' + ano;
  } 
  String hora = String(Hour,DEC);
  if (hora.length() == 1){
    hora = '0' + hora; 
  }
  String minu = String(Minute,DEC);
  if (minu.length() == 1){
    minu = '0' + minu; 
  }
  client.print("</td><td><form action='HORA' id='F1'><input type='time' name='horaini' value='");
  client.print(hora);
  client.print(":");
  client.print(minu);
  client.print("'><input type='date' name='fechaini' value='");
  client.print(ano);
  client.print("-");
  client.print(mes);
  client.print("-");
  client.print(dia);
  client.print("'><input type='hidden' name='horafin' value='0'>");          
  client.println("</td><td><input class='button' type='submit' value='Cambiar'></form></font></td><tr></form><p></p><br/>");
  client.print("<tr><td><font size='+2'>Temperatura:</td><td>");
  //leeTemperatura();
  client.print(celsius);
  client.print(" &#8451</font></td><td><form action='TEMP' id='G'><input type='number' name='tempmin' min ='21' max='29' value='");
  client.print(tempcale);
  client.println("'><input type='hidden' name='tempefin' value='0'><input class='buttontemp' type='submit' value='Cambiar'></form></td></tr>");
  //leePH(); 
  client.print("<tr><td><font size='+2'>Nivel de P.H.:</td><td>");                
  client.print(valorPH);
  client.print("</font></td><td><form action='PHM' id='H'><input type='number' name='phmin' min ='0' max='10' value='");
  client.print(phmin);
  client.println("'><input type='hidden' name='phfin' value='0'><input class='buttonph' type='submit' value='Cambiar'></form></td></tr>");
  sensorReading = analogRead(analogInPin2);
  client.print("<tr><td><font size='+2'>Entrada anal&oacute;gica 2:</td><td>");                
  float vals2 = (float)(sensorReading * divisor) * 3.15;
  client.print(vals2);
  client.println("</font></td><td></td></tr>");
  sensorReading = analogRead(analogInPin3);
  client.print("<tr><td><font size='+2'>Entrada anal&oacute;gica 3:</td><td>");                
  float vals3 = (float)sensorReading * divisor;
  client.print(vals3);
  client.println("</font></td><td></td></tr>");
  client.print("<tr><td><font size='+2'>Caudal&iacute;metro (L/Min):</td><td>");
  client.print(caudal_L_m,DEC);
  client.print("</td><td><form action='CAUDAL' id='F2'><input type='number' name='caudalmax' min ='0' max='1000' value='");
  client.print(Alarflow);
  client.print("'><input type='hidden' name='caudalfin' value='0'><input class='buttonflow' type='submit' value='Cambiar'></form></td></tr>");
  client.println("</font></td></tr></table><br></br><h2>Programas: (cambia cada linea por separado)</h2><br></br>");
  client.print("<table>");
  client.print("<tr><th>Hora/Min.</th><th>Luz blanca</th><th>Luz azul</th><th>Comedero</th><th>Cambiar</th></tr>");
  for (int a=0; a<10;a++){
    if (Numactual == a){
      client.print("<tr><td>*<form action='PROG' id='F2'>");
    }else{
      client.print("<tr><td><form action='PROG' id='F2'>");
    }
    client.print("<input type='time' name='horamemo");
    client.print(a+1);
    client.print("' min='00:00' max='23:59' value='");
    String memoh = String(Memohora[a],DEC);
    if (memoh.length() == 1){
       memoh = '0' + memoh; 
    }
    client.print(memoh);
    client.print(":");
    String memom = String(Memominu[a],DEC);
    if (memom.length() == 1){
       memom = '0' + memom; 
    }            
    client.print(memom);
    client.print(":00'></td><td><input type='number' name='valor1' step='1' min='0' max='100' value='");
    client.print(Memovalor1[a]);
    client.print("'></td><td><input type='number' name='valor2' step='1' min='0' max='100' value='");
    client.print(Memovalor2[a]);
    if (Memovalor3[a] == 1){
      client.print("'></td><td><select name='valor3'><option value='activar' selected>Activado</option><option value='desactivar'>Desactivado</option></select>");
    }else{
      client.print("'></td><td><select name='valor3'><option value='desactivar' selected>Desactivado</option><option value='activar'>Activado</option></select>");
    }
    client.println("<input type='hidden' name='programador' value='0'></td><td><input type='submit' value='Cambiar'></form></td></tr>");
  }
  client.println("</table><br></br>");
  client.println("<h2>Activaci&oacute;n manual (muestra valores actuales. Si se modifica luces se para secuencia programas)</h2><br></br><table><tr><th>Luz Blanca</th><th>Luz Azul</th><th>Comedero</th></tr>");
  client.print("<tr><td><form action='VAL1' id='F3'><input type='number' name='valor1' min='0' max='100' value='");
  client.print(valors1);
  client.println("'><input type='hidden' name='valorfin1' value='0'><input type='submit' value='Cambiar'></form></td>");
  client.print("<td><form action='VAL2' id='F4'><input type='number' name='valor2' min='0' max='100' value='");
  client.print(valors2);
  client.println("'><input type='hidden' name='valorfin2' value='0'><input type='submit' value='Cambiar'></form></td>");
  client.print("<td><form action='VAL3' id='F4'><select name='valor3'>");
  if (valors3 == 0){
    client.print("<option value='desactivar' selected>Desactivado</option>");
    client.print("<option value='activar' >Activado</option>");
  }else{
    client.print("<option value='activar' selected>Activado</option>");
    client.print("<option value='desactivar'>Desactivado</option>");
  }            
  client.println("</select><input type='hidden' name='valorfin3' value='0'><input type='submit' value='Cambiar'></form></td></tr></table>");
  client.println("<br/>");
  //Reles
  client.println("<h2>Estado Reles y activaci&oacute;n manual JP1 y JP2</h2><br><table><tr><th>Rele CO2</th><th>Esterilizador</th><th>Rele 3</th><th>Rele 4</th></tr>");
  client.print("<tr><td><form action='JP1_1' id='F5'><select name='rele1jp1'>");
  if (digitalRead(releco2) == LOW){
    client.print("<option value='activar' selected>Activado</option>");
    client.print("<option value='desactivar'>Desactivado</option>");
  }else{
    client.print("<option value='desactivar' selected>Desactivado</option>");
    client.print("<option value='activar' >Activado</option>");
  }            
  client.println("</select><input type='hidden' name='rele1jp1fin' value='0'><input type='submit' value='Cambiar'></form></td>");
  client.print("<td><form action='JP1_2' id='F6'><select name='rele2jp1'>");
  if (digitalRead(esterilizador) == LOW){
    client.print("<option value='activar' selected>Activado</option>");
    client.print("<option value='desactivar'>Desactivado</option>");
  }else{
    client.print("<option value='desactivar' selected>Desactivado</option>");
    client.print("<option value='activar' >Activado</option>");
  }            
  client.println("</select><input type='hidden' name='rele2jp1fin' value='0'><input type='submit' value='Cambiar'></form></td>");
  client.print("<td><form action='JP1_3' id='F7'><select name='rele3jp1'>");
  if (digitalRead(jp1rele3) == LOW){
    client.print("<option value='activar' selected>Activado</option>");
    client.print("<option value='desactivar'>Desactivado</option>");
  }else{
    client.print("<option value='desactivar' selected>Desactivado</option>");
    client.print("<option value='activar' >Activado</option>");
  }            
  client.println("</select><input type='hidden' name='rele3jp1fin' value='0'><input type='submit' value='Cambiar'></form></td>");
  client.print("<td><form action='JP1_4' id='F8'><select name='rele4jp1'>");
  if (digitalRead(jp1rele4) == LOW){
    client.print("<option value='activar' selected>Activado</option>");
    client.print("<option value='desactivar'>Desactivado</option>");
  }else{
    client.print("<option value='desactivar' selected>Desactivado</option>");
    client.print("<option value='activar' >Activado</option>");
  }            
  client.println("</select><input type='hidden' name='rele4jp1fin' value='0'><input type='submit' value='Cambiar'></form></td></tr><br>");
  //Reles JP2
  client.println("<tr><th>Rele calentador</th><th>Rele Comedero</th><th>Rele dosificadora</th><th>Rele 4</th></tr>");
  client.print("<tr><td><form action='JP2_1' id='F9'><select name='rele1jp2'>");
  if (digitalRead(calentador) == LOW){
    client.print("<option value='activar' selected>Activado</option>");
    client.print("<option value='desactivar'>Desactivado</option>");
  }else{
    client.print("<option value='desactivar' selected>Desactivado</option>");
    client.print("<option value='activar' >Activado</option>");
  }            
  client.println("</select><input type='hidden' name='rele1jp2fin' value='0'><input type='submit' value='Cambiar'></form></td>");
  client.print("<td><form action='JP2_2' id='F10'><select name='rele2jp2'>");
  if (digitalRead(comedero) == LOW){
    client.print("<option value='activar' selected>Activado</option>");
    client.print("<option value='desactivar'>Desactivado</option>");
  }else{
    client.print("<option value='desactivar' selected>Desactivado</option>");
    client.print("<option value='activar' >Activado</option>");
  }            
  client.println("</select><input type='hidden' name='rele2jp2fin' value='0'><input type='submit' value='Cambiar'></form></td>");
  client.print("<td><form action='JP2_3' id='F11'><select name='rele3jp2'>");
  if (digitalRead(dosificadora) == LOW){
    client.print("<option value='activar' selected>Activado</option>");
    client.print("<option value='desactivar'>Desactivado</option>");
  }else{
    client.print("<option value='desactivar' selected>Desactivado</option>");
    client.print("<option value='activar' >Activado</option>");
  }            
  client.println("</select><input type='hidden' name='rele3jp2fin' value='0'><input type='submit' value='Cambiar'></form></td>");
  client.print("<td><form action='JP2_4' id='F12'><select name='rele4jp2'>");
  if (digitalRead(jp2rele4) == LOW){
    client.print("<option value='activar' selected>Activado</option>");
    client.print("<option value='desactivar'>Desactivado</option>");
  }else{
    client.print("<option value='desactivar' selected>Desactivado</option>");
    client.print("<option value='activar' >Activado</option>");
  }            
  client.println("</select><input type='hidden' name='rele4jp2fin' value='0'><input type='submit' value='Cambiar'></form></td></tr></table><br></br>");
  client.println("</body></html>");
}


