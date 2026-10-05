/* KORA Battery Meter v0.1 — ESP32-C3 + ILI9341 + INA226.
   Hardware validation pending. Monitor only: no load control or automatic cutoff.
   Set SHUNT_OHMS to the actual module shunt value before measuring current.
   TFT pins preserved from Marcin's C3 weather station. I2C 0/1 are NEW wiring.
   Positive current = discharge (battery+ -> IN+ -> IN- -> load+).
*/
#include <SPI.h>
#include <Wire.h>
#include <WiFi.h>
#include <WebServer.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <math.h>
constexpr int TFT_CS=10,TFT_RST=5,TFT_DC=4,TFT_MOSI=7,TFT_SCK=6,TFT_MISO=2;
constexpr int I2C_SDA=0,I2C_SCL=1,BACK_SW=8;
constexpr float SHUNT_OHMS=0.0f; // REQUIRED: e.g. R100=0.1 ohm; R010=0.01 ohm
constexpr uint8_t INA_ADDR=0x40;
Adafruit_ILI9341 tft(TFT_CS,TFT_DC,TFT_RST);
WebServer server(80);
float voltage=0,current=0,powerW=0; double outMAh=0,outWh=0,inMAh=0,inWh=0;
bool valid=false,havePrevious=false; float prevI=0,prevP=0;
uint32_t lastSample=0,lastDraw=0,lastGood=0,lastButton=0;
float history[100]={}; int historyHead=0,historyCount=0; String command;
const uint16_t BG=0x0842,PANEL=0x10C4,MINT=0x6F18,WHITE=0xFFFF,MUTED=0x9CF3;
bool writeReg(uint8_t reg,uint16_t v){Wire.beginTransmission(INA_ADDR);Wire.write(reg);Wire.write(v>>8);Wire.write(v&255);return Wire.endTransmission()==0;}
bool readReg(uint8_t reg,uint16_t &v){Wire.beginTransmission(INA_ADDR);Wire.write(reg);if(Wire.endTransmission(false)!=0)return false;if(Wire.requestFrom(INA_ADDR,(uint8_t)2)!=2)return false;v=(Wire.read()<<8)|Wire.read();return true;}
void resetSession(){outMAh=outWh=inMAh=inWh=0;historyHead=historyCount=0;havePrevious=false;}
void sample(){
 uint32_t now=millis();uint16_t bus,shunt,id;
 valid=readReg(0xFE,id)&&id==0x5449&&readReg(2,bus)&&readReg(1,shunt);
 if(!valid){havePrevious=false;return;}
 voltage=bus*0.00125f;
 if(SHUNT_OHMS>0){
  current=(int16_t)shunt*0.0000025f/SHUNT_OHMS;
  // Bus voltage is measured at IN-; include shunt drop for battery-terminal voltage.
  voltage+=(int16_t)shunt*0.0000025f; powerW=voltage*current;
  bool saturated=(int16_t)shunt>=32760||(int16_t)shunt<=-32760;
  if(saturated){valid=false;havePrevious=false;return;}
  if(havePrevious && now-lastGood<=1000){
   double hours=(now-lastGood)/3600000.0;
   outMAh+=(fmaxf(prevI,0)+fmaxf(current,0))*0.5*hours*1000;
   inMAh+=(fmaxf(-prevI,0)+fmaxf(-current,0))*0.5*hours*1000;
   outWh+=(fmaxf(prevP,0)+fmaxf(powerW,0))*0.5*hours;
   inWh+=(fmaxf(-prevP,0)+fmaxf(-powerW,0))*0.5*hours;
  }
  prevI=current;prevP=powerW;havePrevious=true;lastGood=now;
 }
 history[historyHead]=voltage;historyHead=(historyHead+1)%100;if(historyCount<100)historyCount++;
}
void label(int x,int y,const char* s,uint16_t c,int size=1){tft.setCursor(x,y);tft.setTextSize(size);tft.setTextColor(c);tft.print(s);}
void metric(int y,const char* name,float value,const char* unit,bool ok){
 tft.fillRect(12,y,216,40,PANEL);label(20,y+5,name,MUTED);tft.setCursor(20,y+18);tft.setTextSize(2);tft.setTextColor(WHITE);if(ok)tft.print(value,3);else tft.print("---");tft.print(" ");tft.print(unit);
}
void draw(){
 label(12,12,"KORA / ENERGY",MINT,2);
 tft.fillRect(12,36,220,16,BG);label(12,36,!valid?"SENSOR ERROR":SHUNT_OHMS<=0?"SET SHUNT_OHMS FIRST":current>0.005?"DISCHARGING":current< -0.005?"CHARGING":"IDLE",valid?MINT:0xF800);
 metric(56,"VOLTAGE",voltage,"V",valid);metric(100,"CURRENT",current,"A",valid&&SHUNT_OHMS>0);metric(144,"POWER",powerW,"W",valid&&SHUNT_OHMS>0);
 tft.fillRect(12,190,220,32,BG);char b[50];snprintf(b,sizeof b,"OUT %.1f mAh / %.3f Wh",outMAh,outWh);label(12,190,b,MINT);snprintf(b,sizeof b,"IN  %.1f mAh / %.3f Wh",inMAh,inWh);label(12,205,b,MUTED);
 tft.fillRect(12,228,216,54,PANEL);
 if(historyCount>1){float lo=history[(historyHead-historyCount+100)%100],hi=lo;for(int j=0;j<historyCount;j++){float v=history[(historyHead-historyCount+j+100)%100];lo=fminf(lo,v);hi=fmaxf(hi,v);}if(hi-lo<0.05f){lo-=0.025f;hi+=0.025f;}int px=14,py=0;for(int j=0;j<historyCount;j++){int x=14+j*212/99;int y=278-(history[(historyHead-historyCount+j+100)%100]-lo)/(hi-lo)*46;if(j)tft.drawLine(px,py,x,y,MINT);px=x;py=y;}}
 label(12,288,"WiFi: KORA-Meter",MUTED);label(12,302,"192.168.4.1 / hold BACK reset",MUTED);
}
const char PAGE[] PROGMEM=R"HTML(<!doctype html><html lang="en"><meta name="viewport" content="width=device-width,initial-scale=1"><title>KORA Energy</title><style>body{background:#080e17;color:#edf7ff;font:18px system-ui;max-width:600px;margin:30px auto;padding:20px}h1{color:#6be0c1}article{background:#142131;padding:22px;margin:12px 0;border-radius:18px}b{font-size:34px}button{padding:15px;border-radius:12px}small{color:#aabccc}</style><h1>KORA / ENERGY</h1><small>Live measurements. Session totals, not battery percentage.</small><article id="data">Connecting...</article><button onclick="if(confirm('Reset session totals?'))fetch('/reset',{method:'POST'})">Reset session</button><p><a style="color:#6be0c1" href="/csv">Download current totals CSV</a></p><script>async function update(){try{let r=await fetch('/api');if(!r.ok)throw Error();let d=await r.json();document.getElementById('data').innerHTML=!d.valid?'Sensor unavailable':`<b>${d.v.toFixed(3)} V</b><p>${d.i===null?'Configure shunt resistor in sketch':d.i.toFixed(3)+' A / '+d.w.toFixed(3)+' W'}</p><p>OUT ${d.out_mAh.toFixed(1)} mAh / ${d.out_Wh.toFixed(3)} Wh</p><p>IN ${d.in_mAh.toFixed(1)} mAh / ${d.in_Wh.toFixed(3)} Wh</p>`}catch(e){document.getElementById('data').textContent='Connection lost'}}setInterval(update,500);update();</script></html>)HTML";
void setup(){
 Serial.begin(115200);pinMode(BACK_SW,INPUT_PULLUP);
 SPI.begin(TFT_SCK,TFT_MISO,TFT_MOSI,TFT_CS);tft.begin();tft.setRotation(0);tft.fillScreen(BG);tft.setTextWrap(false);
 Wire.begin(I2C_SDA,I2C_SCL);Wire.setTimeOut(30);writeReg(0,0x4527); // 16-sample averaging, continuous bus + shunt
 WiFi.mode(WIFI_AP);WiFi.softAP("KORA-Meter","KoraMeter1");
 server.on("/",HTTP_GET,[]{server.send_P(200,"text/html",PAGE);});
 server.on("/api",HTTP_GET,[]{char b[360];String i=valid&&SHUNT_OHMS>0?String(current,6):"null",w=valid&&SHUNT_OHMS>0?String(powerW,6):"null";snprintf(b,sizeof b,"{\"valid\":%s,\"v\":%.5f,\"i\":%s,\"w\":%s,\"out_mAh\":%.5f,\"out_Wh\":%.6f,\"in_mAh\":%.5f,\"in_Wh\":%.6f}",valid?"true":"false",voltage,i.c_str(),w.c_str(),outMAh,outWh,inMAh,inWh);server.send(200,"application/json",b);});
 server.on("/reset",HTTP_POST,[]{resetSession();server.send(200,"text/plain","Reset");});
 server.on("/csv",HTTP_GET,[]{server.sendHeader("Content-Disposition","attachment; filename=kora-session.csv");String s="valid,voltage_V,current_A,power_W,out_mAh,out_Wh,in_mAh,in_Wh\n";s+=String(valid?1:0)+","+(valid?String(voltage,5):"")+","+(valid&&SHUNT_OHMS>0?String(current,6):"")+","+(valid&&SHUNT_OHMS>0?String(powerW,6):"")+","+String(outMAh,5)+","+String(outWh,6)+","+String(inMAh,5)+","+String(inWh,6)+"\n";server.send(200,"text/csv",s);});server.begin();
}
void loop(){
 server.handleClient();uint32_t now=millis();if(now-lastSample>=200){lastSample=now;sample();}if(now-lastDraw>=500){lastDraw=now;draw();}
 static bool held=false,done=false;if(digitalRead(BACK_SW)==LOW){if(!held){held=true;done=false;lastButton=now;}if(!done&&now-lastButton>1500){resetSession();done=true;}}else held=false;
 while(Serial.available()){char c=Serial.read();if(c=='\n'){command.trim();if(command=="RESET")resetSession();command="";}else if(command.length()<40)command+=c;}
}
