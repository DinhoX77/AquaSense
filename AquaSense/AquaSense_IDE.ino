// ======================================================
// PROJETO ALERTA AGUA
// ESP32 + 2 sensores ultrassonicos + LEDs + buzzer + Java
// ======================================================

#include <WiFi.h>
#include <HTTPClient.h>

// ======================================================
// CONFIGURACOES DO WI-FI E DO SERVIDOR JAVA
// ======================================================

const char* WIFI_NOME = "FAB LAB GUARAPIRANGA";
const char* WIFI_SENHA = "@&*itS104";

// Confirme o IP do computador onde o Spring Boot esta rodando
const char* URL_SERVIDOR =
  "http://192.168.18.20:8080/sensor/leitura";

// ======================================================
// SENSOR DO BUEIRO
// ======================================================

#define TRIG_BUEIRO 5
#define ECHO_BUEIRO 18

// ======================================================
// SENSOR DA RUA
// ======================================================

#define TRIG_RUA 4
#define ECHO_RUA 15

// ======================================================
// LEDS
// ======================================================

#define LED_VERDE 21
#define LED_AMARELO 22
#define LED_VERMELHO 23

// ======================================================
// BUZZER
// ======================================================

#define BUZZER 19

// ======================================================
// CONFIGURACOES DA MAQUETE
// ======================================================

// Altura total interna do bueiro da maquete
const float ALTURA_TOTAL_BUEIRO = 28.0;

// Aciona atencao quando a agua fica a 20 cm ou menos do sensor
// Equivale a aproximadamente 8 cm ou mais de agua no bueiro
const float LIMITE_DISTANCIA_BUEIRO = 20.0;

// Aciona alagamento quando o sensor da rua detecta algo
// a menos de 12 cm
const float LIMITE_DISTANCIA_RUA = 12.0;

// Intervalo entre envios ao backend
const unsigned long INTERVALO_ENVIO_JAVA = 1000;

// Intervalo entre tentativas de reconexao Wi-Fi
const unsigned long INTERVALO_RECONEXAO_WIFI = 10000;

// ======================================================
// VARIAVEIS DE CONTROLE
// ======================================================

int contadorFalhas = 0;

unsigned long ultimoEnvioJava = 0;
unsigned long ultimaTentativaWiFi = 0;

bool wifiEstavaConectado = false;

// ======================================================
// LEITURA DOS SENSORES
// Utiliza a mediana de 7 medicoes para reduzir oscilacoes
// ======================================================

float lerSensor(int trig, int echo)
{
  const int N = 7;
  float leituras[N];

  for (int i = 0; i < N; i++)
  {
    digitalWrite(trig, LOW);
    delayMicroseconds(2);

    digitalWrite(trig, HIGH);
    delayMicroseconds(10);

    digitalWrite(trig, LOW);

    long duracao = pulseIn(echo, HIGH, 20000);

    if (duracao == 0)
      leituras[i] = 999;
    else
      leituras[i] = duracao * 0.0343 / 2.0;

    delay(10);
  }

  // Ordenacao das leituras
  for (int i = 0; i < N - 1; i++)
  {
    for (int j = i + 1; j < N; j++)
    {
      if (leituras[j] < leituras[i])
      {
        float temp = leituras[i];
        leituras[i] = leituras[j];
        leituras[j] = temp;
      }
    }
  }

  float mediana = leituras[N / 2];

  if (mediana > 200)
    return -1;

  return mediana;
}

// ======================================================
// CALCULO DA ALTURA DA AGUA NO BUEIRO
// ======================================================

int calcularAlturaAgua(float distanciaBueiro)
{
  if (distanciaBueiro == -1)
    return -1;

  int altura = round(ALTURA_TOTAL_BUEIRO - distanciaBueiro);

  if (altura < 0)
    altura = 0;

  if (altura > ALTURA_TOTAL_BUEIRO)
    altura = ALTURA_TOTAL_BUEIRO;

  return altura;
}

// ======================================================
// CONEXAO WI-FI
// ======================================================

