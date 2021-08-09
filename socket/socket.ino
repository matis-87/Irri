#include <DHT12.h>

#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <NTPClient.h>
#include <EEPROM.h>
#include <ArduinoOTA.h>

#include <ESP8266HTTPClient.h>
#include <ESP8266mDNS.h>
#include <EMailSender.h>
#include <BitManLib.h>
#include <TimeLib.h>
#include <strefy.h>
#include <obslugastref.h>
#include <CloudConnection.h>


int chmura=0;
  bool tSt1, tSt2, tSt3;
  int nrProbki=0;
   int srednia = 600;
const int analogInPin = A0;
int sensorValue = 0; 
int wilgGlebyPrev=600;
int wilgGleby=600;

int probkiGleby[10];
bool pierwszyPomiar = false;
DHT12 dht12;
EMailSender emailSend("domolszynowa@gmail.com", "Zakrzewo.5a");
int wyjscie =2;

//****************** Inne ************************
void FloatToSend(float Temp, byte Output[])
{
  byte t1, t2;
  t1 = (byte)Temp;
  t2 = Temp*100 - t1*100;
  Output[0]=t1;
  Output[1]=t2;
}


 //************************Zmienne globalne*********************************
 bool zagarUstawiony;
 byte wifisgn;
 float wilgotnosc;
 float temperatura;
int timeSinceLastRead = 0;
int timeSynchro = 35;

byte NrDnia;
byte AktualnyNrDnia;
Strefy DanyDzien;
const char* ssid     = "INEA-3102_2.4G"; //"Internet_Domowy_38C7B2"; // Tu wpisz nazwÄ™ swojego wifi
const char* password = "aAHpFH4T";//"MzhDN0Iy"; // Tu wpisz hasĹ‚o do swojego wifi
bool jeden;
byte pierwszy;
WiFiUDP Udp;
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pl.pool.ntp.org", 7200, 60000);
byte packetBuffer[255];
WiFiServer wifiServer(80);
 int ledPin = 2; // D4
 int wySt1 =16; //D0
 int wySt2 = 0; //D3
 int wySt3 = 14; //D5
 int wySt4 = 12; //D6
 int wySt5 = 13; //D7
 byte replyBuffer[34];
time_t updateTime=0;
time_t divTime;
byte  dane[10][10];
int godzina;
  Strefy Tydzien[7];
TimeElements Czas;
void ChceckTime()
{
  time_t tempTime;
  
  Serial.println("Czas jest aktualizowany");

   if(timeClient.update())
     {
    tempTime= timeClient.getEpochTime();
    setTime(tempTime);
    zagarUstawiony = true;
    timeSynchro=0;
    Serial.println("Czas został zaktualizowany.:");
        Serial.print(hour());
            Serial.print(":");
                Serial.println(minute());
     }     
  }

