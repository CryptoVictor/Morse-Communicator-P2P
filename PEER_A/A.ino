#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <WiFi.h>
#include <esp_now.h>
#include "esp_system.h"
#include "esp_wifi.h"

#define LED_PIN 25
#define BUZZER_PIN 26
#define BUTTON_PIN 27
#define POT_PIN 34

#define SENHA 100
#define TOLERANCIA 3

// Estados do fluxo
bool senhaLiberada = false;
bool macRecebido = false;
bool aguardandoEnvio = false;
bool confirmacaoEnvio = false;

LiquidCrystal_I2C lcd(0x27, 16, 2);

String mensagem = "";
uint8_t macDest[6]; // MAC do destinatário
String meuMAC = "";

// ---------------- Funções auxiliares ----------------
void piscarBuzzerLed(int vezes, int duracao){
  for(int i=0;i<vezes;i++){
    digitalWrite(LED_PIN,HIGH);
    digitalWrite(BUZZER_PIN,HIGH);
    delay(duracao);
    digitalWrite(LED_PIN,LOW);
    digitalWrite(BUZZER_PIN,LOW);
    delay(duracao);
  }
}

void lcdPrintClean(int linha, String msg){
  lcd.setCursor(0,linha);
  lcd.print(msg);
  for(int i=msg.length();i<16;i++) lcd.print(" ");
}

// ---------------- Morse ----------------
String charToMorse(char c){
  c = toupper(c);
  String morseTable[36] = {".-","-...","-.-.","-..",".","..-.","--.","....","..",".---",
                           "-.-",".-..","--","-.","---",".--.","--.-",".-.","...","-",
                           "..-","...-",".--","-..-","-.--","--..",
                           "-----",".----","..---","...--","....-",".....","-....","--...",
                           "---..","----."};
  char asciiTable[36] = {'A','B','C','D','E','F','G','H','I','J',
                         'K','L','M','N','O','P','Q','R','S','T',
                         'U','V','W','X','Y','Z',
                         '0','1','2','3','4','5','6','7','8','9'};
  for(int i=0;i<36;i++) if(asciiTable[i]==c) return morseTable[i];
  if(c==' ') return "/"; // espaço entre palavras
  return "";
}

char morseToChar(String code){
  String morseTable[36] = {".-","-...","-.-.","-..",".","..-.","--.","....","..",".---",
                           "-.-",".-..","--","-.","---",".--.","--.-",".-.","...","-",
                           "..-","...-",".--","-..-","-.--","--..",
                           "-----",".----","..---","...--","....-",".....","-....","--...",
                           "---..","----."};
  char asciiTable[36] = {'A','B','C','D','E','F','G','H','I','J',
                         'K','L','M','N','O','P','Q','R','S','T',
                         'U','V','W','X','Y','Z',
                         '0','1','2','3','4','5','6','7','8','9'};
  for(int i=0;i<36;i++) if(morseTable[i]==code) return asciiTable[i];
  if(code=="/") return ' ';
  return '?';
}

void piscarMorse(String code){
  for(int i=0;i<code.length();i++){
    if(code[i]=='.'){
      digitalWrite(LED_PIN,HIGH);
      digitalWrite(BUZZER_PIN,HIGH);
      delay(200);
      digitalWrite(LED_PIN,LOW);
      digitalWrite(BUZZER_PIN,LOW);
      delay(200);
    }else if(code[i]=='-'){
      digitalWrite(LED_PIN,HIGH);
      digitalWrite(BUZZER_PIN,HIGH);
      delay(600);
      digitalWrite(LED_PIN,LOW);
      digitalWrite(BUZZER_PIN,LOW);
      delay(200);
    }else if(code[i]==' '){
      delay(400);
    }
  }
}

String mensagemParaMorse(String msg){
  String morseMsg="";
  for(int i=0;i<msg.length();i++){
    morseMsg += charToMorse(msg[i]);
    if(i<msg.length()-1) morseMsg += " ";
  }
  return morseMsg;
}

String morseParaMensagem(String morse){
  String msg="";
  String letra="";
  for(int i=0;i<morse.length();i++){
    if(morse[i]!=' '){
      letra += morse[i];
    }else{
      msg += morseToChar(letra);
      letra="";
    }
  }
  if(letra!="") msg += morseToChar(letra);
  return msg;
}

// ---------------- Callbacks ESP-NOW ----------------
void onDataSent(const wifi_tx_info_t *tx_info, esp_now_send_status_t status){
  Serial.print("Status do envio: ");
  Serial.println(status==ESP_NOW_SEND_SUCCESS?"Sucesso":"Falha");
  piscarBuzzerLed(1,150); // confirma envio
}

void onDataRecv(const esp_now_recv_info_t *info,const uint8_t *incomingData,int len){
  char macStr[18];
  snprintf(macStr,sizeof(macStr),"%02X:%02X:%02X:%02X:%02X:%02X",
           info->src_addr[0],info->src_addr[1],info->src_addr[2],
           info->src_addr[3],info->src_addr[4],info->src_addr[5]);

  String morseMsg="";
  for(int i=0;i<len;i++) morseMsg += (char)incomingData[i];

  String msg = morseParaMensagem(morseMsg); // decodifica

  Serial.print("Recebido de "); Serial.println(macStr);
  Serial.print("Mensagem: "); Serial.println(msg);

  piscarMorse(morseMsg); // toca Morse

  lcd.clear();
  lcdPrintClean(0,msg.length()<=16?msg:msg.substring(0,16));
  if(msg.length()>16) lcdPrintClean(1,msg.substring(16,32));
  delay(4000);

  lcd.clear();
  lcdPrintClean(0,meuMAC.substring(0,8));
  lcdPrintClean(1,meuMAC.substring(8,17));
}

