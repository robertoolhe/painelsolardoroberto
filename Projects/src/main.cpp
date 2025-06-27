#include <Arduino.h>
#include <TFT_eSPI.h>
#include <SPI.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <String.h>
#include "arduino_secrets.h" // Você deve criar este arquivo com suas credenciais
#include <ESP_Google_Sheet_Client.h>
#include <time.h>

// Substitua pelos dados do seu arquivo JSON da conta de serviço
#define CLIENT_EMAIL "esp32painelsolardoroberto@painelsolardoroberto.iam.gserviceaccount.com"
#define PROJECT_ID "painelsolardoroberto"
#define PRIVATE_KEY "-----BEGIN PRIVATE KEY-----\nMIIEvQIBADANBgkqhkiG9w0BAQEFAASCBKcwggSjAgEAAoIBAQC4bIoiB4itb1yu\nwMyh8qMF+vTJ5UsOcLobYZ96UC0VEd1MY8msuxAlXY5qbEEFo9q3goQqR1XbnPJQ\nfJVL80zimNW/veUNtDSS802fMHJbhdg35adFSIm2BeMQujMchJSSTnozVkF1UjGP\n5464mmAJMsachyqUjw+QMmYuiWu0bPgPGmnUhpFSu5ly9GXXBpzeDvNU/8b7DSBT\ncd1Q1tdobB26bQkTUCREZjPvIzOqfUhod3uhq78Gd7wnCh0hjBgK+84/D6X7Iyhc\nnh8ggynEp2Jwb2LDB1AbrcnkG6BAOTK4Nq8J1Aa0gM7tTh2wLVyIP5mwFrKhYWoA\n130gE80rAgMBAAECggEAUyetiPZ4rCrgUXNhUUxEMXgcU8RteU9euuXjsJTiHvP1\nInKEBrw55X8vrdCt6Mse3UueKCoODn3PzHbAIoTLkAh1qIUI+Iregbvure58QcQx\n39JO/7BbV5WD8pDiZuNo0idMdkVYMnwjGM3Bzn7c+ojIgN95VY7D57Kx3B3eUHVl\noEI02IRd5DtogV5JYogNc5ARoF9irADmWxeN8HVZaNAT9uCVlPbA9IohLl4kAmRS\n4HquKqkkMLFC0cblfMJyl9BXBRug1z0YXUBQdIaFZqTLL/0R5fUq8Ghf+JS/n5TH\nJqPFOenADJigyqW6xIvg105YXU0QpGOp5kmpMvpisQKBgQDj8i22yWfDoZGLIgs1\nZCeeuJsN6xGOXsQqishBRfJ06u84h+ER6bWKVhSTHi5LJ6HFkPRkDaeFO5jnly1V\nXRKZFQvUa58L9v76CJ9qpdOeyiu3UnYB3yJvwbmHRAyc96RiDm+4fQRlXuEFi8vw\nigN5MamDUeitCoHMdxv6WXtXPQKBgQDPHyBkbTIMvPywRy8PjklcVjstDikJKIep\nbP0Lm7JHBrONy0IiDLe162GbDzv2EsBvnqZuxWuocPwmrk7Cx8jXAxSHGB3ikoOn\nXlVA9KxxK0pS5pigvz03fYgR5QeLFhaVsEi0oA1qxSY+//kGOzFGQz1ThDL+u5X5\nmC/Gy1i8hwKBgAwn8yYnUYBOUnuwAhcD01UUlThFy/biuOVn50wL1eVETiOeo3MY\nGpVZB9ncpy1c3LSziUT4sXFaf0oCdulxgDdntuzKHH4/2tMsuIuwcjuqnnA1VpO8\n+ZIqK6G6EE4iqsKL8ItPJ3fcenWYQfT+9zZ0XozhRl5MDCYCSByuvvQlAoGAfGyM\nfwgCCxj1C8iXgtbNR0UeI4DgMfpBFQy+Lt0HiheCgena/q15JzYR2p6aPMcjB2rd\nPKoodHX7ZBlOg0CVbGUTTdy1B0lRAZhvyqexeAKzkX2prtdzpQQqW+WkVG+efan6\n41dK+BsbULlhFdc1UKpQCv3dzJ1QCTLdcKTz170CgYEAko8uI/ksNXRyxtYPlXwn\nLHhbzcWtKxtjRevUI40Nen3GgYzj1Nf06nN4L8iYF4fhLo6OuZDIIjIcNythN6bV\nNX9eKJ8dQ21SyAE5KzGtkXtO20ZbLrRoZHFU9U6LVaKT3Xh7RyZktUWE4BPZpe/K\ndH3iBsYDVcTMhUsVtiXqQpM=\n-----END PRIVATE KEY-----\n"

// ID da sua planilha (pegue na URL do Google Sheets)
#define SHEET_ID "1L8qyOAQR0OvTN1L5sn_vzfEANAgseM9iVS9GvDphNT0"

// Definições e variáveis
unsigned long tempoInicial;
const int tempoEspera = 0.5 * 60 * 1000;