void OdczytStatusow(byte dane[])
{
      dane[0]=3;
       dane[1]=0;
          if( DanyDzien.SprawdzStrefe(1,godzina))
          {
           dane[2]=1;
           int pos =  DanyDzien.ZnajdzPozycje(1,true);
          dane[3]= DanyDzien.Dane[pos].Pozostalo;
          dane[4]= DanyDzien.Dane[pos].Dlugosc;
          }
          else
          {
           dane[2]=0;
           dane[3]=0;
          }
//*****************************************************************************************
         if( DanyDzien.SprawdzStrefe(2,godzina))
          {
           dane[15]=1;
           int pos =  DanyDzien.ZnajdzPozycje(2,true);
          dane[16]= DanyDzien.Dane[pos].Pozostalo;
          dane[17]= DanyDzien.Dane[pos].Dlugosc;
          }
          else
          {
           dane[15]=0;
           dane[16]=0;
          }

         if( DanyDzien.SprawdzStrefe(3,godzina))
          {
           dane[18]=1;
           int pos =  DanyDzien.ZnajdzPozycje(3,true);
          dane[19]= DanyDzien.Dane[pos].Pozostalo;
          dane[20]= DanyDzien.Dane[pos].Dlugosc;
          }
          else
          {
           dane[18]=0;
           dane[19]=0;
          }

        if( DanyDzien.SprawdzStrefe(4,godzina))
          {
           dane[21]=1;
           int pos =  DanyDzien.ZnajdzPozycje(4,true);
          dane[22]= DanyDzien.Dane[pos].Pozostalo;
          dane[23]= DanyDzien.Dane[pos].Dlugosc;
          }
          else
          {
           dane[21]=0;
           dane[22]=0;
          }

            if( DanyDzien.SprawdzStrefe(5,godzina))
          {
           dane[24]=1;
           int pos =  DanyDzien.ZnajdzPozycje(5,true);
          dane[25]= DanyDzien.Dane[pos].Pozostalo;
          dane[26]= DanyDzien.Dane[pos].Dlugosc;
          }
          else
          {
           dane[24]=0;
           dane[25]=0;
          }

          //***************************************************************************
          int dzien = AktualnyNrDnia;
        Serial.println(dzien);
          int Nastepny = ZnajdzNastepneZadanie(dzien, Tydzien);
        Serial.println(dzien);
          byte btemp[2];
          byte bwilg[2];
          byte bgleba[2];
          FloatToSend(temperatura,btemp);
          FloatToSend(wilgotnosc,bwilg);


           bgleba[1]= highByte(srednia);
           bgleba[0] = lowByte(srednia);
           dane[27] = bgleba[0];
          dane[28] = bgleba[1];

          dane[5] = btemp[0];
          dane[6] = btemp[1];
          dane[7] = bwilg[0];
          dane[8] = bwilg[1];
          dane[9] = hour();
          dane[10] = minute();
          if(Nastepny>-1)
          {
            int tempG, tempM;
            dane[11] = Tydzien[dzien].Dane[Nastepny].nrStrefy;
            Tydzien[dzien].KonwersjaGodz(Nastepny, tempG, tempM);
            dane[12] = tempG;
            dane[13] = tempM;
            dane[14] = Tydzien[dzien].Dane[Nastepny].Dlugosc;
            dane[32]=dzien;
          }
          else
           dane[11] = 0;
         //Infor ze nie bedzie podlewania dzisiaj 
            dane[29] = DanyDzien.niepodlewaj;
              dane[30]=weekday();
              dane[31]=wifisgn;
}
void OdczytStatusow()
{
    replyBuffer[0]=3;
       replyBuffer[1]=0;
          if( DanyDzien.SprawdzStrefe(1,godzina))
          {
           replyBuffer[2]=1;
           int pos =  DanyDzien.ZnajdzPozycje(1,true);
          replyBuffer[3]= DanyDzien.Dane[pos].Pozostalo;
          replyBuffer[4]= DanyDzien.Dane[pos].Dlugosc;
          }
          else
          {
           replyBuffer[2]=0;
           replyBuffer[3]=0;
          }
//*****************************************************************************************
         if( DanyDzien.SprawdzStrefe(2,godzina))
          {
           replyBuffer[15]=1;
           int pos =  DanyDzien.ZnajdzPozycje(2,true);
          replyBuffer[16]= DanyDzien.Dane[pos].Pozostalo;
          replyBuffer[17]= DanyDzien.Dane[pos].Dlugosc;
          }
          else
          {
           replyBuffer[15]=0;
           replyBuffer[16]=0;
          }

         if( DanyDzien.SprawdzStrefe(3,godzina))
          {
           replyBuffer[18]=1;
           int pos =  DanyDzien.ZnajdzPozycje(3,true);
          replyBuffer[19]= DanyDzien.Dane[pos].Pozostalo;
          replyBuffer[20]= DanyDzien.Dane[pos].Dlugosc;
          }
          else
          {
           replyBuffer[18]=0;
           replyBuffer[19]=0;
          }

        if( DanyDzien.SprawdzStrefe(4,godzina))
          {
           replyBuffer[21]=1;
           int pos =  DanyDzien.ZnajdzPozycje(4,true);
          replyBuffer[22]= DanyDzien.Dane[pos].Pozostalo;
          replyBuffer[23]= DanyDzien.Dane[pos].Dlugosc;
          }
          else
          {
           replyBuffer[21]=0;
           replyBuffer[22]=0;
          }

            if( DanyDzien.SprawdzStrefe(5,godzina))
          {
           replyBuffer[24]=1;
           int pos =  DanyDzien.ZnajdzPozycje(5,true);
          replyBuffer[25]= DanyDzien.Dane[pos].Pozostalo;
          replyBuffer[26]= DanyDzien.Dane[pos].Dlugosc;
          }
          else
          {
           replyBuffer[24]=0;
           replyBuffer[25]=0;
          }

          //***************************************************************************
          int dzien = AktualnyNrDnia;
        Serial.println(dzien);
          int Nastepny = ZnajdzNastepneZadanie(dzien, Tydzien);
        Serial.println(dzien);
          byte btemp[2];
          byte bwilg[2];
          byte bgleba[2];
          FloatToSend(temperatura,btemp);
          FloatToSend(wilgotnosc,bwilg);


           bgleba[1]= highByte(srednia);
           bgleba[0] = lowByte(srednia);
           replyBuffer[27] = bgleba[0];
          replyBuffer[28] = bgleba[1];

          replyBuffer[5] = btemp[0];
          replyBuffer[6] = btemp[1];
          replyBuffer[7] = bwilg[0];
          replyBuffer[8] = bwilg[1];
          replyBuffer[9] = hour();
          replyBuffer[10] = minute();
          if(Nastepny>-1)
          {
            int tempG, tempM;
            replyBuffer[11] = Tydzien[dzien].Dane[Nastepny].nrStrefy;
            Tydzien[dzien].KonwersjaGodz(Nastepny, tempG, tempM);
            replyBuffer[12] = tempG;
            replyBuffer[13] = tempM;
            replyBuffer[14] = Tydzien[dzien].Dane[Nastepny].Dlugosc;
            replyBuffer[32]=dzien;
          }
          else
           replyBuffer[11] = 0;
         //Infor ze nie bedzie podlewania dzisiaj 
            replyBuffer[29] = DanyDzien.niepodlewaj;
              replyBuffer[30]=weekday();
              replyBuffer[31]=wifisgn;
}



