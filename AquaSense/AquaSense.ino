// ======================================================
// PROJETO ALERTA DE ALAGAMENTO
// ESP + 2 sensores ultrassônicos + LEDs + buzzer + Java
// ======================================================

#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>

// ======================================================
// CONFIGURAÇÕES DO WI-FI E SERVIDOR JAVA
// ======================================================

const char* WIFI_NOME = "FAB LAB GUARAPIRANGA";
const char* WIFI_SENHA = "@&*itS104";

// Coloque aqui o IP do computador onde o Spring Boot está rodando
String URL_SERVIDOR = "http://192.168.18.20:8080/sensor/leitura";


// ======================================================
// PINOS
// ======================================================

// Sensor 1 - Bueiro
#define TRIG_BUEIRO 5     // D1
#define ECHO_BUEIRO 14    // D5

// Sensor 2 - Poste
#define TRIG_POSTE 4      // D2
#define ECHO_POSTE 12     // D6

// Saídas
#define LED_VERDE 13      // D7
#define LED_VERMELHO 15   // D8
#define BUZZER 16         // D0


// ======================================================
// CONFIGURAÇÕES DO SISTEMA
// ======================================================

// Quanto MENOR a distância, MAIS PERTO a água está do sensor
#define LIMITE_RISCO_BUEIRO 20
#define LIMITE_ALAGADO_POSTE 20

// Altura total do bueiro em cm
// Ajuste conforme o protótipo de vocês
#define ALTURA_TOTAL_BUEIRO 100

// Tempo máximo para leitura do sensor
#define TIMEOUT_SENSOR 30000

// Intervalos
#define INTERVALO_LEITURA 500
#define INTERVALO_ENVIO_JAVA 5000

unsigned long ultimoEnvioJava = 0;


// ======================================================
// CONFIGURAÇÃO INICIAL
// ======================================================

void setup() {
  Serial.begin(9600);

  configurarPinos();
  conectarWiFi();

  Serial.println("Sistema de Alerta de Alagamento iniciado...");
}


// ======================================================
// LOOP PRINCIPAL
// ======================================================

void loop() {
  float distanciaBueiro = medirDistancia(TRIG_BUEIRO, ECHO_BUEIRO);
  float distanciaPoste = medirDistancia(TRIG_POSTE, ECHO_POSTE);

  int alturaAgua = calcularAlturaAgua(distanciaBueiro);
  String statusAtual = verificarStatus(distanciaBueiro, distanciaPoste);

  exibirLeitura(distanciaBueiro, distanciaPoste, alturaAgua, statusAtual);
  controlarAlerta(statusAtual);

  if (podeEnviarParaJava()) {
    enviarParaJava(alturaAgua);
    ultimoEnvioJava = millis();
  }

  delay(INTERVALO_LEITURA);
}


// ======================================================
// CONFIGURAÇÃO DOS PINOS
// ======================================================

void configurarPinos() {
  pinMode(TRIG_BUEIRO, OUTPUT);
  pinMode(ECHO_BUEIRO, INPUT);

  pinMode(TRIG_POSTE, OUTPUT);
  pinMode(ECHO_POSTE, INPUT);

  pinMode(LED_VERDE, OUTPUT);
  pinMode(LED_VERMELHO, OUTPUT);
  pinMode(BUZZER, OUTPUT);

  desligarAlertas();
}


// ======================================================
// CONEXÃO WI-FI
// ======================================================

void conectarWiFi() {
  Serial.println();
  Serial.println("==================================");
  Serial.println("       CONECTANDO AO WI-FI");
  Serial.println("==================================");

  WiFi.begin(WIFI_NOME, WIFI_SENHA);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("Wi-Fi conectado com sucesso!");

  Serial.print("IP do ESP: ");
  Serial.println(WiFi.localIP());

  Serial.println("==================================");
}


// ======================================================
// MEDIÇÃO DO SENSOR ULTRASSÔNICO
// ======================================================

float medirDistancia(int trig, int echo) {
  digitalWrite(trig, LOW);
  delayMicroseconds(2);

  digitalWrite(trig, HIGH);
  delayMicroseconds(10);

  digitalWrite(trig, LOW);

  long duracao = pulseIn(echo, HIGH, TIMEOUT_SENSOR);

  if (duracao == 0) {
    return -1;
  }

  float distancia = duracao * 0.034 / 2;

  return distancia;
}


// ======================================================
// CÁLCULO DA ALTURA DA ÁGUA
// ======================================================

int calcularAlturaAgua(float distanciaBueiro) {
  if (distanciaBueiro == -1) {
    return 0;
  }

  int altura = ALTURA_TOTAL_BUEIRO - distanciaBueiro;

  if (altura < 0) {
    altura = 0;
  }

  if (altura > ALTURA_TOTAL_BUEIRO) {
    altura = ALTURA_TOTAL_BUEIRO;
  }

  return altura;
}


// ======================================================
// VERIFICAÇÃO DO STATUS
// ======================================================