void iniciarWiFi()
{
  WiFi.mode(WIFI_STA);
  WiFi.setAutoReconnect(true);

  Serial.println();
  Serial.println("================================");
  Serial.println("CONECTANDO AO WI-FI");
  Serial.println("================================");

  WiFi.begin(WIFI_NOME, WIFI_SENHA);

  ultimaTentativaWiFi = millis();
}

void verificarWiFi()
{
  bool conectado = WiFi.status() == WL_CONNECTED;

  if (conectado && !wifiEstavaConectado)
  {
    Serial.println();
    Serial.println("Wi-Fi conectado com sucesso!");

    Serial.print("IP do ESP32: ");
    Serial.println(WiFi.localIP());

    Serial.println("================================");
  }

  wifiEstavaConectado = conectado;

  if (!conectado &&
      millis() - ultimaTentativaWiFi >= INTERVALO_RECONEXAO_WIFI)
  {
    Serial.println();
    Serial.println("Wi-Fi desconectado. Tentando reconectar...");

    WiFi.disconnect();
    WiFi.begin(WIFI_NOME, WIFI_SENHA);

    ultimaTentativaWiFi = millis();
  }
}

// ======================================================
// ENVIO DOS DADOS AO BACKEND JAVA
// ======================================================

void enviarParaJava(int alturaAgua, bool alagadoRua)
{
  if (alturaAgua == -1)
  {
    Serial.println();
    Serial.println("Envio cancelado: sensor do bueiro sem leitura.");
    return;
  }

  if (WiFi.status() != WL_CONNECTED)
  {
    Serial.println();
    Serial.println("Envio cancelado: ESP32 sem conexao Wi-Fi.");
    return;
  }

  WiFiClient client;
  HTTPClient http;

  http.begin(client, URL_SERVIDOR);
  http.addHeader("Content-Type", "application/json");

  String json = "{";
  json += "\"altura\":";
  json += String(alturaAgua);
  json += ",";
  json += "\"alagadoRua\":";
  json += alagadoRua ? "true" : "false";
  json += "}";

  int codigoResposta = http.POST(json);

  Serial.println();
  Serial.println("================================");
  Serial.println("ENVIO PARA O BACKEND JAVA");
  Serial.println("================================");

  Serial.print("URL: ");
  Serial.println(URL_SERVIDOR);

  Serial.print("JSON enviado: ");
  Serial.println(json);

  Serial.print("Codigo HTTP: ");
  Serial.println(codigoResposta);

  if (codigoResposta > 0)
  {
    String resposta = http.getString();

    Serial.println("Resposta do servidor:");
    Serial.println(resposta);
  }
  else
  {
    Serial.println("Erro ao enviar dados para o backend.");
  }

  Serial.println("================================");

  http.end();
}

// ======================================================
// CONFIGURACAO INICIAL
// ======================================================

void setup()
{
  Serial.begin(115200);

  pinMode(TRIG_BUEIRO, OUTPUT);
  pinMode(ECHO_BUEIRO, INPUT);

  pinMode(TRIG_RUA, OUTPUT);
  pinMode(ECHO_RUA, INPUT);

  pinMode(LED_VERDE, OUTPUT);
  pinMode(LED_AMARELO, OUTPUT);
  pinMode(LED_VERMELHO, OUTPUT);

  pinMode(BUZZER, OUTPUT);

  digitalWrite(LED_VERDE, LOW);
  digitalWrite(LED_AMARELO, LOW);
  digitalWrite(LED_VERMELHO, LOW);
  digitalWrite(BUZZER, LOW);

  iniciarWiFi();

  Serial.println();
  Serial.println("Sistema de alerta de alagamento iniciado.");
}

// ======================================================
// LOOP PRINCIPAL
// ======================================================

