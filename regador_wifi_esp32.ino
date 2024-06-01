#include <WiFi.h>
#include <HTTPClient.h>

const char* ssid = "ELIAS VIVO 2.4G";       // SSID da sua rede Wi-Fi
const char* password = "lara1305";          // Senha da sua rede Wi-Fi
const char* server_ip = "44.206.253.220";
const int server_port = 4000;

const int sensor_pin = 35;  // Pino analógico onde o sensor de umidade do solo está conectado (GPIO 35 no ESP32)
const int rele_pin = 2;     // Pino digital onde o relé está conectado (GPIO 2 no ESP32)

const int limite_umidade_alta = 3001;  // Limite inferior para umidade alta (valor do ADC)

int valor_analogico = 0; // Valor inicial da leitura analógica

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
  pinMode(sensor_pin,INPUT);
  
  digitalWrite(rele_pin, LOW); // Inicialmente desliga o relé
}

void loop() {
  if (WiFi.status() == WL_CONNECTED) {
    // Ler a umidade do solo
    valor_analogico = lerUmidadeSolo();

    // Verificar umidade do solo e acionar relé se necessário
    controlarRele(valor_analogico);

    // Enviar leitura para o servidor
    enviarParaServidor(valor_analogico);

    delay(2000); // Aguardar 2 segundos antes de enviar outra leitura
  }
}

int lerUmidadeSolo() {
  int valor_analogico = analogRead(sensor_pin);
  Serial.print("Valor analógico lido: ");
  Serial.println(valor_analogico);
  
  return valor_analogico;
}

void enviarParaServidor(int valor_analogico) {
  HTTPClient http;

  // Construir a URL
  String url = "http://" + String(server_ip) + ":" + String(server_port) + "/umidade";

  Serial.print("Enviando valor analógico de umidade do solo: ");
  Serial.println(valor_analogico);

  http.begin(url); // Iniciar conexão HTTP
  http.addHeader("Content-Type", "application/x-www-form-urlencoded"); // Adicionar cabeçalho para requisição POST

  // Enviar solicitação POST com dados no corpo
  String postData = "dados=" + String(valor_analogico);
  int httpCode = http.POST(postData);

  if (httpCode > 0) {
    // Verificar código de resposta
    if (httpCode == HTTP_CODE_OK) {
      Serial.println("Valor analógico de umidade do solo enviado com sucesso!");
    } else {
      Serial.print("Falha ao enviar valor analógico de umidade do solo. Código de erro HTTP: ");
      Serial.println(httpCode);
    }
  } else {
    Serial.print("Falha ao conectar ao servidor. Código de erro: ");
    Serial.println(http.errorToString(httpCode).c_str());
  }

  http.end(); // Fechar conexão HTTP
}

void controlarRele(int valor_analogico) {
  // Considera-se umidade baixa quando a leitura do ADC está entre limite_umidade_alta e limite_umidade_baixa
  if (valor_analogico >= limite_umidade_alta) {
    Serial.println("Umidade baixa, acionando relé...");
    digitalWrite(rele_pin, HIGH); // Liga o relé
  } else {
    Serial.println("Umidade normal, desligando relé...");
    digitalWrite(rele_pin, LOW); // Desliga o relé
  }
}
