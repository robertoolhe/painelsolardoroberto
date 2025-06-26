#include <Arduino.h>
#include <TFT_eSPI.h>
#include <SPI.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <String.h>
#include "arduino_secrets.h" // Você deve criar este arquivo com suas credenciais
#include <ESP_Google_Sheet_Client.h>

// Substitua pelos dados do seu arquivo JSON da conta de serviço
#define CLIENT_EMAIL "esp32painelsolardoroberto@painelsolardoroberto.iam.gserviceaccount.com"
#define PROJECT_ID "painelsolardoroberto"
#define PRIVATE_KEY "-----BEGIN PRIVATE KEY-----\nMIIEvwIBADANBgkqhkiG9w0BAQEFAASCBKkwggSlAgEAAoIBAQDh8pw+5NTLTlsO\nWO+PCiXjoEHMsN+Y4tD/50mbWjZ0Jjs/w8DYR/vJ/D3EZwD3DyM69xHZch/7bdSL\nGOvqBwirfyjACSfC6e7ofitlO4GOMo47M8FQ2HZY2wXPWHxFheF4YFH1v+18Vic4\n/A+TvlL3Mc451BV0S7AleFDXnZKA/V3+M4Xe2onUwh5gwlk56OGKjaNSG7me0SLx\nCJmEjqEqnszA6uBrLw4hiZntdty+CHGnh8yea01/e8rP6ZBpjQklWzhynI+aSpjg\n/PCMHnjQnjbivoekUlGgodC0/zPoJvR6F6UXCe5GY3Wg3aXKvMIIa06cKccwEtx6\nEI1cTJfPAgMBAAECggEAG39IA+DGA3v6BDP563bOXPZQK6tMJYdXl10RYyZGbDo0\nNF1hUbDo8+WOVXbnnNQrHcObA33VEJHcL5VigyzvTC0By4+8VrHbe6XR0tRmvuRo\n1MxGoXfGZ1KNpsnboyZY5dUVrwpNdOKTNrx433+7tGOg96lXdxC4U3G3UkDV5yaJ\nPtP+XboBo1O+pDVy6tZ+GW+nuGSjdbw5tjwwk2W2jvgl3g3XMQ5flCBIOViaP6nk\nuE4Pn6RvRI1H1hoz9ZUqtbllaN7Zc/NtRFUry6ojfqHp2UGzxdOQI0+N6g83HKif\nZd5MYwugad9J8GBHn3f8BNvKSPisnSnStYmR/Ktc+QKBgQD5hPhYWmPXUDQwRrqs\nhBbnDGp5s9wVQu3jAwLLRNHOgZ9AuNBnK+ZtxTV1Ri9q7lp1jBy88jWoFCL1g6jk\nzXgyKlmVhkjuDVLlgdyw6QbZVslYpT1XshIzqxXXH0nF6FDaGlKlMU4Qi9OPm5/+\noTKtG+MNWSux2s9cZ2OrRG+N3QKBgQDn0OoKGfDp1Svfcfw7r24q5fLvq3Cd9aJ+\nlIxgLZUb3RdmmW3w45r9bYFg/4rc06/w7kZKUxgOFxK4McVT78GBQw0tM66eEbH0\ny1WctD7G5i4Vhz9Psf2tE2sL1t+ZuGq8EPKPMQuPfaZ9mKMHLKIjqWwXCiCk9udt\nTkAQ6nzPmwKBgQDBY2e/wr7jIExypj8EepPtm1fi353R9L+/VJQTy1D5RsmRNix/\n6ix8I37dV/pkXDxMIr4bS3Z+wKyfsN10CDQCXYR1OVDJABahvta3XsRqKrN/OwKi\nN2eYGB+jTaK1+uq/P+uSEPa6KEVeZnXIulM2jAicTeJpxHc23QMIuM/lGQKBgQDk\nqwDiW/p4Pq5baK3+FivfJxs/7eZV+sKCiHf8O5qVdL8rlveLIol1qfpwu0K4WBx6\n1iACGvtkksAFcBsNlDhWENVDnHKIPD5FkfnubSJrwwz8cYAzVgk9HCWA9UF1+iNX\nRoRuWVvk/HYp+FIAtrdt5Cbah0PyVheti/3IcfYSNwKBgQDw1V+yVnJfrjdWOffY\nAZPqm8sDEPLaLohlyI0II3Y/MiBPsFv3mkIBzqG47H1IFZ4gbJBQY2he0bXUYYgt\nWvQapjDNW1edwUgrc9EBgiBNso2PGPpZWmnxUVm9eTDgTTAZ8kcpXUvy2DtT/O7l\n1JBAnBmkD4k432wmioVrAN5LPA==\n-----END PRIVATE KEY-----\n"

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

float fator = 540.0; // Fator de correção para a leitura do ADC


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

ESP_Google_Sheet_Client sheet; // objeto principal do client
ServiceAccount_t sa;           // dados da conta de serviço

void setup() {
  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_GREY);
  tft.setCursor(0, 0, 2);
  tft.setTextColor(TFT_WHITE,TFT_BLACK);  tft.setTextSize(1);
  tft.println("Iniciando...");
  bipe();
  delay(1000);

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

  sa.client_email = CLIENT_EMAIL;
  sa.project_id = PROJECT_ID;
  sa.private_key = PRIVATE_KEY;

  sheet.begin(&sa);
}

void loop() {
  checkAndPrintWiFiStatus();

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

    // Monta os dados para enviar para a planilha
    String range = "dados!A1"; // = 'nomedaaba'!A1
    String values = "[[\"" + String(voltagem) + "\",\"" + String(mediaLeituras) + "\",\"" + String(volts) + "\"]]";

    if (sheet.append(SHEET_ID, range, values)) {
      Serial.println("Dados enviados para o Google Sheets!");
    } else {
      Serial.println(sheet.errorReason());
    }

    tempoInicial = millis();

  
    Serial.println(voltagem);

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
    tft.print(volts*4);

  }

  delay(20);
}