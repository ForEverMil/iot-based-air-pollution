//Định nghĩa địa chỉ kết nối và mã xác thực với template trên Blynk
#define BLYNK_TEMPLATE_ID " TMPL68DCWEu3y" //ID kết nối với template
#define BLYNK_TEMPLATE_NAME " DO AN TOI NGHIEP" //Tên template
#define BLYNK_AUTH_TOKEN "dFL272H54etXFDxdOSnVccVPj4X978XT" //Mã xác thực với template

//Khai báo các thư viện được sử dụng
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <SoftwareSerial.h>
#include "DHT.h"
#include "Wire.h"
#include <LiquidCrystal_I2C.h>
#include <PCF8574.h>

//Khai báo địa chỉ giao tiếp cho các module I2C
LiquidCrystal_I2C lcd(0x27, 20, 4);
PCF8574 pcf8574(0x20);

//Khai báo chuỗi mã xác thức với utemplate và WIFI kết nối
char auth[] = BLYNK_AUTH_TOKEN;
char ssid[] = "Redmi Note 9 Pro";        //tên WIFI kết nối
char pass[] = "12071974";         //Pass WIFI kết nối

//Định nghĩa chân kết nối cho DHT11
#define DHTPIN D5  
#define DHTTYPE DHT11 
DHT dht(DHTPIN, DHTTYPE);

//Khai báo các biến và hằng
int select_cb=D6;
int dustPin=A0;
int ledPower=D7;
int delayTime=280;
int delayTime2=40;
float offTime=9680; //10000-280-40
int dustVal=0;
char s[32];
float voltage = 0;
float dustdensity = 0.0;
int data_mq135 = 0;
int k = 0;
int t, h, kk, bui, tt;
int DenTr, DenNg, QuatG, CoiB;
int Nguong = 200;
const int DEN_TRONG = 0;
const int DEN_NGOAI = 1;
const int QUAT_GIO = 2;
const int COI_BAO = 3;

//Khai báo các mãng để thiết lập ký tự đặc biệt cho LCD
byte pp[] = {
  B11100,
  B10100,
  B11000,
  B10111,
  B10101,
  B00110,
  B00100,
  B00100
}; //Ký tự pp

byte oC[] = {
  B11000,
  B11000,
  B00111,
  B01000,
  B01000,
  B01000,
  B01000,
  B00111
}; //Ký tự oC

byte p_tram[] = {
  B11100,
  B10101,
  B11110,
  B00100,
  B01000,
  B10111,
  B00101,
  B00111
}; //Ký tự %


//Khai báo Widget của LCD trên app Blynk để truyền dữ liệu hiện thị
WidgetLCD Blynk_LCD(V8);

//Chương trình thiết lập ban đầu
void setup(){
  Blynk.begin(auth, ssid, pass);
  dht.begin();
  Wire.begin(D2, D1); //D5=SDA, D6=SCL
  pcf8574.begin();
  Serial.begin(9600);

  //Thiết lập piMode cho các biến
  pinMode(ledPower,OUTPUT);
  pinMode(select_cb,OUTPUT); 

  //Thiết lập trang thái ban đầu cho các pinMode
  digitalWrite(select_cb, LOW);
  pcf8574.write(DEN_TRONG, LOW);
  pcf8574.write(DEN_NGOAI, LOW);
  pcf8574.write(QUAT_GIO, LOW);
  pcf8574.write(COI_BAO, LOW);

  //Thiết lập LCD ban đầu
  lcd.init();
  lcd.clear();
  lcd.backlight();
  lcd.createChar(0, oC);
  lcd.createChar(1, p_tram);
  lcd.createChar(2, pp);

  //Thiết lập hiển thị hàng 0
  lcd.setCursor(0,0);
  lcd.print("--DO AN TOT NGHIEP--");

  //Thiết lập hiển thị hàng 1
  lcd.setCursor(0,1);
  lcd.print("NhDo:");
  lcd.setCursor(7,1);
  lcd.write(0);
  lcd.setCursor(8,1);
  lcd.print("   DoAm:");
  lcd.setCursor(19,1);
  lcd.write(1);

  //Thiết lập hThiết lập hiển thị hàng 2
  lcd.setCursor(0,2);
  lcd.print("KDoc:");
  lcd.setCursor(8,2);
  lcd.write(2);
  lcd.setCursor(9,2);
  lcd.print("m ");
  lcd.setCursor(11,2);
  lcd.print("Bui:");
  lcd.setCursor(18,2);
  lcd.print("ug");
}