void OdczytDaty()
{  
  
  replyBuffer[2]=hour();
  replyBuffer[3]=minute();
  replyBuffer[4]=second();
  replyBuffer[5]=day();
  replyBuffer[6]=month();
  byte tHigh =highByte(year());
  byte tLow = lowByte(year());
  replyBuffer[7]= tHigh;
  replyBuffer[8]=tLow; 
}

void OdczytManual()
{
  replyBuffer[0]=3;
       replyBuffer[1]=3;
          if( DanyDzien.SprawdzStrefe(1,godzina))
          {
           replyBuffer[2]=1;
           int pos =  DanyDzien.ZnajdzPozycje(1,true);
          replyBuffer[3]= DanyDzien.Dane[pos].Pozostalo;
          replyBuffer[4]= DanyDzien.Dane[pos].Dlugosc;
          }
          else
          {
           replyBuffer[2]=0;
           replyBuffer[3]=0;
          }


          if( DanyDzien.SprawdzStrefe(2,godzina))
          {
           replyBuffer[5]=1;
           int pos =  DanyDzien.ZnajdzPozycje(2,true);
          replyBuffer[6]= DanyDzien.Dane[pos].Pozostalo;
          replyBuffer[7]= DanyDzien.Dane[pos].Dlugosc;
          }
          else
          {
           replyBuffer[5]=0;
           replyBuffer[6]=0;
          }

          
          if( DanyDzien.SprawdzStrefe(3,godzina))
          {
           replyBuffer[8]=1;
           int pos =  DanyDzien.ZnajdzPozycje(3,true);
          replyBuffer[9]= DanyDzien.Dane[pos].Pozostalo;
          replyBuffer[10]= DanyDzien.Dane[pos].Dlugosc;
          }
          else
          {
           replyBuffer[8]=0;
           replyBuffer[9]=0;
          }


          
          if( DanyDzien.SprawdzStrefe(4,godzina))
          {
           replyBuffer[11]=1;
           int pos =  DanyDzien.ZnajdzPozycje(4,true);
          replyBuffer[12]= DanyDzien.Dane[pos].Pozostalo;
          replyBuffer[13]= DanyDzien.Dane[pos].Dlugosc;
          }
          else
          {
           replyBuffer[11]=0;
           replyBuffer[12]=0;
          }

  
          if( DanyDzien.SprawdzStrefe(5,godzina))
          {
           replyBuffer[14]=1;
           int pos =  DanyDzien.ZnajdzPozycje(5,true);
          replyBuffer[15]= DanyDzien.Dane[pos].Pozostalo;
          replyBuffer[16]= DanyDzien.Dane[pos].Dlugosc;
          }
          else
          {
           replyBuffer[14]=0;
           replyBuffer[15]=0;
          }
}




