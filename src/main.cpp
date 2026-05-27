#include <Arduino.h>
#include <Nextion.h>
#include <MqttManager.h>
#include <DebugManager.h>
#include <WiFiManager.h>

// =========================================================
// CONFIGURAÇÃO DA COMUNICAÇÃO COM O NEXTION
// =========================================================

// Velocidade padrão de comunicação serial do Nextion
const uint32_t BAUD_NEXTION = 9600;

// RX do ESP32-S3 ligado ao TX do Nextion
const int8_t PINO_RX_NEXTION = 18;

// TX do ESP32-S3 ligado ao RX do Nextion
const int8_t PINO_TX_NEXTION = 17;

// =========================================================
// COMPONENTES DO NEXTION
// =========================================================

// A estrutura é:
// Classe nomeDoObjeto(pageId, componentId, "nomeNoNextion");

// Evite chamar a variável de j0, porque existe uma função matemática j0()
// na biblioteca padrão do C/C++.
NexProgressBar barraJ0(0, 2, "j0");

// Componentes que recebem toque
NexButton botaoB0(0, 3, "b0");
NexSlider sliderH0(0, 4, "h0");
NexDSButton botaoDualBt0(0, 5, "bt0");

// Textos usados para mostrar o estado dos componentes
NexText textoT1(0, 6, "t1");
NexText textoT2(0, 7, "t2");
NexText textoT3(0, 8, "t3");
NexText textoT4(0, 9, "t4");

// =========================================================
// VARIÁVEIS DE CONTROLE DA APLICAÇÃO
// =========================================================

uint32_t valorBarra = 0;
uint32_t contadorBotao = 0;
uint32_t valorSlider = 0;
uint32_t estadoBotaoDual = 0;
uint8_t tema = 0;

// Buffer usado para montar textos antes de enviar ao display
char texto[50];

// =========================================================
// PROTÓTIPOS DAS FUNÇÕES
// =========================================================

void configurarNextion();
void configurarTelaInicial();
void configurarEventosNextion();

void botaoBack();
void sliderH0Soltou();
void botaoDualBt0Soltou();

void atualizarTextoBarra();
void atualizarTextoBotao();
void atualizarTextoSlider();
void atualizarTextoBotaoDual();

// =========================================================
// SETUP
// =========================================================

void setup()
{
    Serial.begin(115200);
    delay(1000);

    Serial.println();
    Serial.println("Iniciando exemplo com ESP32-S3 e Nextion...");

    conectarWiFi();
    conectarMQTT();

    configurarNextion();
    configurarTelaInicial();
    configurarEventosNextion();

    Serial.println("Sistema iniciado.");
}

// =========================================================
// LOOP
// =========================================================

void loop()
{
    // Mantém o ESP32 ouvindo os eventtaoDualBt0Solos enviados pelo Nextion.
    // Quando o usuário toca em algum componente registrado com nexListen(),
    // a biblioteca identifica o componente e executa a função associada.
    nexLoop();
}

// =========================================================
// CONFIGURAÇÃO DO NEXTION
// =========================================================

void configurarNextion()
{
    // Inicializa a comunicação serial com o display Nextion.
    // Parâmetros:
    // 1. baud rate
    // 2. pino RX do ESP32-S3
    // 3. pino TX do ESP32-S3
    bool nextionOk = nexInit(BAUD_NEXTION, PINO_RX_NEXTION, PINO_TX_NEXTION);

    if (!nextionOk)
    {
        Serial.println("Aviso: o Nextion nao confirmou a inicializacao.");
    }
    else
    {
        Serial.println("Nextion inicializado.");
    }
}

// =========================================================
// CONFIGURAÇÃO INICIAL DA TELA
// =========================================================

void configurarTelaInicial()
{
    // Garante que o display esteja na página principal.
    // O nome "page0" precisa ser igual ao nome da página no Nextion Editor.
    sendCommand("page inicial");

    delay(500);

    // Define os valores iniciais da aplicação.
    valorBarra = 0;
    contadorBotao = 0;
    valorSlider = 0;
    estadoBotaoDual = 0;

    // Atualiza os componentes visuais do display.
    barraJ0.setValue(valorBarra);
    sliderH0.setValue(valorSlider);
    botaoDualBt0.setValue(estadoBotaoDual);

    // Atualiza os textos iniciais.
    textoT1.setText("j0 = 0%");
    textoT2.setText("b0 aguardando");
    textoT3.setText("h0 = 0");
    textoT4.setText("bt0 = Desligado");
}

// =========================================================
// CONFIGURAÇÃO DOS EVENTOS DO NEXTION
// =========================================================

void configurarEventosNextion()
{
    // attachPop() executa a função quando o componente é solto.
    // No Nextion Editor, use "Send Component ID" em Touch Release Event.

    botaoB0.attachPop(botaoBack);
    sliderH0.attachPop(sliderH0Soltou);
    botaoDualBt0.attachPop(botaoDualBt0Soltou);
    

    // Limpa a lista interna de componentes monitorados.
    nexClearListenList();

    // Registra quais componentes devem ser ouvidos pelo ESP32.
    nexListen(botaoB0);
    nexListen(sliderH0);
    nexListen(botaoDualBt0);
}

// =========================================================
// EVENTO DO BOTÃO b0
// =========================================================

void botaoBack()
{
    Serial.println("Botao Back foi pressionado!");
    if(tema == 0)
    sendCommand("page inicial0");
    else if (tema == 1)
    sendCommand("page inicial1");
    else if (tema == 2)
    sendCommand("page inicial2");
}

// =========================================================
// EVENTO DO SLIDER h0
// =========================================================

void sliderH0Soltou()
{
    Serial.println("Slider h0 solto.");

    // Lê o valor atual do slider no Nextion.
    // A biblioteca grava o// Componentes que recebem toque valor dentro da variável valorSlider.
    sliderH0.getValue(&valorSlider);

    atualizarTextoSlider();

    Serial.print("Valor do slider h0 = ");
    Serial.println(valorSlider);
}

// =========================================================
// EVENTO DO DUAL STATE BUTTON bt0
// =========================================================

void botaoDualBt0Soltou()
{
    Serial.println("Botao dual bt0 solto.");

    // Lê o valor atual do Dual State Button.
    // Normalmente:
    // 0 = desligado
    // 1 = ligado
    botaoDualBt0.getValue(&estadoBotaoDual);

    atualizarTextoBotaoDual();

    Serial.print("Estado do bt0 = ");
    Serial.println(estadoBotaoDual);
}

// =========================================================
// FUNÇÕES DE ATUALIZAÇÃO DOS TEXTOS
// =========================================================

void atualizarTextoBarra()
{
    // snprintf monta o texto de forma segura dentro do vetor texto.
    // Exemplo final: j0 = 30%
    snprintf(texto, sizeof(texto), "j0 = %lu%%", valorBarra);

    textoT1.setText(texto);
}

void atualizarTextoBotao()
{
    // Exemplo final: b0 clicado 3x
    snprintf(texto, sizeof(texto), "b0 clicado %lux", contadorBotao);

    textoT2.setText(texto);
}

void atualizarTextoSlider()
{
    // Exemplo final: h0taoDualBt0Sol = 75
    snprintf(texto, sizeof(texto), "h0 = %lu", valorSlider);

    textoT3.setText(texto);
}

void atualizarTextoBotaoDual()
{
    if (estadoBotaoDual == 1)
    {
        textoT4.setText("bt0 = Ligado");
    }
    else
    {
        textoT4.setText("bt0 = Desligado");
    }
}