void loop()
{
  verificarWiFi();

  float distBueiro = lerSensor(TRIG_BUEIRO, ECHO_BUEIRO);

  // Pequena pausa para reduzir interferencia entre sensores
  delay(50);

  float distRua = lerSensor(TRIG_RUA, ECHO_RUA);

  int alturaAgua = calcularAlturaAgua(distBueiro);

  bool alagadoRua =
    distRua != -1 &&
    distRua < LIMITE_DISTANCIA_RUA;

  // ====================================================
  // VERIFICA FALHA TOTAL DOS SENSORES
  // ====================================================

  if (distBueiro == -1 && distRua == -1)
    contadorFalhas++;
  else
    contadorFalhas = 0;

  if (contadorFalhas >= 5)
  {
    Serial.println();
    Serial.println("!!! FALHA DOS SENSORES !!!");
    Serial.println("VERIFICAR CABOS E ALIMENTACAO");
    Serial.println();

    digitalWrite(LED_VERDE, LOW);
    digitalWrite(LED_AMARELO, LOW);

    digitalWrite(LED_VERMELHO, HIGH);
    digitalWrite(BUZZER, HIGH);

    delay(200);

    digitalWrite(LED_VERMELHO, LOW);
    digitalWrite(BUZZER, LOW);

    delay(200);

    return;
  }

  // ====================================================
  // LIMPA AS SAIDAS ANTES DE DEFINIR O NOVO ESTADO
  // ====================================================

  digitalWrite(LED_VERDE, LOW);
  digitalWrite(LED_AMARELO, LOW);
  digitalWrite(LED_VERMELHO, LOW);
  digitalWrite(BUZZER, LOW);

  // ====================================================
  // CONTROLE DOS ALERTAS
  // Prioridade: rua alagada > falha parcial > bueiro
  // ====================================================

  if (alagadoRua)
  {
    digitalWrite(LED_VERMELHO, HIGH);
    digitalWrite(BUZZER, HIGH);
  }
  else if (distBueiro == -1 || distRua == -1)
  {
    // Pelo menos um dos sensores esta sem leitura
    digitalWrite(LED_AMARELO, HIGH);
  }
  else if (distBueiro > LIMITE_DISTANCIA_BUEIRO)
  {
    // Menos de aproximadamente 8 cm de agua no bueiro
    digitalWrite(LED_VERDE, HIGH);
  }
  else
  {
    // Agua subindo dentro do bueiro
    digitalWrite(LED_AMARELO, HIGH);
  }

  // ====================================================
  // EXIBICAO NO MONITOR SERIAL
  // ====================================================

  Serial.println();
  Serial.println("================================");

  Serial.print("BUEIRO - DISTANCIA: ");

  if (distBueiro == -1)
  {
    Serial.println("ERRO");
  }
  else
  {
    Serial.print(distBueiro, 1);
    Serial.println(" cm");
  }

  Serial.print("ALTURA DA AGUA: ");

  if (alturaAgua == -1)
  {
    Serial.println("SEM LEITURA");
  }
  else
  {
    Serial.print(alturaAgua);
    Serial.println(" cm");
  }

  Serial.print("RUA - DISTANCIA: ");

  if (distRua == -1)
  {
    Serial.println("ERRO");
  }
  else
  {
    Serial.print(distRua, 1);
    Serial.println(" cm");
  }

  Serial.print("STATUS BUEIRO: ");

  if (distBueiro == -1)
  {
    Serial.println("ERRO");
  }
  else if (distBueiro > LIMITE_DISTANCIA_BUEIRO)
  {
    Serial.println("VERDE");
  }
  else
  {
    Serial.println("AMARELO");
  }

  Serial.print("STATUS RUA: ");

  if (distRua == -1)
  {
    Serial.println("ERRO");
  }
  else if (alagadoRua)
  {
    Serial.println("VERMELHO - ALAGAMENTO DETECTADO");
  }
  else
  {
    Serial.println("NORMAL");
  }

  // ====================================================
  // ENVIO PERIODICO AO BACKEND
  // ====================================================

  if (millis() - ultimoEnvioJava >= INTERVALO_ENVIO_JAVA)
  {
    enviarParaJava(alturaAgua, alagadoRua);
    ultimoEnvioJava = millis();
  }

  delay(100);
}