int analogPin = 34;
int leitura = 0;
int somaLeituras = 0;
float voltagem = 0;

const int numLeituras = 128;
String faixa = "branca";

float fator = 550.0; // Fator de correção para a leitura do ADC


#define BUTTON_1 35 
#define BUTTON_2 0  
const int buzzerPin = 12;

void bipe() {
  tone(buzzerPin, 500);
  delay(200);
  noTone(buzzerPin);
  delay(100);
}

#define TFT_GREY 0x5AEB
TFT_eSPI tft = TFT_eSPI();

char ssid[] = SECRET_SSID;
char pass[] = SECRET_PASS;
int status = WL_IDLE_STATUS;
const char* hora_min = "";


void checkAndPrintWiFiStatus() {
  status = WiFi.status();
  while (status != WL_CONNECTED) {
    tft.println("Lost connection - attempting to re-connect to SSID: ");
    WiFi.reconnect();
    WiFi.setSleep(false);
    delay(10000);
    tft.println(ssid);
    status = WiFi.status();
  }
  IPAddress ip = WiFi.localIP();
  long rssi = WiFi.RSSI();
}


void tokenStatusCallback(TokenInfo info) {
  String msg = "Token status: " + String(info.status);
  Serial.println(msg);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextFont(2);
  //tft.setCursor(0, 60, 2); // Ajuste a posição conforme necessário
  tft.println(msg);

  if (info.status == token_status_error) {
    String err = "Token error: " + String(GSheet.getTokenError(info).c_str());
    Serial.println(err);
    tft.setTextColor(TFT_RED, TFT_BLACK);
    //tft.setCursor(0, 80, 2); // Linha abaixo do status
    tft.println(err);
  }
    #include "../meus_secrets/arduino_secrets.h"// Pausa de 15 segundos após exibir as mensagens
  delay(15000);
}




void setup() {
  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_GREY);
  tft.setCursor(0, 0, 2);
  tft.setTextColor(TFT_WHITE,TFT_BLACK);  tft.setTextSize(1);
  tft.println("Iniciando...");
  bipe();
  delay(1000);

  GSheet.setTokenCallback(tokenStatusCallback);
  GSheet.begin(CLIENT_EMAIL, PROJECT_ID, PRIVATE_KEY);

  pinMode(BUTTON_1, INPUT_PULLUP);
  pinMode(BUTTON_2, INPUT_PULLUP);
  Serial.begin(115200);
  tempoInicial = millis();
  while (!Serial) { ; }

  delay(100);
  pinMode(3, OUTPUT);
  digitalWrite(3, LOW);
  delay(100);
  pinMode(14, OUTPUT); 
  digitalWrite(14, HIGH);
  WiFiGenericClass::mode(WIFI_MODE_STA);
  tft.fillScreen(TFT_PURPLE);
  tft.setCursor(0, 0, 2);
  tft.print("conectando: ");
  status = WiFi.begin(ssid, pass);
  WiFi.setSleep(false);

  while (status != WL_CONNECTED) {
    delay(10000);
    tft.print("Aguardando Wifi: ");
    tft.println(ssid);
    status = WiFi.status();
  }
  tft.fillScreen(TFT_GREEN);
  tft.setCursor(0, 0, 2);
  tft.println("WiFi OK");

  configTime(0, 0, "a.st1.ntp.br");
  setenv("TZ", "GMT+3", 1); // GMT+3 significa UTC-3 (Brasil)
  tzset();
  GSheet.setSystemTime(time(nullptr));
}