void OdczytajDzien(int nrDnia, byte dane[40])
{
    int Adres = nrDnia*40;
  EEPROM.begin(512);
  Serial.println("Odczyt z pamieci: ");
  for(int i=0;i<40;i++)
  {
    dane[i] = EEPROM.read(Adres+i);     
    Serial.println(dane[i]);
  }  
 
}
















void Odpowiedz(byte adres)
{
  Serial.print("Adres: ");
  Serial.println(adres);
  switch(adres)
  {
    case 0 : OdczytStatusow();
              break;
    case 1 : OdczytDaty();
              break;
    case 3: OdczytManual();
            break;
    case 30: OdczytHarmo(Tydzien, replyBuffer);
            break;
  }
}





int kodMaila;
void WyslijMail()
{
    EMailSender::EMailMessage message;
String dni[8];
dni[1] = "Niedziela";
dni[2] = "Poniedzialek";
dni[3] = "Wtorek";
dni[4] = "Środa";
dni[5] = "Czwartek";
dni[6] = "Piątek";
dni[7] = "Sobota";
  if(kodMaila==1)
  {
    message.subject = "Nowy dzien";
    message.message = "Jest "+dni[weekday()]+" <br> Godzina: "+ hour()+":"+minute()+":"+second()+". <br> Pozdrawiam z Olszynowej";
  }
    if(kodMaila==2)
  {
    message.subject = "Nawadnianie";
    message.message = dni[weekday()]+" Godzina: "+ hour()+":"+minute()+":"+second()+"<br> Załączyłem nawadnienie strefy 1. <br> Pozdrawiam z Olszynowej";
  }

      if(kodMaila==3)
  {
    message.subject = "Nawadnianie";
    message.message = dni[weekday()]+" Godzina: "+ hour()+":"+minute()+":"+second()+"<br> Wyłączyłem nawadnienie strefy 1. <br> Pozdrawiam z Olszynowej";
  }

      if(kodMaila==4)
  {
    message.subject = "Nawadnianie";
    message.message = dni[weekday()]+" Godzina: "+ hour()+":"+minute()+":"+second()+"<br> Załączyłem nawadnienie strefy 2. <br> Pozdrawiam z Olszynowej";
  }

      if(kodMaila==5)
  {
    message.subject = "Nawadnianie";
    message.message = dni[weekday()]+" Godzina: "+ hour()+":"+minute()+":"+second()+"<br> Wyłączyłem nawadnienie strefy 2. <br> Pozdrawiam z Olszynowej";
  }

      if(kodMaila==6)
  {
    message.subject = "Nawadnianie";
    message.message = dni[weekday()]+" Godzina: "+ hour()+":"+minute()+":"+second()+"<br> Załączyłem nawadnienie strefy 3. <br> Pozdrawiam z Olszynowej";
  }

      if(kodMaila==7)
  {
    message.subject = "Nawadnianie";
    message.message = dni[weekday()]+" Godzina: "+ hour()+":"+minute()+":"+second()+"<br> Wyłączyłem nawadnienie strefy 3. <br> Pozdrawiam z Olszynowej";
  }

if(kodMaila>0)
{
EMailSender::Response resp = emailSend.send("m4thi.s@gmail.com", message);
if(resp.status==true)
 kodMaila=0;
}
}


int sredniaGleby()
{

  int sum =0;
  for(int i=0;i<10;i++)
  {
    sum+= probkiGleby[i];
  }
  return sum/10;
}
void WyslijDoChmury()
{
  byte test[10];
  for(int i=0;i<10;i++)
  {
    test[i]=i*5;
  }
unsigned char base64[25];

//int j = encode_base64(test,10,base64);
   String dd = String(hour())+":"+String(minute())+":"+String(second());// (char *)base64;
   

      chmura=0;
}
 
 CloudConnection iChmura;