void loop(){
  Blynk.run(); //Chương trình con đọc và truyền tín hiếu hiệu điều khiển qua Blynk
  CB_KK_BUI(); //Chương trình con đọc cảm biến MQ135 và GP2Y1014
  CB_DHT11(); // Chương trình con đọc cảm biến DHT11

  //Truyền các dữ liệu đọc từ cảm biến lên Blynk
  Blynk.virtualWrite(V0, t);
  Blynk.virtualWrite(V1, h);
  Blynk.virtualWrite(V2, kk);
  Blynk.virtualWrite(V3, bui);

  thietlap_TTCB(bui = bui); // Chương trình con thiết lập trạng thái cảnh báo
  Hien_Thi(t=t, h=h, kk=kk, bui=bui, tt=tt); //Chương trình con hiển thị lên LCD20x4
  DK_Canhbao(kk=kk, Nguong = Nguong, QuatG = QuatG ); //chương trình con điều khiển cảnh báo
  delay(500);

}

//chương trình con điều khiển cảnh báo
void DK_Canhbao(int kk, int Nguong, int QuatG){
  if((kk > Nguong) || (QuatG == 1)){
    pcf8574.write(QUAT_GIO, HIGH);
    if(kk > Nguong) pcf8574.write(COI_BAO, HIGH);
  } else{
    pcf8574.write(QUAT_GIO, LOW);
    pcf8574.write(COI_BAO, LOW);
  }
}

// Chương trình con thiết lập trạng thái cảnh báo
void thietlap_TTCB(int bui){
  if(bui<=50) tt=1;
  if((bui>=51)&&(bui<=100)) tt=2;
  if((bui>=101)&&(bui<=150)) tt=3;
  if((bui>=151)&&(bui<=200)) tt=4;
  if((bui>=201)&&(bui<=300)) tt=5;
  if(bui>=301) tt=6;
}

//Chương trình con hiển thị lên LCD20x4
void Hien_Thi(int t, int h, int kk, int bui, int tt){
  lcd.setCursor(5,1);
  lcd.print(t);
  lcd.setCursor(16,1);
  lcd.print(h);
  lcd.setCursor(5,2);
  lcd.print("   ");
  lcd.setCursor(5,2);
  lcd.print(kk);
  lcd.setCursor(15,2);
  lcd.print("   ");
  lcd.setCursor(15,2);
  lcd.print(bui);

  if( tt == 1){
    lcd.setCursor(0,3);
    lcd.print("                    ");
    lcd.setCursor(0,3);
    lcd.print("     Trong Sach     "); //Khi chất lượng KK từ 0 - 50

    Blynk_LCD.print(0, 0, "Chất lượng K Khí");
    Blynk_LCD.print(0, 1, "   Trong Sạch   ");
  }
  if( tt == 2){
    lcd.setCursor(0,3);
    lcd.print("                    "); 
    lcd.setCursor(0,3);
    lcd.print("     Trung Binh     "); //Khi chất lượng KK từ 51 - 100

    Blynk_LCD.print(0, 0, "Chất lượng K Khí");
    Blynk_LCD.print(0, 1, "   Trung Bình   ");
  }
  if( tt == 3){
    lcd.setCursor(0,3);
    lcd.print("                    ");
    lcd.setCursor(0,3);
    lcd.print("         Kem        "); //Khi chất lượng KK từ 101 - 150

    Blynk_LCD.print(0, 0, "Chất lượng K Khí");
    Blynk_LCD.print(0, 1, "       Kém      ");
  }
  if( tt == 4){
    lcd.setCursor(0,3);
    lcd.print("                    ");
    lcd.setCursor(0,3);
    lcd.print("         Xau        "); //Khi chất lượng KK từ 151 - 200  

    Blynk_LCD.print(0, 0, "Chất lượng K Khí");
    Blynk_LCD.print(0, 1, "       Xấu      ");
  }
  if(tt == 5){
    lcd.setCursor(0,3);
    lcd.print("                    ");
    lcd.setCursor(0,3);
    lcd.print("       Rat Xau      "); //Khi chất lượng KK từ 201 - 300

    Blynk_LCD.print(0, 0, "Chất lượng K Khí");
    Blynk_LCD.print(0, 1, "      Rất Xấu   ");
  }
  if(tt == 6){
    lcd.setCursor(0,3);
    lcd.print("                    ");
    lcd.setCursor(0,3);
    lcd.print("      Nguy hai      "); //Khi chất lượng KK từ lớn hơn 300

    Blynk_LCD.print(0, 0, "Chất lượng K Khí");
    Blynk_LCD.print(0, 1, "     Nguy Hại   ");
  }
}

