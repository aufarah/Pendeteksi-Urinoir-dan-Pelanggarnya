#include<EEPROM.h>

const int pingPin = 7; // Trigger Pin of Ultrasonic Sensor
const int echoPin = 6; // Echo Pin of Ultrasonic Sensor
const int piezo = A4; //piezosensor di A0
const int batas = 30;
int milos = 1; //ini periode ke..
int pengguna = 0; //pengguna direset per periode
   int pelanggar = 0;
int wait = 10; //tunggu kepastian orang bak (biasanya 10s)
int period = 3600*1000; //periode tiap berapa sekon data diambil (perjam harusnya 3600s)
int take = 12; //data diambil selama 12 periode (harusnya 12 jam)
   // int take = 12*2; //data diambil selama 12 periode (harusnya 12 jam), untuk slot memori dua kali lipat
int benar = 0;
int out;
int out2;
int digunakan;
int mem_pengguna, mem_pelanggar;

long microsecondsToCentimeters(long microseconds) {
   return microseconds / 29 / 2; // rumus fungsi konversi ke inci
}

int baca_piezo(){
  int bacapiezo = analogRead(piezo);
  int out;
  if (bacapiezo>330){
    out = 1;
  } else {
    out =0;
  }
  return out;
}

int baca_sonik(){
   long duration, inches, cm;
   pinMode(pingPin, OUTPUT); //pin 7 jadi output masuk ke trigger
   digitalWrite(pingPin, LOW); //pin 7 low
   delayMicroseconds(2);
   digitalWrite(pingPin, HIGH); //pin 7 di on 10 us, ketrigger
   delayMicroseconds(10);   digitalWrite(pingPin, LOW); //tutup trigger, karena sensor sudah nyala
   pinMode(echoPin, INPUT); //pin 6 terima pantulan
   duration = pulseIn(echoPin, HIGH);
   cm = microsecondsToCentimeters(duration); //ke meter
   
   return cm;
}





void setup() {
   Serial.begin(9600); // Starting Serial Terminal
}

void loop() {
   int i = 0, j;
   int avesonik = 0;
   while(i<=wait){
    out = baca_sonik();
    avesonik = (avesonik*(i+1) + out)/(i+2);
    //syarat out<1000 dipakai untuk menganulir out noise 2000an yang sering terjadi, avesonik untuk menganulir eror dadakan di rentang 60-120
    if (out>batas && out<1000 && (avesonik == 0 || avesonik>75)){ 
      break;
    }
    i++;
    //Serial.print(".");
   delay(1000);
   }

   if (i>=wait) {
    //Serial.println("Ada pengguna");
    pengguna++;
    digunakan = 1;
    Serial.print("Jumlah pengguna sampai saat ini: ");
    Serial.println(pengguna);
    //Serial.println(digunakan);
   }

   
   //loop ini dipakai untuk mengecek penekanan flush selama orang masih ada, cek terus menerus
   j=0;
   out = baca_sonik();
   while(digunakan ==1 && (out<batas || out>1000)){ //loop tunggu yang buang air pergi dari urinoir
    out2 = baca_piezo(); //break kalau ada hasil piezo
    //out2 = 0;
    if (out2 == 1){
      benar=1; //kalau sudah flush maka benar
      break;
    }
    avesonik = (avesonik*(j+1) + out)/(j+2);
    if (out>batas && out<1000 && (avesonik == 0 || avesonik>75)){
      break;
    }
    out = baca_sonik();
    j++;
    delay(100);                             
   }

  //loop kedua ini untuk memastikan orang benar-benar keluar sesudah flush
  j=0;
  out = baca_sonik();
   while(benar == 1 && (out<batas) ){ //loop tunggu yang buang air pergi dari urinoir
    avesonik = (avesonik*(j+1) + out)/(j+2);
    if (out>batas && out<1000 && (avesonik == 0 || avesonik>75)){
      break;
    }
    out = baca_sonik();
    j++;
    delay(100); 
   }

   if (i>=wait && digunakan>0 && benar==0){   //syarat terjadi pelanggaran
    pelanggar++;
    Serial.print("Jumlah pelanggar saat ini: ");
    Serial.println(pelanggar);
   }


    
   if(millis()/period>=milos && milos<=take){  //periode 10.000 ms ataua 10 s, tiap periode(milos) direset
    //Serial.println("asu");
    //Serial.println();
    Serial.print("Pengguna per jam: ");
    Serial.println(pengguna);
    Serial.print("Pelanggar per jam: ");
    Serial.println(pelanggar);
    Serial.print("Time: ");
    Serial.println(milos);
    Serial.println();

    //write to eeprom
    EEPROM.write((milos-1)*2,pengguna); //alamat tiap milos-1, isinya total pengguna perperiode
    EEPROM.write((milos-1)*2+1,pelanggar); //alamat tiap milos-1, isinya total pelanggar perperiode

    milos++;
    pengguna = 0;
    pelanggar = 0;
    /*
      //write to eeprom
      EEPROM.write(milos-1,pengguna); //alamat tiap milos-1, isinya total pengguna perperiode
      EEPROM.write(milos, pelanggar); //alamat sebelahnya isinya total pelanggar

      milos+=2;
      pengguna = 0;
      pelanggar = 0;
     */
   }

   digunakan = 0;
   benar = 0;
   
}