//************************SETUP*********************
void setup() {

  digitalWrite(wySt1, HIGH);
digitalWrite(wySt2, HIGH);
digitalWrite(wySt3, HIGH);
digitalWrite(wySt4, HIGH);
digitalWrite(wySt5, HIGH);
  Serial.begin(115200);
 pinMode(ledPin, OUTPUT);
 pinMode(wySt1, OUTPUT);
 pinMode(wySt2, OUTPUT);
 pinMode(wySt3, OUTPUT);
 pinMode(wySt4, OUTPUT);
 pinMode(wySt5, OUTPUT);
tSt1=true;
tSt2=true;
tSt3=true;
  delay(1000);
     Serial.println("Connecting..");
     WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
 jeden=false;
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting..");
    wifisgn=0;
   // ESP.restart();
  }

    ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH) {
      type = "sketch";
    } else { // U_FS
      type = "filesystem";
    }

    // NOTE: if updating FS this would be the place to unmount FS using FS.end()
    Serial.println("Start updating " + type);
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) {
      Serial.println("Auth Failed");
    } else if (error == OTA_BEGIN_ERROR) {
      Serial.println("Begin Failed");
    } else if (error == OTA_CONNECT_ERROR) {
      Serial.println("Connect Failed");
    } else if (error == OTA_RECEIVE_ERROR) {
      Serial.println("Receive Failed");
    } else if (error == OTA_END_ERROR) {
      Serial.println("End Failed");
    }
  });
  ArduinoOTA.begin();
  Serial.println("Ready");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
   timeClient.begin();
  Serial.print("Connected to WiFi. IP:");
  Serial.println(WiFi.localIP());

  Udp.begin(8080);
  replyBuffer[0]=3;
 NrDnia = 0;
 dht12.begin();
 ChceckTime();
OdczytajHarmonogramZPamieci(Tydzien, NrDnia, AktualnyNrDnia, DanyDzien);

wilgGlebyPrev = 600;
byte Buff[255];

String paczka="S:";
char znak;
byte przyklad[10];
for(int i=0;i<10;i++)
{
  przyklad[i] = i * 5;
  znak = przyklad[i];
paczka+=znak;
}

  byte test[10];
  for(int i=0;i<10;i++)
  {
    test[i]=i*5;
  }

     HTTPClient http;


}




