//variáveis para tempo de espera de 1/2 minuto, ou seja, INSERT da tensão a cada 30 segundos
unsigned long tempoInicial;
const int tempoEspera = 0.5 * 60 * 1000; // 1 = 1 minuto(s) em milissegundos, 0.5 = 30 segundos

int analogPin = 34; // Define o pino GPIO 34 como a0
int leitura = 0;
int somaLeituras = 0;
float voltagem = 0;
int item = 0;
int registro = 0;
const int numLeituras = 128;
String faixa = "branca";
char porcentagem = '%'; //  Serial.print("O caractere é: "); Serial.println(porcentagem);

float fator = 540.80; //fator para ajuste da leitura da tensão de AnalogRead(analogPin) GPIO34, exemplo: 540.80
// Button definitions (adjust these if your pins differ)
#define BUTTON_1 35 
#define BUTTON_2 0  

const int buzzerPin = 12;
void bipe() {
  // Emite um beep de 1 segundo
  tone(buzzerPin, 500); // Frequência de 1000 Hz (1 kHz)
  delay(200);           // Tempo de meio segundo
  noTone(buzzerPin);      // Desliga o buzzer
  delay(100);            // Intervalo de menos de meio segundo
}

#include <TFT_eSPI.h> // Graphics and font library para telas TFT como TT-GO T-DISPLAY
#include <SPI.h>

#define TFT_GREY 0x5AEB // New colour

TFT_eSPI tft = TFT_eSPI();  // Invoke library

//arduino_secrets.h contem ssid password e host do postgresql
// NeonPostgresOverHTTP example - https://github.com/neondatabase-labs/NeonPostgresOverHTTP
// Copyright © 2025, Peter Bendel and neondatabase labs (neon.tech)
#include <String.h>
#include <SPI.h>
#include <WiFi.h> // for XIAO ESP32 C6
#include <WiFiClientSecure.h>
#include "arduino_secrets.h" // copy arduino_secrets.h.example to arduino_secrets.h and fill in your secrets
#include <NeonPostgresOverHTTP.h>

///////please enter your sensitive data in the Secret tab/arduino_secrets.h
char ssid[] = SECRET_SSID;  // your network SSID (name)
char pass[] = SECRET_PASS;  // your network password (use for WPA, or use as key for WEP)

int status = WL_IDLE_STATUS;

const char* hora_min = "" ;

// Initialize the Ethernet client library
// with the IP address and port of the server
// that you want to connect to (port 80 is default for HTTP):
WiFiClientSecure client;
NeonPostgresOverHTTPProxyClient sqlClient(client, DATABASE_URL, NEON_PROXY);

const char* create = R"SQL(
CREATE TABLE if not exists sensorvalues(
  measure_time timestamp without time zone DEFAULT now() NOT NULL,
  voltagem float,
  sensor_name text NOT NULL, sensor_value float, PRIMARY KEY (sensor_name, measure_time, item)
)
)SQL";

const char* horalocal = R"SQL(
ALTER DATABASE dbpainel SET TIMEZONE = 'America/Sao_Paulo'
SET TIMEZONE TO 'America/Sao_Paulo'
SELECT *
FROM (
    SELECT measure_time AT TIME ZONE 'UTC' FROM sensorvalues
        ORDER BY measure_time ASC
) AS ultimos
SELECT current_setting('TIMEZONE')
)SQL";

const char* insert = R"SQL(
INSERT INTO SENSORVALUES (measure_time, sensor_name, sensor_value, voltagem ) VALUES (now(),'voltagem', $1::int, $2::float)
)SQL";


const char* query = R"SQL(
SELECT measure_time, item, sensor_value, sensor_name, voltagem, to_char(measure_time, 'HH24:MI') AS hora_min
FROM (SELECT * FROM sensorvalues 
      WHERE measure_time >= now() - interval '1440 minutes'
      ORDER BY item DESC LIMIT 1) subquery
WHERE item >0 
ORDER BY item ASC 
)SQL";

int counter = 1;

