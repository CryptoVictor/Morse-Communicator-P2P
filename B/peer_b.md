# Documentação Peer B

## Bibliotecas utilizadas

```cpp
#include <Wire.h>                  // Comunicação I2C (necessária para o display LCD)
#include <LiquidCrystal_I2C.h>     // Controle do display LCD 16x2 via I2C
#include <WiFi.h>                  // Funções de rede Wi-Fi (modo estação)
#include <esp_now.h>               // Protocolo ESP-NOW para comunicação direta entre ESPs
#include "esp_system.h"            // Funções de sistema do ESP
#include "esp_wifi.h"              // Controle e leitura do MAC address
```

Essas bibliotecas permitem que o ESP32:

- Controle o display LCD;
- Envie e receba dados via ESP-NOW sem internet;
- Acesse e mostre o MAC address do dispositivo.

## Parâmetros

```cpp
#define LED_PIN 25
#define BUZZER_PIN 26
#define BUTTON_PIN 27
#define POT_PIN 34

#define SENHA 100
#define TOLERANCIA 3
```

**LED_PIN / BUZZER_PIN:** LED e buzzer usados para sinalização sonora/visual.
**BUTTON_PIN:** Botão para confirmar ações.
**POT_PIN:** Potenciômetro usado para digitar a senha.
**SENHA:** Valor de referência da senha.
**TOLERANCIA:** Margem de erro aceitável no valor da senha.

## Estados e variáveis globais

```cpp
bool senhaLiberada = false;
bool macRecebido = false;
bool aguardandoEnvio = false;
bool confirmacaoEnvio = false;

LiquidCrystal_I2C lcd(0x27, 16, 2);
String mensagem = "";
uint8_t macDest[6]; 
String meuMAC = "";
```

Essas variáveis controlam o fluxo de execução:

- **senhaLiberada:** indica se o usuário passou da etapa de autenticação.
- **macRecebido:** define se o endereço MAC do destino já foi cadastrado.
- **aguardandoEnvio e confirmacaoEnvio:** controlam o envio da mensagem.
- **meuMAC:** guarda o endereço MAC local do ESP.

## Funções auxiliares

**1. piscar BuzzerLed(int vezes, int duracao)**

Ativa o LED e buzzer em conjunto, repetindo o número de vezes definido.
Usado para confirmar ações, erros e recepção de mensagens.

**2. lcdPrintClean(int linha, String msg)**

Imprime uma mensagem e preenche o restante da linha com espaços,
evitando caracteres “fantasmas” de mensagens anteriores.

**3. charToMorse(char c)**

Converte um caractere (A–Z, 0–9) para sua representação em Morse.
Espaços são representados por /.

**4. morseToChar(String code)**

Faz o inverso: converte uma sequência de pontos e traços em uma letra.

**5. piscarMorse (String code)**

Toca um código Morse no LED + buzzer, com:
. → pulso curto (200ms)
- → pulso longo (600ms)
Espaço → pausa (400ms)

**6. mensagemParaMorse(String morse)**

Transforma uma mensagem inteira em Morse, separando cada letra com espaço.

**7. morseParaMensagem(String morse)**

Lê uma string em Morse e reconstrói o texto original.

**8. onDataSent(...)**

Executada após uma tentativa de envio via ESP-NOW.
Mostra no Serial se foi “Sucesso” ou “Falha”
e pisca o LED/buzzer uma vez.

**9. onDataRecv(...)**

Executada quando o ESP recebe uma mensagem.
Etapas:

- Obtém o MAC do remetente;
- Lê os bytes da mensagem (em Morse);
- Decodifica em texto (morseParaMensagem);
- Toca o som/LED em Morse (piscarMorse);
- Mostra a mensagem no LCD.

**10. lerPotenciometro()**

Faz uma média de 10 leituras do pino analógico para suavizar ruídos.
Usado na etapa de senha.

## Setup

Configurações iniciais:

1. Define pinos como entrada ou saída;
2. Inicia LCD e Serial;
3. Ativa Wi-Fi em modo estação (WIFI_STA);
4. Obtém e exibe o MAC address local;
5. Inicializa o ESP-NOW e registra callbacks;
6. Exibe instruções iniciais no display.

## Loop

**1. Digitação e verificação de senha**

```cpp
if(!senhaLiberada){ ... }
```

- Mostra o valor lido do potenciômetro;
- Se o botão for pressionado, compara com SENHA;
- Se estiver dentro da TOLERANCIA, libera o acesso.

**2. Inserção do MAC do destinatário**

```cpp
if(senhaLiberada && !macRecebido){ ... }
```

- Aguarda o MAC ser enviado via Serial (ex: AA:BB:CC:DD:EE:FF);
- Converte a string para bytes;
- Adiciona o peer ao ESP-NOW (esp_now_add_peer);
- Mostra “MAC recebido!” no LCD.

**3. Digitação e confirmação da mensagem**

```cpp
if(macRecebido && !aguardandoEnvio){ ... }
```

- Lê a mensagem via Serial;
- Exibe no LCD para revisão;
- Aguarda confirmação com o botão.

**4. Envio via ESP-NOW (em Morse)**

```cpp
if(aguardandoEnvio && confirmacaoEnvio && digitalRead(BUTTON_PIN)==LOW)
```

- Converte a mensagem em Morse (mensagemParaMorse);
- Envia via ESP-NOW (esp_now_send);
- Mostra “Mensagem Enviada!” no LCD;
- Pisca LED/buzzer como confirmação;
- Retorna à tela principal com o MAC do dispositivo.

## Fluxo de funcionamento

**1. Entrada de senha via potenciômetro**

Ação: Gira o potenciômetro e pressiona o botão

**2. Leitura do MAC do destinatário**

Ação: Digita no Serial Monitor

**3. Entrada da mensagem**

Ação: Digita no Serial Monitor

**4. Envio e codificação Morse**

Ação: Pressiona o botão novamente

**5. Recepção**

Ação: Mostra texto e toca o Morse no buzzer/LED

## Resumo

Funcionamento Geral e Resumo do Funcionamento do PEER B
