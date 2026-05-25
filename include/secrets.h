//!secrets.h
#ifndef SECRETS_H
#define SECRETS_H

// COnfiguraçoes WiFI
extern const char* WIFI_SSID;
extern const char* WIFI_SENHA;


//=======================MQTT===========================
extern const char* MQTT_BROKER;  //endereço do broker
extern const int MQTT_PORTA;    //numero da porta

extern const char* MQTT_CLIENT_ID;  //ID do cliente - apenas um por esp

extern const char* MQTT_USUARIO;
extern const char* MQTT_SENHA;

extern const bool MQTT_USAR_TLS;       //indica se sera usado protocolo de segurança
extern const bool USAR_AWS_IOT;        //indica se o AWS sera usado 

extern const char MQTT_CERTIFICADO_CA[];

extern const char* TOPICOS_PUBLICAR[];

extern const int TOTAL_TOPICOS_PUBLICAR;   //Quantidade de topicos que podem ser publicados

extern const char* TOPICOS_RECEBER[];

extern const int TOTAL_TOPICOS_RECEBER;    //quantidade de topicos que pode receber


//==================DEBUG===========================

// 0 = sem mensagens
// 1 = apenas erros
// 2 = todas as mensagens
extern const int DEBUG_NIVEL_INICIAL;

// Pino usado para forcar todas as mensagens na inicializacão
extern const int PINO_HABILITA_DEBUG_COMPLETO;



extern const char AWS_CERT_CA[];

extern const char AWS_CERT_CRT[];

extern const char AWS_CERT_PRIVATE[];

extern const char* AWS_IOT_ENDPOINT;

extern const int AWS_IOT_PORT;

extern const char* AWS_IOT_CLIENT_ID;

#endif