#include <Arduino.h>

#include <PubSubClient.h>
#include <WiFiClient.h>
#include <WiFiClientSecure.h>

#include "secrets.h"
#include "WiFiManager.h"
#include "MqttManager.h"
#include "DebugManager.h"

//==============Instancias===========================
WiFiClient wifiCliente;
WiFiClientSecure wifiClienteSecure;
PubSubClient mqttClient;

CallbackMensagemMQTT callbackDaAplicacao = nullptr; // Chamando uma funcão que não aponta nada  -  nullptr - ponteiro nulo

void registrarCallbackMensagem(CallbackMensagemMQTT callback)
{
    callbackDaAplicacao = callback;

    if (callbackDaAplicacao != nullptr)  debugInfo("Callback da aplicação registrada com sucesso");

    else debugErro("Callback da aplicação não foi registrada");
}

const char *obterTopicoPublicacao(int indiceTopico)
{
    if (indiceTopico < 0 || indiceTopico >= TOTAL_TOPICOS_PUBLICAR)
    {
        debugErro("Indice inválido para topico de publicacao: " + String(indiceTopico));
        return "";
    }
    return TOPICOS_PUBLICAR[indiceTopico];
}

const char *obterTopicoRecebimento(int indiceTopico)
{
    if (indiceTopico < 0 || indiceTopico >= TOTAL_TOPICOS_RECEBER)
    {
        debugErro("Indice inválido para topico de receber: " + String(indiceTopico));
        return "";
    }
    return TOPICOS_RECEBER[indiceTopico];
}

void callbackInternoMQTT(char *topico, byte *payload, unsigned int tamanho)
{
    String mensagem = "";
    for (unsigned int i = 0; i < tamanho; i++)
    {
        mensagem += (char)payload[i];
    }

    debugInfo("==============================");
    debugInfo(" Mensagem MQTT recebida");
    debugInfo("==============================");
    debugInfo(" Topico: " + String(topico));
    debugInfo(" Mensagem: " + mensagem);

    if (callbackDaAplicacao != nullptr)
        callbackDaAplicacao(topico, mensagem);

    else
        debugErro("Mensagem recebida, mas nenhum callback da aplicacao foi registrado");
}

void configurarMQTT()
{
    debugInfo("=========================");
    debugInfo(" Configurando MQTT...");
    debugInfo("=========================");

    if (USAR_AWS_IOT)
    {
        debugInfo("Modo Selecionado: AWS IoT Core");
        debugInfo("Configurando certificados da AWS IoT Core");

        wifiClienteSecure.setCACert(AWS_CERT_CA);
        wifiClienteSecure.setCertificate(AWS_CERT_CRT);
        wifiClienteSecure.setPrivateKey(AWS_CERT_PRIVATE);

        mqttClient.setClient(wifiClienteSecure);
        mqttClient.setServer(AWS_IOT_ENDPOINT, AWS_IOT_PORT);

        debugInfo("Endpoint AWS IoT: " + String(AWS_IOT_ENDPOINT));
        debugInfo("Porta AWS IoT: " + String(AWS_IOT_PORT));
    }
    else if (MQTT_USAR_TLS)
    {
        debugInfo("Modo selecionado: MQTT com TLS");

        if(strlen(MQTT_CERTIFICADO_CA) > 100)
        {
            debugInfo("Certificado CA do broker MQTT configurado.");
            wifiClienteSecure.setCACert(MQTT_CERTIFICADO_CA);
        }
        else
        {
            debugErro("Certificado CA do MQTT não configurado. Usando setInsecure apenas para teste");
            wifiClienteSecure.setInsecure();
        }
       

        mqttClient.setClient(wifiClienteSecure);
        mqttClient.setServer(MQTT_BROKER, MQTT_PORTA);

        debugInfo("Broker MQTT: " + String(MQTT_BROKER));
        debugInfo("Porta MQTT: " + String(MQTT_PORTA));
        
        
    }
    else // conectar no broker sem certificacao
    {
        debugInfo("Modo selecionado: MQTT sem TLS");

        mqttClient.setClient(wifiCliente);
        mqttClient.setServer(MQTT_BROKER, MQTT_PORTA);

        debugInfo("Broker MQTT: " + String(MQTT_BROKER));
        debugInfo("Porta MQTT: " + String(MQTT_PORTA));
    }
    mqttClient.setCallback(callbackInternoMQTT);
    debugInfo("Callback interno do MQTT consfigurado.");
}