// ---------------- Funções auxiliares ----------------
int lerPotenciometro(){
  int soma=0;
  const int leituras=10;
  for(int i=0;i<leituras;i++){
    soma += analogRead(POT_PIN);
    delay(5);
  }
  return soma/leituras;
}

// ---------------- Setup ----------------
void setup(){
  pinMode(LED_PIN,OUTPUT);
  pinMode(BUZZER_PIN,OUTPUT);
  pinMode(BUTTON_PIN,INPUT_PULLUP);
  pinMode(POT_PIN,INPUT);

  Wire.begin(32,33);
  lcd.init();
  lcd.backlight();
  Serial.begin(115200);

  // ---------- WiFi e MAC ----------
  WiFi.mode(WIFI_STA);
  delay(100);
  uint8_t mac[6];
  esp_wifi_get_mac(WIFI_IF_STA,mac);
  meuMAC="";
  for(int i=0;i<6;i++){
    if(mac[i]<16) meuMAC += "0";
    meuMAC += String(mac[i],HEX);
    if(i<5) meuMAC += ":";
  }
  meuMAC.toUpperCase();
  Serial.print("Meu MAC: "); Serial.println(meuMAC);

  // ---------- ESP-NOW ----------
  if(esp_now_init()!=ESP_OK){ Serial.println("Erro inicializando ESP-NOW"); return; }
  esp_now_register_send_cb(onDataSent);
  esp_now_register_recv_cb(onDataRecv);

  // ---------- Tela inicial ----------
  lcd.clear();
  lcdPrintClean(0,"Digite a senha");
  lcdPrintClean(1,"e aperte botao");
}

// ---------------- Loop ----------------
void loop(){
  int valorMapeado = map(lerPotenciometro(),0,4095,0,200);

  // ---------- SENHA ----------
  if(!senhaLiberada){
    lcdPrintClean(0,"Senha: "+String(valorMapeado));
    if(digitalRead(BUTTON_PIN)==LOW){
      delay(300);
      if(abs(valorMapeado-SENHA)<=TOLERANCIA){
        senhaLiberada=true;
        lcd.clear();
        lcdPrintClean(0,"Acesso Liberado!");
        piscarBuzzerLed(1,150);
        delay(1000);
        lcd.clear();
        lcdPrintClean(0,"Digite MAC");
        lcdPrintClean(1,"do destinatario");
      }else{
        lcd.clear();
        lcdPrintClean(0,"Senha incorreta!");
        piscarBuzzerLed(1,300);
        delay(1000);
        lcd.clear();
        lcdPrintClean(0,"Digite a senha");
        lcdPrintClean(1,"e aperte botao");
      }
    }
    return;
  }

  // ---------- LEITURA MAC ----------
  if(senhaLiberada && !macRecebido){
    if(Serial.available()){
      String macStr = Serial.readStringUntil('\n');
      macStr.trim();
      if(macStr.length()==17){
        for(int i=0;i<6;i++) macDest[i]=strtoul(macStr.substring(i*3,i*3+2).c_str(),NULL,16);
        esp_now_peer_info_t peerInfo={};
        memcpy(peerInfo.peer_addr,macDest,6);
        peerInfo.channel=0;
        peerInfo.encrypt=false;
        if(esp_now_add_peer(&peerInfo)!=ESP_OK) Serial.println("Falha ao adicionar peer");
        else Serial.println("Peer adicionado com sucesso");

        macRecebido=true;
        lcd.clear();
        lcdPrintClean(0,"MAC recebido!");
        delay(1000);
        lcd.clear();
        lcdPrintClean(0,meuMAC.substring(0,8));
        lcdPrintClean(1,meuMAC.substring(8,17));
      }else lcdPrintClean(0,"MAC invalido!");
    }
    return;
  }

  // ---------- ENVIO DE MENSAGEM ----------
  if(macRecebido && !aguardandoEnvio){
    if(Serial.available()){
      mensagem = Serial.readStringUntil('\n');
      aguardandoEnvio=true;
      lcd.clear();
      lcdPrintClean(0,mensagem.length()<=16?mensagem:mensagem.substring(0,16));
      if(mensagem.length()>16) lcdPrintClean(1,mensagem.substring(16,32));
    }
  }

  // ---------- CONFIRMACAO ----------
  if(aguardandoEnvio && !confirmacaoEnvio && digitalRead(BUTTON_PIN)==LOW){
    delay(300);
    confirmacaoEnvio=true;
    lcd.clear();
    lcdPrintClean(0,"Enviar ao");
    lcdPrintClean(1,"destinatario?");
  }

  // ---------- ENVIO VIA ESP-NOW EM MORSE ----------
  if(aguardandoEnvio && confirmacaoEnvio && digitalRead(BUTTON_PIN)==LOW){
    delay(300);
    String morseEnvio = mensagemParaMorse(mensagem);
    esp_err_t result = esp_now_send(macDest,(uint8_t*)morseEnvio.c_str(),morseEnvio.length());
    Serial.print("Resultado envio: ");
    Serial.println(result==ESP_OK?"Sucesso":"Falha");

    lcd.clear();
    lcdPrintClean(0,"Mensagem");
    lcdPrintClean(1,"Enviada!");
    piscarBuzzerLed(1,150);
    delay(2000);
    lcd.clear();
    lcdPrintClean(0,meuMAC.substring(0,8));
    lcdPrintClean(1,meuMAC.substring(8,17));

    aguardandoEnvio=false;
    confirmacaoEnvio=false;
  }
}
