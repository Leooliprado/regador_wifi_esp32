#include <WiFi.h>
#include <HTTPClient.h>

const char* ssid = "ELIAS VIVO 2.4G";       // SSID da sua rede Wi-Fi
const char* password = "lara1305";          // Senha da sua rede Wi-Fi
const char* server_ip = "44.206.253.220";
const int server_port = 4000;

const int sensor_pin = 35;  // Pino analógico onde o sensor de umidade do solo está conectado (GPIO 35 no ESP32)
const int rele_pin = 4;     // Pino digital onde o relé está conectado (GPIO 18 no ESP32)
const int estado_pin = 2; // ve o estado da conequisao e o estato da bomba

const int limite_umidade_alta = 2000;  // Limite inferior para umidade alta (valor do ADC)

int valor_analogico = 0; // Valor inicial da leitura analógica

void setup() {
 

  Serial.begin(115200);
  WiFi.begin(ssid, password);

  Serial.println("Conectando ao WiFi...");

  while (WiFi.status() != WL_CONNECTED) {
    Serial.println("Conectando...");
    
    digitalWrite(estado_pin, HIGH); 
    delay(500);
    digitalWrite(estado_pin, LOW); 
    delay(500);
  }

  Serial.println("Conectado ao WiFi com sucesso!");

  pinMode(rele_pin, OUTPUT); 
  digitalWrite(rele_pin, LOW);
  pinMode(estado_pin, OUTPUT);
  pinMode(sensor_pin,INPUT);
}

void loop() {
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("Conectado ao WiFi com sucesso!");
    // Ler a umidade do solo
    valor_analogico = lerUmidadeSolo();

    // Enviar leitura para o servidor
    enviarParaServidor(valor_analogico);

    // Verificar umidade do solo e acionar relé se necessário
    controlarRele(valor_analogico);

    delay(2000); // Aguardar 2 segundos antes de enviar outra leitura
  }else{
    Serial.println("Erro de conexão!! Talvez a internet caiu?");

    WiFi.begin(ssid, password);

    Serial.println("Conectando ao WiFi...");

    digitalWrite(estado_pin, HIGH); //led para ver status de conexão
    delay(500);
    digitalWrite(estado_pin, LOW); //led para ver status de conexão
    delay(500);
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

    digitalWrite(estado_pin, HIGH);  //led de para ver o status da bomba d'água

    delay(6000);
  } else {
    Serial.println("Umidade normal, desligando relé...");
    digitalWrite(rele_pin, LOW); // Desliga o relé

    digitalWrite(estado_pin, LOW); //led de para ver o status da bomba d'água
  }
}