// Chương trình con đọc cảm biến DHT11
void CB_DHT11(){
  h = dht.readHumidity();
  t = dht.readTemperature();

  if (isnan(h) || isnan(t)) {
    Serial.println(F("Failed to read from DHT sensor!"));
    return;
  }
  Serial.print(F("Humidity: "));
  Serial.print(h);
  Serial.print(F("%  Temperature: "));
  Serial.print(t);
  Serial.println(F("°C "));
}

//Chương trình con đọc cảm biến MQ135 và GP2Y1014
void CB_KK_BUI(){
  if (k==0){
    digitalWrite(select_cb, LOW);
    delay(200);
    data_mq135 = analogRead(dustPin);
    kk = data_mq135;
    Serial.print(data_mq135);
    Serial.println(" ppm");
    k = 1;
  }
  else{
  digitalWrite(select_cb,   HIGH);
  digitalWrite(ledPower,LOW); // power on the LED
  delayMicroseconds(delayTime);
  dustVal=analogRead(dustPin); // read the dust value
  delayMicroseconds(delayTime2);
  digitalWrite(ledPower,HIGH); // turn the LED off
  delayMicroseconds(offTime);
  voltage = dustVal*(5.0 / 1024.0);//dustval*5/1024
  dustdensity = 0.172*voltage-0.1;  
  if (dustdensity < 0 )
    dustdensity = 0;
  if (dustdensity > 0.5)
    dustdensity = 0.5;
  bui = dustdensity * 1000;
  String dataString = "";
  dataString += dtostrf(voltage, 9, 4, s);
  dataString += "V,";
  dataString += dtostrf(dustdensity*1000.0, 5, 2, s);
  dataString += "ug/m3";
  Serial.println(dataString);
  delay(500);
  k=0;
  }
}

//Các chương trình con đọc và truyền tín hiếu hiệu điều khiển qua Blynk
BLYNK_WRITE(V4)
{
  DenTr = param.asInt();
  if (DenTr == 1){
    pcf8574.write(DEN_TRONG, HIGH);
  } else {
    pcf8574.write(DEN_TRONG, LOW);  
  }
}

BLYNK_WRITE(V5)
{
  DenNg = param.asInt();
  if (DenNg == 1){
    pcf8574.write(DEN_NGOAI, HIGH);
  } else {
    pcf8574.write(DEN_NGOAI, LOW);
  }
}

BLYNK_WRITE(V6)
{
  QuatG = param.asInt();
}

BLYNK_WRITE(V7)
{
  Nguong = param.asInt();
}
