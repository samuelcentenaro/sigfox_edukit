#define RESET 4
#define TX 2
#define RX 3
#define LED 13
#define SW1 5

#include <SoftwareSerial.h>

SoftwareSerial serial_HT(RX, TX);

int Contador;

int16_t Temperatura;

void setup() {
  digitalWrite(RESET, HIGH);
  pinMode(RESET, OUTPUT);
  Serial.begin(9600);
  serial_HT.begin(9600);

  Serial.println("************************************************");
  Serial.println("* Menu de opções de comandos, tecle de 1 a 5   *");
  Serial.println("* 1 - RESET HT32SX por comando AT              *");
  Serial.println("* 2 - RESET HT32SX por hardware                *");
  Serial.println("* 3 - Configura para operar em RCZ2            *");
  Serial.println("* 4 - Envia dados (UpLink)                     *");
  Serial.println("* 5 - Envia dados e aguarda (Donwlink)         *");
  Serial.println("*                                              *");
  Serial.println("*OBS: nas opções 4 e 5 aguardar 35 segundos ou *");
  Serial.println("*até o LED apagar para envio de novos comandos *");
  Serial.println("************************************************");
  reset_HT();
  delay(5000);
  Serial.println("Vai enviar configuração AT+CFGCZ=2");
  serial_HT.print("AT+CFGRCZ=2;");

  pinMode(SW1, INPUT_PULLUP);
  pinMode(LED, OUTPUT);
}

void loop() {

 char buf[36];
 char c;

 Contador = 0;
 Temperatura = 1000;

 while(true){
   if(serial_HT.available())
    {
      c = serial_HT.read();
      Serial.print(c);
    }
    
   if(Serial.available())
   {
    c = Serial.read();
   
 
switch(c) {
  case '1':
    Serial.println("Vai executar AT+RESET");
    serial_HT.print("AT+RESET;");
    break;
  
  case '2':
   Serial.println("Vai executar reset de hardware");
    reset_HT();
  break;
  
  case '3':
   Serial.println("Vai enviar configurações AT+CFGRCZ=2");
    serial_HT.print("AT+CFGRCZ=2;");
  break;
  
  case '4':
   Serial.println("Vai enviar mensagem SIGFOX");
   sprintf(buf, "AT+SEND=0:%02x%04x;",Contador, Temperatura);
   Serial.println(buf);
   serial_HT.print(buf);
   delay_msg();
   break;
  
  case '5':
   Serial.println("Enviar mensagem SIGFOX e esperar downlink");
    sprintf(buf, "AT+SEND=1:%02x%04x;",Contador, Temperatura);
   Serial.println(buf);
   serial_HT.print(buf);
   delay_msg();
  break;
}
}


 int sensorVal = digitalRead(SW1);
 if(sensorVal == HIGH) {
  digitalWrite(LED,LOW);
 }else{
  digitalWrite(LED,HIGH);
  Contador++;
  Temperatura += 33;
  Serial.println("Botao pressionado, vai enviar mensagem SIGFOX");
  sprintf(buf, "AT+SEND=0:%02x%04x;",Contador, Temperatura);
   Serial.println(buf);
   serial_HT.print(buf);
   delay_msg();

   while(!digitalRead(SW1))
   {
    delay(10);
   }
 }
 }

}

void reset_HT() {
  digitalWrite(RESET, HIGH);
  delay(1000);
  digitalWrite(RESET, LOW);
  delay(100);
}


int Char2Int(char c){
  int x;
  if(c >= '0' && c <= '9'){
    x = c - '0';    
  }else if(c >= 'a' && c <= 'f'){
    x = (c - 'a') + 10;
  }else return(-1);
  return(x);
}

void delay_msg()
{
  int Temp_H;
  int Temp_L;
  int Cont;
  char c;
  int x;
  int xPos;
  char buf_aux[60];

  digitalWrite(LED, HIGH);
  Serial.println("Aguarde 45 segundos ou até o LED apagar...");
  uint16_t i;

  for(i=0; i<45000;i++){
    if(serial_HT.available())
    {
      String resposta = serial_HT.readString();
      Serial.print(resposta);
      if(resposta.indexOf("custumer resp")> 0){
        xPos = resposta.indexOf('x');
        c = resposta[xPos+1];
        Cont = Char2Int(c);
        c = resposta[xPos+2];
        if(c != ','){
          x = Char2Int(c);
          Cont *=16;
          Cont += x;
          }

          xPos = resposta.indexOf('x', xPos + 1 );
          c = resposta[xPos+1];
          Temp_H = Char2Int(c);
          c = resposta[xPos+2];
          if(c!=','){
            x = Char2Int(c);
          Cont *=16;
          Cont += x;
          }

           xPos = resposta.indexOf('x', xPos + 1 );
          c = resposta[xPos+1];
          Temp_H = Char2Int(c);
          c = resposta[xPos+2];
          if(c!=','){
            x = Char2Int(c);
          Cont *=16;
          Cont += x;
          }

          Contador = Cont;
          Temperatura = Temp_H;
          Temperatura *=256;
          Temperatura += Temp_L;

          sprintf(buf_aux, "Valor recebido: Contador=0x%02x Temperatura=0x%04x;", Contador, Temperatura);
          Serial.println(buf_aux);
      }
      if(resposta.indexOf("AT_cmd_Waiting...") > 0){
        i = 50000;
      }
    }
    delay(1);
  }
  digitalWrite(LED, LOW);
  Serial.println("Pronto para receber novo comando");
}