void setup() {
  tft.init();
  tft.setRotation(1); // opcional: tft.setRotation(TFT_180); ou TFT_0, TFT_90, TFT_270 ou 0,1,2 ou 3 
  tft.fillScreen(TFT_GREY);
  tft.setCursor(0, 0, 2);
  tft.setTextColor(TFT_WHITE,TFT_BLACK);  tft.setTextSize(1);
  tft.println("Iniciando...");
  bipe();
  delay(1000);

  pinMode(BUTTON_1, INPUT_PULLUP);
  pinMode(BUTTON_2, INPUT_PULLUP);
  //Initialize serial and wait for port to open:
  Serial.begin(115200);
  tempoInicial = millis();
  while (!Serial) {
    ;  // wait for serial port to connect. Needed for native USB port only
  }

  // attempt to connect to WiFi network:
  delay(100);
  pinMode(3, OUTPUT);
  digitalWrite(3, LOW);//turn on this function
  delay(100);
  pinMode(14, OUTPUT); 
  digitalWrite(14, HIGH);//use external antenna
  WiFiGenericClass::mode(WIFI_MODE_STA);
  tft.fillScreen(TFT_PURPLE);
  tft.setCursor(0, 0, 2);
  tft.print("conectando: ");
  status = WiFi.begin(ssid, pass);
  WiFi.setSleep(false);

  while (status != WL_CONNECTED) {
    // wait 10 seconds before trying to connect
    delay(10000);
    tft.print("Aguardando Wifi: ");
    //Serial.print("Waiting to connect to SSID: ");
    tft.println(ssid);
    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
    status = WiFi.status();
  }
  tft.fillScreen(TFT_GREEN);
  tft.setCursor(0, 0, 2);
  tft.println("WiFi OK");
  client.setInsecure(); // avoid cert validation

  // create table for sensor values
  //Serial.println("\nExecuting create table (if not exists) statement in Postgres database...");
  sqlClient.setQuery(horalocal);
  sqlClient.setQuery(create);
  const char* errorMessage = sqlClient.execute();
  while (errorMessage != nullptr) {
    //Serial.println(errorMessage);
    checkAndPrintWiFiStatus();
    errorMessage = sqlClient.execute();
  }
  
}

