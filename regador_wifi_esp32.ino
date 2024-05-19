#include <WiFi.h>
#include <HTTPClient.h>

const char* ssid = "ELIAS VIVO 2.4G";       // SSID da sua rede Wi-Fi
const char* password = "lara1305";  // Senha da sua rede Wi-Fi
const char* server_ip = "44.206.253.220 ";
const int server_port = 4000;

const int sensor_pin = 35; // Pino analógico onde o sensor de umidade do solo está conectado (GPIO 35 no ESP32)
const int rele_pin = 2; // Pino digital onde o relé está conectado (GPIO 2 no ESP32)

const int limite_umidade_alta = 601;  // Limite inferior para umidade alta (valor do ADC)
const int limite_umidade_baixa = 1023; // Limite superior para umidade baixa (valor do ADC)

float umidade_solo = 0.0; // Valor inicial da umidade do solo

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);

  Serial.println("Conectando ao WiFi...");

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Conectando...");
  }

  Serial.println("Conectado ao WiFi com sucesso!");

  pinMode(rele_pin, OUTPUT); // Configurar o pino do relé como saída
  digitalWrite(rele_pin, LOW); // Inicialmente desliga o relé
}

void loop() {
  if (WiFi.status() == WL_CONNECTED) {
    // Ler a umidade do solo
    umidade_solo = lerUmidadeSolo();

    // Enviar leitura para o servidor
    enviarParaServidor(umidade_solo);

    // Verificar umidade do solo e acionar relé se necessário
    controlarRele(umidade_solo);

    delay(5000); // Aguardar 5 segundos antes de enviar outra leitura
  }
}

float lerUmidadeSolo() {
  int valor_analogico = analogRead(sensor_pin);
  // Faça o mapeamento do valor lido para a faixa de umidade do solo
  float umidade = map(valor_analogico, limite_umidade_alta, limite_umidade_baixa, 100, 0);
  return umidade;
}

void enviarParaServidor(float umidade) {
  HTTPClient http;

  // Construir a URL
  String url = "http://" + String(server_ip) + ":" + String(server_port) + "/umidade";

  Serial.print("Enviando umidade do solo: ");
  Serial.println(umidade);

  http.begin(url); // Iniciar conexão HTTP
  http.addHeader("Content-Type", "application/x-www-form-urlencoded"); // Adicionar cabeçalho para requisição POST

  // Enviar solicitação POST com dados no corpo
  String postData = "dados=" + String(umidade);
  int httpCode = http.POST(postData);

  if (httpCode > 0) {
    // Verificar código de resposta
    if (httpCode == HTTP_CODE_OK) {
      Serial.println("Umidade do solo enviada com sucesso!");
    } else {
      Serial.print("Falha ao enviar umidade do solo. Código de erro HTTP: ");
      Serial.println(httpCode);
    }
  } else {
    Serial.print("Falha ao conectar ao servidor. Código de erro: ");
    Serial.println(http.errorToString(httpCode).c_str());
  }

  http.end(); // Fechar conexão HTTP
}

void controlarRele(float umidade) {
  // Considera-se umidade baixa quando a leitura do ADC está entre 601 e 1023
  if (analogRead(sensor_pin) >= limite_umidade_alta && analogRead(sensor_pin) <= limite_umidade_baixa) {
    Serial.println("Umidade baixa, acionando relé...");
    digitalWrite(rele_pin, HIGH); // Liga o relé
  } else {
    Serial.println("Umidade normal, desligando relé...");
    digitalWrite(rele_pin, LOW); // Desliga o relé
  }
}