void loop() {
  checkAndPrintWiFiStatus();

  // Verifica se a autenticação está pronta
  if (!GSheet.ready()) {
    // Não faz nada até estar pronto
    delay(100);
    return;
  }


  if (digitalRead(BUTTON_1) == LOW) {
    fator = fator-0.50;
    delay(100);
  }
  if (digitalRead(BUTTON_2) == LOW) {
    fator = fator+1.50;
    delay(100);
  }
  delay(20); 


  if (millis() - tempoInicial >= tempoEspera) {
    for (int i = 0; i < numLeituras; i++) {
      leitura = analogRead(analogPin);
      somaLeituras += leitura;
      delay(10);
    }
    int mediaLeituras = (int)somaLeituras / numLeituras;
    float volts = (mediaLeituras/fator);
    float voltagem = (volts*4);
    somaLeituras = 0;

    FirebaseJson json;
    FirebaseJsonArray row;

    // Coluna 1: item (deixe vazio para o Google Sheets preencher)
    row.add(""); 

    // Coluna 2: nome fixo
    row.add("voltagem");

    // Coluna 3: data/hora (exemplo usando millis, substitua por RTC ou NTP se quiser data real)
    time_t now = time(nullptr);
    now -= 3 * 3600; // Ajuste manual para UTC-3 (Brasil)
    struct tm timeinfo;
    localtime_r(&now, &timeinfo);

    char dataHora[32];
    strftime(dataHora, sizeof(dataHora), "%Y-%m-%d %H:%M:%S", &timeinfo);
    const char* tz = getenv("TZ");
    if (tz) {
        Serial.print("Timezone atual (TZ): ");
        Serial.println(tz);
    } else {
        Serial.println("Timezone atual (TZ): não definido");
    }


    String dataehora = String(dataHora);



    row.add(dataehora); // Adiciona a data/hora como texto

    // Coluna 4: voltagem
    row.add(voltagem);

    FirebaseJsonArray values;
    values.add(row);

    json.add("values", values);

    FirebaseJson response;
    const char* range = "dados!A1";
    const char* valueInputOption = "USER_ENTERED";
    const char* insertDataOption = "INSERT_ROWS";
    const char* includeValuesInResponse = "false";
    const char* responseValueRenderOption = "FORMATTED_VALUE";
    const char* responseDateTimeRenderOption = "SERIAL_NUMBER";

    bool success = GSheet.values.append(
      &response, SHEET_ID, range, &json,
      valueInputOption, insertDataOption,
      includeValuesInResponse, responseValueRenderOption, responseDateTimeRenderOption
    );

    if (success) {
      Serial.println("Dados enviados para o Google Sheets!");
      response.toString(Serial, true);
    } else {
      Serial.print("Erro ao enviar para Google Sheets: ");
      Serial.println(GSheet.errorReason());
    }

    tempoInicial = millis();
    // Exibe os dados no Serial Monitor
    Serial.println("====================================");
    Serial.println(voltagem);
    Serial.println("Fator: " + String(fator));
    Serial.println("Leitura: " + String(leitura));
    Serial.println("Volts: " + String(volts));
    Serial.println("Voltagem: " + String(voltagem));
    Serial.println("Faixa: " + faixa);
    Serial.print("Variável dataHora:  ");
    Serial.println(dataHora);
    Serial.print("Data/hora capturada: ");
    Serial.println(dataehora);
    Serial.print("Timezone atual (TZ): ");
    Serial.println(tz ? tz : "não definido");
    Serial.println("====================================");


    // Mostra na tela TFT
    tft.fillScreen(TFT_BLACK);
    tft.setCursor(0, 0, 2);
    tft.setTextColor(TFT_WHITE,TFT_BLACK);  tft.setTextSize(1);
    tft.println("Status Bateria by Roberto La Bella");
    tft.println("");
    tft.setTextColor(TFT_BLUE,TFT_BLACK); 
    tft.setTextFont(7);
    String strvoltagem = String(voltagem);
    tft.print(strvoltagem);

    if       (voltagem >= 13.800) {
      tft.setTextColor(TFT_GREEN,TFT_GREY); 
      faixa = "100";
    } else if (voltagem >= 13.400 && voltagem <= 13.799) {
      tft.setTextColor(TFT_GREEN,TFT_BLACK); 
      faixa = " 90";
    } else if (voltagem >= 13.280 && voltagem <= 13.399) {
      tft.setTextColor(TFT_GREEN,TFT_GREY);
      faixa = " 80";
    } else if (voltagem >= 13.200 && voltagem <= 13.279) {
      tft.setTextColor(TFT_GREEN,TFT_GREY);
      faixa = " 70";
    } else if (voltagem >= 13.080 && voltagem <= 13.199) {
      tft.setTextColor(TFT_YELLOW,TFT_GREY);
      faixa = " 60";
    } else if (voltagem >= 13.040 && voltagem <= 13.079) {
      tft.setTextColor(TFT_ORANGE,TFT_GREY);
      faixa = " 50";
    } else if (voltagem >= 13.000 && voltagem <= 13.039) {
      tft.setTextColor(TFT_ORANGE,TFT_GREY);
      faixa = " 40";
    } else if (voltagem >= 12.880 && voltagem <= 12.999) {
      tft.setTextColor(TFT_RED,TFT_GREY);  
      faixa = " 30";
    } else if (voltagem >= 12.800 && voltagem <= 12.879) {
      tft.setTextColor(TFT_RED,TFT_GREY);  
      faixa = " 20";   
    } else if (voltagem <= 12.799) {
      tft.setTextColor(TFT_PURPLE,TFT_GREY);  
      faixa = "Off<20";
      bipe();
      bipe();    
    }
    tft.setTextFont(7);
    tft.println(faixa);
    tft.setTextColor(TFT_WHITE,TFT_BLUE);
    tft.setTextFont(2);
    tft.print("         Volts         ");
    tft.setTextColor(TFT_PURPLE,TFT_WHITE);
    tft.println("     Carga     ");
    tft.setTextColor(TFT_YELLOW,TFT_BLACK);
    String strleitura = ( String(leitura) );
    tft.print(strleitura);
    tft.print("/");
    tft.print(fator);
    tft.print("=");
    tft.print(volts);
    tft.print("x4=");
    tft.println(volts*4);
    tft.setTextColor(TFT_GREY, TFT_WHITE);
    tft.setTextFont(2);
    tft.print("em: ");
    tft.println(dataHora);

  }

  delay(20);


}