void loop() {

  checkAndPrintWiFiStatus();
  if (millis() - tempoInicial >= tempoEspera) {

    for (int i = 0; i < numLeituras; i++) {
      leitura = analogRead(analogPin); // Lê o valor do pino analógico A0 = GPIO34
      somaLeituras += leitura; // Acumula a leitura na variável somaLeituras
      delay(10); //Pausa para estabilização da leitura (opcional)
    }

    // Calcula a média das leituras (opcional)
    int mediaLeituras = (int)somaLeituras / numLeituras;

    int leitura = mediaLeituras;

    // Lê o sensor de Tensão (Voltagem) Vbat interno da TTGO T-Display em GPIO 34 (= A0 interno)
    // substituida pelo for acima : leitura = analogRead(analogPin);  // Lê o valor analógico do pino
    // divide a leitura por um fator, exemplo 571.48 ou 556.30 , calculado por ('val' leitura analógica / Voltagem Real) (meça com um multimetro)
    float volts = (leitura/fator);
    float voltagem = (volts*4);
    //Serial.println(voltagem);
    somaLeituras = 0; // Reseta a soma para a próxima iteração 

    //Serial.println("\nExecuting insert statement...");
    sqlClient.setQuery(horalocal);
    sqlClient.setQuery(insert);
    tempoInicial = millis(); // Resetar o temporizador

    JsonArray params = sqlClient.getParams();
    params.clear();
    params.add(counter++);
    params.add(voltagem);
    const char* errorMessage = sqlClient.execute();
    if (errorMessage != nullptr) {
      Serial.println(errorMessage);
    }

    // retrieve and print last 12 values in database
    //Serial.println("\nRetrieving last x sensor values...");
    sqlClient.setQuery(horalocal);
    sqlClient.setQuery(query);
    
    params.clear();
    errorMessage = sqlClient.execute();
    // // to debug:
    // serializeJson(sqlClient.getRawJsonResult(), Serial);
    // // or:
    //sqlClient.printRawJsonResult(Serial);

    if (errorMessage != nullptr) {
      //Serial.println(errorMessage);
    } else {
        //Serial.print("Rowcount: ");
        //Serial.println(sqlClient.getRowCount());
        JsonArray rows = sqlClient.getRows();
        for (JsonObject row : rows) {
          const char* measure_time = row["measure_time"];  // "2025-01-01 09:54:26.966508", ...
          const char* sensor_name = row["sensor_name"];    // "counter" ou "voltagem"
          float sensor_value = row["sensor_value"];
          const char* horamin = row["hora_min"]; // HH:MM
          int item = row["item"]; //item nr sequencial da tabela
          //Serial.print("Time: ");
          //Serial.print(measure_time);
          //Serial.print(" Sensor: ");
          //Serial.print(sensor_name);
          //Serial.print(" Value: ");
          //Serial.println(sensor_value); 
          //Serial.print(" Voltagem: ");
          //Serial.print(voltagem);
          registro = int(item);
          
                    
        }
    }
  
  Serial.println(voltagem);

  // INICIO DE MOSTRA CONTEÚDO NA TELA TFT
  // Fill screen with grey so we can see the effect of printing with and without 
  // a background colour defined
  //tft.fillScreen(TFT_GREY);
  tft.fillScreen(TFT_BLACK);
  
  // Set "cursor" at top left corner of display (0,0) and select font 2
  // (cursor will move to next line automatically during printing with 'tft.println'
  //  or stay on the line is there is room for the text with tft.print)
  tft.setCursor(0, 0, 2);
  // Set the font colour to be white with a black background, set text size multiplier to 1
  tft.setTextColor(TFT_WHITE,TFT_BLACK);  tft.setTextSize(1);
  // We can now plot text on screen using the "print" class
  tft.println("Status Bateria by Roberto La Bella");
  tft.println("");
  
  // Set the font colour to be yellow with no background, set to font 7
  tft.setTextColor(TFT_BLUE,TFT_BLACK); 
  tft.setTextFont(7);
  String strvoltagem = String(voltagem); // string que contém a Voltagem, ex: 12.80
  tft.print(strvoltagem);

  if       (voltagem >= 13.800) {
    tft.setTextColor(TFT_GREEN,TFT_GREY); 
    faixa = "100";  //VERDE 100%

  } else if (voltagem >= 13.400 && voltagem <= 13.799) {
    tft.setTextColor(TFT_GREEN,TFT_BLACK); 
    faixa = " 90";  //VERDE 90%

  } else if (voltagem >= 13.280 && voltagem <= 13.399) {
    tft.setTextColor(TFT_GREEN,TFT_GREY);
    faixa = " 80";  //VERDE 80%

  } else if (voltagem >= 13.200 && voltagem <= 13.279) {
    tft.setTextColor(TFT_GREEN,TFT_GREY);
    faixa = " 70";  //VERDE 70%

  } else if (voltagem >= 13.080 && voltagem <= 13.199) {
    tft.setTextColor(TFT_YELLOW,TFT_GREY);
    faixa = " 60";  //AMARELA 60%

  } else if (voltagem >= 13.040 && voltagem <= 13.079) {
    tft.setTextColor(TFT_ORANGE,TFT_GREY);
    faixa = " 50"; //LARANJA 50%

  } else if (voltagem >= 13.000 && voltagem <= 13.039) {
    tft.setTextColor(TFT_ORANGE,TFT_GREY);
    faixa = " 40"; //LARANJA 40%

  } else if (voltagem >= 12.880 && voltagem <= 12.999) {
    tft.setTextColor(TFT_RED,TFT_GREY);  
    faixa = " 30"; //VERMELHA 30%

  } else if (voltagem >= 12.800 && voltagem <= 12.879) {
    tft.setTextColor(TFT_RED,TFT_GREY);  
    faixa = " 20"; //VERMELHA 20%    

  } else if (voltagem <= 12.799) {
    tft.setTextColor(TFT_PURPLE,TFT_GREY);  
    faixa = "Off<20"; // RÔXA ABAIXO DE 20%
    bipe();
    bipe();    
  }
  
  tft.setTextFont(7);
  tft.println(faixa); //100% A <20%

  //tft.println(String(porcentagem)); // sinal de porcentagem %
  
  tft.setTextColor(TFT_WHITE,TFT_BLUE);
  tft.setTextFont(2);

  tft.print("         Volts         ");
  tft.setTextColor(TFT_PURPLE,TFT_WHITE);
  tft.println("     Carga     ");

  tft.setTextColor(TFT_YELLOW,TFT_BLACK);
  //tft.println ("");
  String strleitura = ( String(leitura) );
  tft.print(strleitura);
  tft.print("/");
  tft.print(fator);
  tft.print("=");
  tft.print(volts);
  tft.print("x4=");
  tft.print(volts*4);
  tft.print(" #");
  tft.println(registro);


  //FIM DO MOSTRA TELA

  }
 
  if (digitalRead(BUTTON_1) == LOW) {
    // Button 1 is pressed
    fator = fator-0.50;
    delay(100); // Debounce delay
  }
  if (digitalRead(BUTTON_2) == LOW) {
    // Button 2 is pressed
    fator = fator+1.50;
    delay(100); // Debounce delay
  }
  
  //wait 2 seconds before next request
  delay(2000);
}

void checkAndPrintWiFiStatus() {
  status = WiFi.status();
  while (status != WL_CONNECTED) {
    tft.println("Lost connection - attempting to re-connect to SSID: ");
    WiFi.reconnect();
    WiFi.setSleep(false);
    // wait 10 seconds before trying to connect again
    delay(10000);
    tft.println(ssid);
    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
    status = WiFi.status();
  }
  // print the SSID of the network you're attached to:
  //tft.print("SSID: ");
  //tft.println(WiFi.SSID());

  // print your board's IP address:
  IPAddress ip = WiFi.localIP();
  //tft.print("IP Address: ");
  //tft.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  //tft.print("signal strength (RSSI):");
  //tft.print(rssi);
  //tft.println(" dBm");
}