void conectarMQTT()
{
    if (!wifiEstaConectado())
    {
        debugErro("MQTT não pode conectar porque o WiFi está deconectado.");
        return;
    }

    debugInfo("===========================");
    debugInfo("Iniciando conexão MQTT...");
    debugInfo("===========================");

    int tentativasMQTT = 0;
    const int maxTentativasMQTT = 5;

    while (!mqttClient.connected() && tentativasMQTT < maxTentativasMQTT)
    {
        debugInfo("Tentando conectar ao broker MQTT. Tentaiva: " + String(tentativasMQTT));

        bool conectado = false;

        if (USAR_AWS_IOT)
        {
            conectado = mqttClient.connect(AWS_IOT_CLIENT_ID);
        }
        else
        {
            if (strlen(MQTT_USUARIO) > 0)
            {
                debugInfo("Conectando MQTT com usuário e senha");

                conectado = mqttClient.connect(MQTT_CLIENT_ID, MQTT_USUARIO, MQTT_SENHA);
            }

            else // sem usuario
            {
                debugInfo("Conectando MQTT sem usuário e sem senha.");

                conectado = mqttClient.connect(MQTT_CLIENT_ID);
            }
        }
        if(conectado)
        {
            debugInfo("MQTT conectado com sucesso.");

            int totalTopicosReceber = obterTotalTopicosRecebimento();
            debugInfo("Total de tópicos de recebimento para inscrição: " + String(totalTopicosReceber));

            int totalTopicosPublicar = obterTotalTopicosPublicacao();
            debugInfo("Total de tópicos de publicacao para inscrição: " + String(totalTopicosPublicar));

            for(int i = 0; i < totalTopicosReceber; i++)
            {
                const char* topico = obterTopicoRecebimento(i);

                bool inscrito = mqttClient.subscribe(topico);
                
                if(inscrito) debugInfo("Inscrito no tópico: " + String(topico));
                
                else debugErro("Falha ao se inscrever no tópico: " + String(topico));
            }
            
            for(int i = 0; i < totalTopicosPublicar; i++)   //Inscreve nos tópicos de Publicação
            {
                const char* topico = obterTopicoPublicacao(i);

                bool inscrito = mqttClient.subscribe(topico);
                
                if(inscrito) debugInfo("Inscrito no tópico: " + String(topico));
            
                else debugErro("Falha ao se inscrever no tópico: " + String(topico));

                
            }


            //TODO: publicar uma mensagem em um tópico informando que o esp foi conectado.
        }
        else
        {
            debugErro("Falha ao conectar no MQTT. Código de erro: " + String(mqttClient.state()));
            tentativasMQTT++;
            delay(2000);

        }
    }//fim do Whilhe()
    
    if(!mqttClient.connected())
    {
        debugErro("Não foi possível conectar ao broker MQTT após " + String(maxTentativasMQTT) + " tenttivas");
    }
}

void garantirMQTTConectado()
{
    if(!wifiEstaConectado())
    {
        debugErro("MQTT não será conectado porque o WiFi está desconectado.");
        return;
    }

    if(!mqttClient.connected())
    {
        debugErro("MQTT desconectado. Tentando reconectar...");
        conectarMQTT();
    }
}

void loopMQTT()
{
    mqttClient.loop();
}

void publicarMensagem(const char* topico, const char* mensagem)
{
    if(!mqttClient.connected())
    {
        debugErro("Não foi possivel publicar. MQTT desconectado.");
        return;
    }

    bool publicado = mqttClient.publish(topico, mensagem);

    if(publicado)
    {
        debugInfo("Mensagem publicada via MQTT.");
        debugInfo("Topico: " + String(topico));
        debugInfo("Mensgagem: " + String(mensagem));
    }
    else
    {
        debugErro("Falha ao publicar mensagem no tópico: " + String(topico));
    }
}

void publicarMensagemNoTopico(int indiceTopico, const char* mensagem)
{
    const char* topico = obterTopicoPublicacao(indiceTopico);

    if(strlen(topico) == 0)
    {
        debugErro("Não foi possivel publicar. Indice de topico invalido: " + String(indiceTopico));
        return;
    }
    publicarMensagem(topico, mensagem);
}

bool mqttEstaConectado()
{
    return mqttClient.connected();
}

int obterTotalTopicosRecebimento()
{
    return TOTAL_TOPICOS_RECEBER;
}

int obterTotalTopicosPublicacao()
{
    return TOTAL_TOPICOS_PUBLICAR;
}