String verificarStatus(float distanciaBueiro, float distanciaPoste) {
  if (distanciaBueiro == -1 || distanciaPoste == -1) {
    return "ERRO_SENSOR";
  }

  if (distanciaPoste <= LIMITE_ALAGADO_POSTE) {
    return "ALAGADO";
  }

  if (distanciaBueiro <= LIMITE_RISCO_BUEIRO) {
    return "RISCO";
  }

  return "NORMAL";
}


// ======================================================
// ENVIO PARA O JAVA
// ======================================================

bool podeEnviarParaJava() {
  return millis() - ultimoEnvioJava >= INTERVALO_ENVIO_JAVA;
}

void enviarParaJava(int altura) {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println();
    Serial.println("Wi-Fi desconectado. Tentando reconectar...");
    conectarWiFi();
    return;
  }

  WiFiClient client;
  HTTPClient http;

  http.begin(client, URL_SERVIDOR);
  http.addHeader("Content-Type", "application/json");

  String json = "{\"altura\":" + String(altura) + "}";

  int codigoResposta = http.POST(json);

  Serial.println();
  Serial.println("==================================");
  Serial.println("        ENVIO PARA O JAVA");
  Serial.println("==================================");
  Serial.print("URL: ");
  Serial.println(URL_SERVIDOR);

  Serial.print("JSON enviado: ");
  Serial.println(json);

  Serial.print("Codigo HTTP: ");
  Serial.println(codigoResposta);

  if (codigoResposta > 0) {
    String resposta = http.getString();
    Serial.println("Resposta do servidor:");
    Serial.println(resposta);
  } else {
    Serial.println("Erro ao enviar dados para o Java.");
  }

  Serial.println("==================================");

  http.end();
}


// ======================================================
// CONTROLE DOS ALERTAS
// ======================================================

void controlarAlerta(String statusAtual) {
  if (statusAtual == "NORMAL") {
    ligarNormal();
  } 
  else if (statusAtual == "RISCO") {
    ligarRisco();
  } 
  else if (statusAtual == "ALAGADO") {
    ligarAlagado();
  } 
  else {
    erroSensor();
  }
}


// ======================================================
// ESTADOS DO SISTEMA
// ======================================================

void ligarNormal() {
  digitalWrite(LED_VERDE, HIGH);
  digitalWrite(LED_VERMELHO, LOW);
  noTone(BUZZER);
}

void ligarRisco() {
  digitalWrite(LED_VERDE, LOW);
  digitalWrite(LED_VERMELHO, HIGH);
  sireneCurta();
}

void ligarAlagado() {
  digitalWrite(LED_VERDE, LOW);
  digitalWrite(LED_VERMELHO, HIGH);
  sirene();
}

void erroSensor() {
  digitalWrite(LED_VERDE, LOW);
  digitalWrite(LED_VERMELHO, HIGH);
  noTone(BUZZER);

  Serial.println("Erro: sensor sem leitura.");
}

void desligarAlertas() {
  digitalWrite(LED_VERDE, LOW);
  digitalWrite(LED_VERMELHO, LOW);
  noTone(BUZZER);
}


// ======================================================
// BUZZER
// ======================================================

void sireneCurta() {
  tone(BUZZER, 1200);
  delay(150);
  noTone(BUZZER);
  delay(150);
}

void sirene() {
  for (int freq = 800; freq <= 2000; freq += 50) {
    tone(BUZZER, freq);
    delay(5);
  }

  for (int freq = 2000; freq >= 800; freq -= 50) {
    tone(BUZZER, freq);
    delay(5);
  }
}


// ======================================================
// EXIBIÇÃO NO MONITOR SERIAL
// ======================================================

void exibirLeitura(float distanciaBueiro, float distanciaPoste, int alturaAgua, String statusAtual) {
  Serial.println();
  Serial.println("==================================");
  Serial.println("     MONITORAMENTO DA AGUA");
  Serial.println("==================================");

  Serial.print("Sensor do bueiro : ");
  if (distanciaBueiro == -1) {
    Serial.println("Sem leitura");
  } else {
    Serial.print(distanciaBueiro);
    Serial.println(" cm");
  }

  Serial.print("Altura da agua   : ");
  Serial.print(alturaAgua);
  Serial.println(" cm");

  Serial.print("Sensor do poste  : ");
  if (distanciaPoste == -1) {
    Serial.println("Sem leitura");
  } else {
    Serial.print(distanciaPoste);
    Serial.println(" cm");
  }

  Serial.print("Status atual     : ");
  Serial.println(statusAtual);

  Serial.println("----------------------------------");

  if (statusAtual == "NORMAL") {
    Serial.println("Situacao: Nivel seguro.");
  } 
  else if (statusAtual == "RISCO") {
    Serial.println("Situacao: Agua alta no bueiro.");
    Serial.println("Acao: Alerta de risco acionado.");
  } 
  else if (statusAtual == "ALAGADO") {
    Serial.println("Situacao: Agua detectada no poste.");
    Serial.println("Acao: Alagamento confirmado!");
  } 
  else {
    Serial.println("Situacao: Falha na leitura dos sensores.");
    Serial.println("Acao: Verificar conexoes.");
  }

  Serial.println("==================================");
}
