# Morse-Communicator-P2P

Este repositório contém uma atividade desenvolvida no **Inteli**, utilizando a plataforma Arduino.  
O objetivo é um comunicador p2p que envia e recebe, criptografando e descriptogrando com código morse mensagens via ESP-NOW.

## Objetivo

Este projeto tem como objetivo demonstrar a **comunicação entre dois microcontroladores ESP32** utilizando o protocolo **ESP-NOW**, sem necessidade de Wi-Fi tradicional.  
A troca de mensagens é feita de forma **codificada em código Morse**, exibida no **display LCD (I²C)** e com **feedback visual e sonoro** por meio de **LED** e **buzzer**.  

## Lógica Geral

1. **Autenticação:**  
   O usuário gira o **potenciômetro** até aproximar-se da senha configurada (`SENHA = 100`, com tolerância de `3`). Pressiona o botão para confirmar.  

2. **Envio de MAC:**  
   Após liberar o acesso, o usuário digita o **MAC Address** do outro ESP32 no monitor serial.  

3. **Envio de mensagem:**  
   O usuário digita a mensagem desejada no monitor serial.  

4. **Confirmação:**  
   O dispositivo solicita confirmação via botão para enviar.  

5. **Transmissão Morse:**  
   A mensagem é **convertida em código Morse** e enviada via **ESP-NOW**. LED e buzzer piscam conforme o padrão Morse durante envio e recepção.  

6. **Exibição:**  
   A mensagem decodificada é exibida no **LCD**, e o **MAC do dispositivo** é mostrado abaixo.

## Como Executar  

1. Carregue o código nos dois ESP32s (PEER A & PEER B).  
2. Abra o **Monitor Serial** em 115200 baud.  
3. Em um dos ESP32, pegue o MAC próprio exibido e informe-o no outro para parear.  
4. No dispositivo de envio: gire o potenciômetro e pressione o botão para liberar acesso (senha).  
5. Digite o MAC do destino via Serial.  
6. Digite a mensagem e confirme com o botão.  
7. Observe LED/buzzer piscando no receptor e a mensagem exibida no LCD.

## Créditos  

Projeto desenvolvido por  
**Victor Santos** e **João Vitor Araújo**  
