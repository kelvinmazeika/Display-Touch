//! WiFiManager.cpp

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <WiFiClientSecure.h>
#include "WiFiManager.h"
#include "secrets.h"
#include "DebugManager.h"





bool wifiEstaConectado()
{
    return WiFi.status() == WL_CONNECTED;
}




//Função para conectar o ESP ao WiFi
//Execução é no setup()
void conectarWiFi()
{
  debugInfo("==========================");
  debugInfo("Iniciado Conexao WiFi...");
  debugInfo("==========================");
  
  // Configura o ESP32 como station, ou seja
  //ele vai se conectar a um roteador existente
  WiFi.mode(WIFI_STA); 

//Inicia a conexão com SSID e senha
WiFi.begin(WIFI_SSID, WIFI_SENHA);  

debugInfo("conectando");

int tentativasWiFi = 0;
const int maxTentativasWiFi = 30;

//aguarda a conexão por até 30 tentativas
while(WiFi.status() != WL_CONNECTED && tentativasWiFi < maxTentativasWiFi)
{
  delay(500);
  debugInfo(".");
  tentativasWiFi++;
}

Serial.println();

if(WiFi.status() == WL_CONNECTED) //Verifica se a conexão foi estabelecida
{
  debugInfo("WiFi conectado com sucesso!");
  debugInfoSemLinha("[INFO] Endereco IP: ");
  debugInfoSemLinha(WiFi.localIP().toString());
  debugInfoSemLinha("\n\r");
}
else
{
  debugErro("Falaha ao conectar no WiFi.");
  debugErro("Verifique SSID, senha e sinal de rede.");
}

}

//Função que garante a conexão com o WiFi
//Execução no loop()
void garantirWiFiConectado()
{
  if(WiFi.status() != WL_CONNECTED) //verifica se houve perda de conexão
  {
    debugErro("WiFi desconectado. Tentando reconectar...");
    conectarWiFi(); //chama a função conectarWiFi() para reconectar
  }
  if(WiFi.status() != WL_CONNECTED) //verifica se foi possível realizar a reconexão
  {
    debugErro("Nao foi possivel reconectar ao WiFi.");
  }
}