//************************Loop********************************
void loop() {
  ArduinoOTA.handle();
 int packetSize = Udp.parsePacket();
 byte val=0;

 AktualnyNrDnia = weekday();
 if (AktualnyNrDnia != NrDnia)
 {
  NrDnia = AktualnyNrDnia;
  KopiujDaneDnia(Tydzien[NrDnia-1], DanyDzien);
 Serial.println("===== DanyDZien =====");
  Serial.println(DanyDzien.index);
   Serial.println("===Koniec=====");

  DanyDzien.niepodlewaj=false;
  Serial.println("***********NrDnia:********* ");
   Serial.println(NrDnia);
  kodMaila=1;
 }

godzina     = hour()*60 + minute();

//******************Czujnik temp*************
int res=-10;
bool stat;
if(timeSinceLastRead > 1000) {//zmienić na 2000
  Serial.print("======= remote client connection ======== => ");
/*  Serial.println(iChmura.ClientStatus);
if(iChmura.ClientStatus)
  res = iChmura.GetRequest();
else
  stat = iChmura.GetClientStatus();
     if(iChmura.ClientStatus)
   {    
      if((res<100)&&(res>=0))
     {
       Odpowiedz((byte)res);
    
         Serial.print("Buffer:  ");
    Serial.println(sizeof(replyBuffer));
    iChmura.Encode64(replyBuffer);
    byte t = iChmura.SendData();

    res=0;
     }
   }*/
  Serial.print("sensor = ");
  Serial.println(sensorValue);
  long rssi = WiFi.RSSI();
  float sygnal = ((120.0+rssi)/120.0)*100.0;
  wifisgn = (byte)sygnal;
  Serial.print("RSSI [dBm]:");
  Serial.println(rssi);
temperatura = dht12.readTemperature();
wilgotnosc = dht12.readHumidity();
bool dht12Read = true;
if (isnan(temperatura) || isnan(wilgotnosc)) {
 temperatura = 0;
 wilgotnosc = 0;
dht12Read = false;
}

if (dht12Read){
// Compute heat index in Celsius (isFahreheit = false)
float hic12 = dht12.computeHeatIndex(temperatura, wilgotnosc, false);
// Compute dew point in Celsius (isFahreheit = false)
float dpc12 = dht12.dewPoint(temperatura, wilgotnosc, false);
}


sensorValue = analogRead(analogInPin);

          //Wilgotnosc gleby
if((sensorValue>50))
 {
   wilgGleby = sensorValue;
   if(!pierwszyPomiar)
    {
     for(int i=0;i<10;i++)
     {
      probkiGleby[i] = wilgGleby;
     }
     pierwszyPomiar = true;
    }
    else
    {
      probkiGleby[nrProbki] = wilgGleby;
    }
    nrProbki++;
    if(nrProbki>9)
      nrProbki = 0;
    srednia = sredniaGleby();
          }
   /*  chmura++;
     if(chmura>10)
     WyslijDoChmury();   */  

timeSynchro++;
if(timeSynchro>30)
{
  ChceckTime();
}
timeSinceLastRead = 0;


}

    delay(500);
    timeSinceLastRead += 100;
   
   

    //*****************************************************************************
    
   if (packetSize)
  {
    IPAddress remoteIp = Udp.remoteIP();
    int len = Udp.read(packetBuffer, 255);
    if (len > 0) packetBuffer[len] = 0;

//**************Komenda do Arduino////////////////////////////
      if (packetBuffer[0]==1)
      {
 int rozkaz = packetBuffer[1];
//***Tryb reczny***
  switch(rozkaz)
  {
    case 30 :  ManualStrefa(1,packetBuffer,DanyDzien,hour(), minute());
              break;
    case 31 :  ManualStrefa(2,packetBuffer,DanyDzien,hour(), minute());
              break; 
    case 32 :  ManualStrefa(3,packetBuffer,DanyDzien,hour(), minute());
              break;
    case 33 :  ManualStrefa(4,packetBuffer,DanyDzien,hour(), minute());
              break;
    case 34 :  ManualStrefa(5,packetBuffer,DanyDzien,hour(), minute());
              break;
    case 11:  ZapisHarmoStrefy(1, packetBuffer,Tydzien,DanyDzien,NrDnia,AktualnyNrDnia);
              break;
    case 12:  ZapisHarmoStrefy(2, packetBuffer,Tydzien,DanyDzien,NrDnia,AktualnyNrDnia);
              break;
    case 13:  ZapisHarmoStrefy(3, packetBuffer,Tydzien,DanyDzien,NrDnia,AktualnyNrDnia);
              break;
    case 14:  ZapisHarmoStrefy(4, packetBuffer,Tydzien,DanyDzien,NrDnia,AktualnyNrDnia);
              break;
    case 15:  ZapisHarmoStrefy(5, packetBuffer,Tydzien,DanyDzien,NrDnia,AktualnyNrDnia);
              break;
    case 1:  DanyDzien.OminPodlewanie();
             break;
  }
  
       replyBuffer[0]=3;
       replyBuffer[1]=packetBuffer[1];
       replyBuffer[2]=GetByte(jeden,0);

      }
//*****************Pobranie informacji z arduino*******************
     if (packetBuffer[0]==2)
      {
       Odpowiedz(packetBuffer[1]);
      }
    Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
    Udp.write(replyBuffer,34);
    Udp.endPacket();    
    }
bool st1, st2, st3, st4, st5, led;
if(zagarUstawiony)
{
  st1 =  !DanyDzien.SprawdzStrefe(1,godzina);
  st2 =  !DanyDzien.SprawdzStrefe(2,godzina);
  st3 =  !DanyDzien.SprawdzStrefe(3,godzina);
  st4 =  !DanyDzien.SprawdzStrefe(4,godzina);
  st5 =  !DanyDzien.SprawdzStrefe(5,godzina);

jeden=!jeden;
if(!(!st1 || !st2 || !st3 || !st4 || !st5))
  led = true;
else
  led = false;

  digitalWrite(ledPin, led);
  digitalWrite(wySt1, st1);
  digitalWrite(wySt2, st2);
  digitalWrite(wySt3, st3);
  digitalWrite(wySt4, st4);
  digitalWrite(wySt5, st5);
if((st1==false)&&(tSt1==true))
  kodMaila=2;
if((st1==true)&&(tSt1==false))
  kodMaila=3;

if((st2==false)&&(tSt2==true))
  kodMaila=4;
if((st2==true)&&(tSt2==false))
  kodMaila=5;

if((st3==false)&&(tSt3==true))
  kodMaila=6;
if((st3==true)&&(tSt3==false))
  kodMaila=7;

  
  tSt1 = st1;
  tSt2 = st2;
  tSt3 = st3;
}

}
