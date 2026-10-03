/*
 Smart RGB Lamp — Marcin / Maker Portfolio
 REAL GEOMETRY: 137 LEDs on ONE data line
 LED 0..99 = BODY (100 LEDs)
 LED 100..136 = PLUME / "pióropusz" (37 LEDs)

 Replace credentials before upload.
 Libraries: Adafruit NeoPixel, Adafruit GFX, Adafruit SSD1306, SinricPro
*/
#include <Arduino.h>
#include <map>
#if defined(ESP8266)
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <ArduinoOTA.h>
ESP8266WebServer server(80);
#elif defined(ESP32)
#include <WiFi.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <ArduinoOTA.h>
WebServer server(80);
#endif
#include <SinricPro.h>
#include <SinricProLight.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_NeoPixel.h>

#define I2C_SDA_PIN D5
#define I2C_SCL_PIN D6
#define NEOPIXEL_PIN D2
#define BODY_START 0
#define BODY_COUNT 100
#define PLUME_START 100
#define PLUME_COUNT 37
#define NUMPIXELS 137
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define BAUD_RATE 115200

#define WIFI_SSID "YOUR_WIFI_NAME"
#define WIFI_PASS "YOUR_WIFI_PASSWORD"
#define APP_KEY "YOUR_SINRIC_APP_KEY"
#define APP_SECRET "YOUR_SINRIC_APP_SECRET"
#define LIGHT_ID "YOUR_SINRIC_LIGHT_ID"

static const char* MDNS_NAME="lampa";
static const char* OTA_PASS="CHANGE_ME_OTA";

struct Color{uint8_t r,g,b;};
enum EffectMode:uint8_t{STATIC_FX,COMET_FX,RAINBOW_FX,SPARKLE_FX,DUAL_FX,BREATHE_FX,FIRE_FX,AURORA_FX};
enum PlumeMode:uint8_t{PLUME_SYNC,PLUME_COMPLEMENT,PLUME_PULSE,PLUME_SPARK};

struct State{
 bool power=false;
 Color color={214,225,255};
 int brightness=85;
 EffectMode effect=DUAL_FX;
 PlumeMode plume=PLUME_PULSE;
 uint8_t bodyBrightness=100,plumeBrightness=80,tail=18;
 uint16_t cometMs=25,rainbowMs=20,sparkleMs=45;
 bool dirty=true,displayDirty=true;
}st;

Adafruit_SSD1306 display(SCREEN_WIDTH,SCREEN_HEIGHT,&Wire,OLED_RESET);
Adafruit_NeoPixel pixels(NUMPIXELS,NEOPIXEL_PIN,NEO_GRB+NEO_KHZ800);
SinricProLight& myLight=SinricPro[LIGHT_ID];

static uint8_t scale8u(uint8_t v,uint8_t s){return (uint16_t)v*s/255;}
static uint8_t pct(uint8_t p){return (uint16_t)constrain(p,0,100)*255/100;}
static uint8_t secScale(uint8_t section){return scale8u(pct(st.brightness),pct(section));}
static uint32_t rgb(Color c,uint8_t s){return pixels.Color(scale8u(c.r,s),scale8u(c.g,s),scale8u(c.b,s));}
static Color comp(Color c){return {(uint8_t)(255-c.r),(uint8_t)(255-c.g),(uint8_t)(255-c.b)};}
static void fillRange(uint16_t start,uint16_t count,uint32_t c){for(uint16_t i=0;i<count;i++)pixels.setPixelColor(start+i,c);}
static void allOff(){pixels.clear();pixels.show();}
static uint32_t rainbowColor(uint16_t h,uint8_t s){
 uint32_t c=pixels.gamma32(pixels.ColorHSV(h));
 return pixels.Color(scale8u((uint8_t)(c>>16),s),scale8u((uint8_t)(c>>8),s),scale8u((uint8_t)c,s));
}
static void clampState(){
 st.brightness=constrain(st.brightness,0,100);
 st.bodyBrightness=constrain(st.bodyBrightness,0,100);
 st.plumeBrightness=constrain(st.plumeBrightness,0,100);
 st.tail=constrain(st.tail,1,50);
 st.cometMs=constrain(st.cometMs,5,200);
 st.rainbowMs=constrain(st.rainbowMs,5,200);
 st.sparkleMs=constrain(st.sparkleMs,10,250);
 if((int)st.effect<0||(int)st.effect>7)st.effect=DUAL_FX;
 if((int)st.plume<0||(int)st.plume>3)st.plume=PLUME_PULSE;
}

void renderPlume(uint32_t now,uint16_t hue){
 uint8_t ps=secScale(st.plumeBrightness);
 if(st.plume==PLUME_SYNC){fillRange(PLUME_START,PLUME_COUNT,rgb(st.color,ps));return;}
 if(st.plume==PLUME_COMPLEMENT){fillRange(PLUME_START,PLUME_COUNT,rgb(comp(st.color),ps));return;}
 if(st.plume==PLUME_PULSE){
   float x=(now%2600UL)/2600.0f;
   uint8_t pulse=50+(uint8_t)(205.0f*(0.5f-0.5f*cosf(x*2.0f*PI)));
   for(uint16_t i=0;i<PLUME_COUNT;i++){
     uint8_t spatial=150+(uint16_t)105*i/(PLUME_COUNT-1);
     pixels.setPixelColor(PLUME_START+i,rgb(st.color,scale8u(scale8u(ps,pulse),spatial)));
   }
   return;
 }
 fillRange(PLUME_START,PLUME_COUNT,rgb(st.color,scale8u(ps,80)));
 for(uint8_t i=0;i<4;i++)pixels.setPixelColor(PLUME_START+random(PLUME_COUNT),rgb(st.color,ps));
}

void render(){
 if(!st.power)return;
 static uint32_t last=0;
 static int16_t head=0;
 static int8_t dir=1;
 static uint16_t hue=0;
 uint32_t now=millis();
 uint8_t bs=secScale(st.bodyBrightness);

 switch(st.effect){
  case STATIC_FX:
   if(!st.dirty)return;
   fillRange(BODY_START,BODY_COUNT,rgb(st.color,bs));
   renderPlume(now,hue);pixels.show();st.dirty=false;break;

  case COMET_FX:
  case DUAL_FX:
   if(now-last<st.cometMs)return; last=now;
   for(uint16_t i=0;i<BODY_COUNT;i++){
     uint32_t c=pixels.getPixelColor(i);
     pixels.setPixelColor(i,(uint16_t)((c>>16)&255)*220/255,(uint16_t)((c>>8)&255)*220/255,(uint16_t)(c&255)*220/255);
   }
   if(st.effect==DUAL_FX){
     uint32_t bg=rgb(st.color,scale8u(bs,60));
     for(uint16_t i=0;i<BODY_COUNT;i++)if(pixels.getPixelColor(i)==0)pixels.setPixelColor(i,bg);
   }
   pixels.setPixelColor(head,rgb(st.color,bs));
   for(uint8_t t=1;t<=st.tail;t++){
     int16_t idx=head-dir*t;if(idx<0||idx>=BODY_COUNT)continue;
     uint8_t s=(uint16_t)(st.tail-t)*255/max(1,(int)st.tail);
     pixels.setPixelColor(idx,rgb(st.color,scale8u(bs,s)));
   }
   head+=dir;if(head>=BODY_COUNT){head=BODY_COUNT-1;dir=-1;}if(head<0){head=0;dir=1;}
   renderPlume(now,hue);pixels.show();break;

  case RAINBOW_FX:
   if(now-last<st.rainbowMs)return;last=now;hue+=220;
   for(uint16_t i=0;i<BODY_COUNT;i++)pixels.setPixelColor(i,rainbowColor(hue+(uint32_t)i*65535UL/BODY_COUNT,bs));
   renderPlume(now,hue+32768);pixels.show();break;

  case SPARKLE_FX:
   if(now-last<st.sparkleMs)return;last=now;
   for(uint16_t i=0;i<BODY_COUNT;i++){
     uint32_t c=pixels.getPixelColor(i);
     pixels.setPixelColor(i,(uint16_t)((c>>16)&255)*235/255,(uint16_t)((c>>8)&255)*235/255,(uint16_t)(c&255)*235/255);
   }
   for(uint8_t i=0;i<7;i++)pixels.setPixelColor(random(BODY_COUNT),rgb(st.color,scale8u(bs,120+random(136))));
   renderPlume(now,hue);pixels.show();break;

  case BREATHE_FX:{
   if(now-last<18)return;last=now;
   float x=(now%3600UL)/3600.0f;
   uint8_t b=25+(uint8_t)(230.0f*(0.5f-0.5f*cosf(x*2.0f*PI)));
   fillRange(BODY_START,BODY_COUNT,rgb(st.color,scale8u(bs,b)));
   renderPlume(now,hue);pixels.show();break;
  }

  case FIRE_FX:{
   if(now-last<35)return;last=now;
   for(uint16_t i=0;i<BODY_COUNT;i++){
     uint8_t flick=150+random(106),height=80+(uint16_t)i*175/(BODY_COUNT-1);
     Color f={255,(uint8_t)(35+random(125)),(uint8_t)random(20)};
     pixels.setPixelColor(i,rgb(f,scale8u(bs,scale8u(flick,height))));
   }
   uint8_t ps=secScale(st.plumeBrightness);
   for(uint16_t i=0;i<PLUME_COUNT;i++){
     Color f={255,(uint8_t)(20+random(100)),0};
     pixels.setPixelColor(PLUME_START+i,rgb(f,scale8u(ps,140+random(116))));
   }
   pixels.show();break;
  }

  case AURORA_FX:
   if(now-last<24)return;last=now;hue+=90;
   for(uint16_t i=0;i<BODY_COUNT;i++){
     float x=(float)i/BODY_COUNT,t=now/1200.0f;
     float w=(sinf(x*12+t)+sinf(x*5-t*.7f)+2)/4;
     pixels.setPixelColor(i,rainbowColor((uint16_t)(26000+15000*w+hue),scale8u(bs,100+(uint8_t)(155*w))));
   }
   renderPlume(now,hue+10000);pixels.show();break;
 }
}

bool onPowerState(const String&,bool& v){st.power=v;st.dirty=st.displayDirty=true;if(!v)allOff();return true;}
bool onBrightness(const String&,int& v){st.brightness=constrain(v,0,100);st.dirty=st.displayDirty=true;return true;}
bool onAdjustBrightness(const String&,int& d){st.brightness=constrain(st.brightness+d,0,100);d=st.brightness;st.dirty=st.displayDirty=true;return true;}
bool onColor(const String&,byte& r,byte& g,byte& b){st.color={r,g,b};st.dirty=st.displayDirty=true;return true;}

static int getInt(const String& body,const char* key,int def){
 String k=String("\"")+key+"\":";int p=body.indexOf(k);if(p<0)return def;p+=k.length();
 while(p<(int)body.length()&&body[p]==' ')p++;bool neg=false;if(p<(int)body.length()&&body[p]=='-'){neg=true;p++;}
 long v=0;bool ok=false;while(p<(int)body.length()&&isDigit(body[p])){ok=true;v=v*10+body[p++]-'0';}
 return ok?(neg?-v:v):def;
}
static bool getBool(const String& body,const char* key,bool def){
 String k=String("\"")+key+"\":";int p=body.indexOf(k);if(p<0)return def;p+=k.length();
 while(p<(int)body.length()&&body[p]==' ')p++;
 if(body.startsWith("true",p))return true;if(body.startsWith("false",p))return false;return def;
}
static String stateJson(){
 String ip=WiFi.status()==WL_CONNECTED?WiFi.localIP().toString():"0.0.0.0";
 String s="{";
 s+="\"power\":"+String(st.power?"true":"false")+",";
 s+="\"mode\":"+String((int)st.effect)+",\"plumeMode\":"+String((int)st.plume)+",";
 s+="\"brightness\":"+String(st.brightness)+",\"bodyBrightness\":"+String(st.bodyBrightness)+",\"plumeBrightness\":"+String(st.plumeBrightness)+",";
 s+="\"r\":"+String(st.color.r)+",\"g\":"+String(st.color.g)+",\"b\":"+String(st.color.b)+",";
 s+="\"tail\":"+String(st.tail)+",\"cometSpd\":"+String(st.cometMs)+",\"rainbowSpd\":"+String(st.rainbowMs)+",\"sparkleSpd\":"+String(st.sparkleMs)+",";
 s+="\"ip\":\""+ip+"\",\"wifi\":\""+String(WiFi.status()==WL_CONNECTED?"OK":"DOWN")+"\",\"mdns\":\"lampa.local\"}";
 return s;
}
static void sendJson(const String& s){server.sendHeader("Cache-Control","no-store");server.send(200,"application/json",s);}

static const char PAGE[] PROGMEM=R"HTML(
<!doctype html><html><head><meta charset="utf-8"><meta name="viewport" content="width=device-width,initial-scale=1">
<title>Smart Lamp</title><style>
body{font-family:system-ui;margin:0;background:#070912;color:#eef5ff}.w{max-width:850px;margin:auto;padding:18px}.c{background:#111624;border:1px solid #26324a;border-radius:16px;padding:15px;margin:12px 0}.g{display:grid;grid-template-columns:1fr 1fr;gap:12px}@media(max-width:650px){.g{grid-template-columns:1fr}}button,select,input[type=color]{padding:10px;border-radius:10px;background:#0b1020;color:#fff;border:1px solid #33415e}button{cursor:pointer}button.on{background:linear-gradient(90deg,#68e5ff,#b48cff);color:#08101b;font-weight:800}input[type=range]{width:100%}.r{display:flex;gap:8px;flex-wrap:wrap}.small{color:#91a4c4;font-size:12px}label{display:block;margin:7px 0}
</style></head><body><div class="w"><h1>Smart RGB Lamp</h1><p class="small">100 LED body + 37 LED plume • ONE strip • OLED • OTA • Alexa/Sinric</p>
<div class="c"><div id="info">Connecting…</div><div class="r"><button class="on" onclick="power(1)">ON</button><button onclick="power(0)">OFF</button></div></div>
<div class="c"><div class="r"><button onclick="preset(4,2,'#00d9ff')">WOW</button><button onclick="preset(6,3,'#ff3b00')">FIRE</button><button onclick="preset(7,2,'#44ffcc')">AURORA</button><button onclick="preset(5,2,'#ffb46b')">CALM</button><button onclick="preset(2,3,'#ff00cc')">PARTY</button></div></div>
<div class="c g"><div><label>Main effect</label><select id="m" onchange="send()"><option value="0">STATIC</option><option value="1">COMET</option><option value="2">RAINBOW</option><option value="3">SPARKLE</option><option value="4">DUAL</option><option value="5">BREATHE</option><option value="6">FIRE</option><option value="7">AURORA</option></select></div>
<div><label>Plume</label><select id="pm" onchange="send()"><option value="0">SYNC</option><option value="1">COMPLEMENT</option><option value="2">PULSE</option><option value="3">SPARK</option></select></div><div><label>Color</label><input id="col" type="color" value="#d6e1ff" onchange="send()"></div></div>
<div class="c"><label>Global <b id="vb"></b></label><input id="b" type="range" min="0" max="100" oninput="deb()">
<label>BODY 0–99 <b id="vbb"></b></label><input id="bb" type="range" min="0" max="100" oninput="deb()">
<label>PLUME 100–136 <b id="vpb"></b></label><input id="pb" type="range" min="0" max="100" oninput="deb()"></div>
</div><script>
const $=x=>document.getElementById(x);let st={},t;const hex=n=>n.toString(16).padStart(2,'0'),rgb=(r,g,b)=>'#'+hex(r)+hex(g)+hex(b);function c2r(h){h=h.slice(1);return{r:parseInt(h.slice(0,2),16),g:parseInt(h.slice(2,4),16),b:parseInt(h.slice(4,6),16)}}function vals(){vb.textContent=b.value+'%';vbb.textContent=bb.value+'%';vpb.textContent=pb.value+'%'}function deb(){vals();clearTimeout(t);t=setTimeout(send,120)}
async function load(){let r=await fetch('/api/state',{cache:'no-store'}),s=await r.json();st=s;info.textContent=`${s.wifi} • ${s.ip} • ${s.mdns}`;m.value=s.mode;pm.value=s.plumeMode;b.value=s.brightness;bb.value=s.bodyBrightness;pb.value=s.plumeBrightness;col.value=rgb(s.r,s.g,s.b);vals()}
async function power(x){await fetch('/api/power?on='+x);await load()}async function send(){let c=c2r(col.value),p={power:st.power,mode:+m.value,plumeMode:+pm.value,brightness:+b.value,bodyBrightness:+bb.value,plumeBrightness:+pb.value,r:c.r,g:c.g,b:c.b};let r=await fetch('/api/set',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify(p)});st=await r.json()}
function preset(a,p,c){m.value=a;pm.value=p;col.value=c;b.value=100;bb.value=100;pb.value=90;st.power=true;power(1).then(send)}load();setInterval(load,3000);
</script></body></html>)HTML";

void setupWiFi(){
#if defined(ESP8266)
 WiFi.mode(WIFI_STA);WiFi.hostname(MDNS_NAME);WiFi.setSleepMode(WIFI_NONE_SLEEP);WiFi.setAutoReconnect(true);
#else
 WiFi.mode(WIFI_STA);WiFi.setSleep(false);WiFi.setAutoReconnect(true);WiFi.setHostname(MDNS_NAME);
#endif
 WiFi.begin(WIFI_SSID,WIFI_PASS);uint32_t t=millis();while(WiFi.status()!=WL_CONNECTED&&millis()-t<20000){delay(200);yield();}
}
void setupWeb(){
 server.on("/",HTTP_GET,[]{server.send_P(200,"text/html; charset=utf-8",PAGE);});
 server.on("/api/state",HTTP_GET,[]{sendJson(stateJson());});
 server.on("/api/power",HTTP_GET,[]{st.power=server.hasArg("on")&&server.arg("on")=="1";st.dirty=st.displayDirty=true;if(!st.power)allOff();sendJson(stateJson());});
 server.on("/api/set",HTTP_POST,[]{
   String b=server.arg("plain");st.power=getBool(b,"power",st.power);st.effect=(EffectMode)getInt(b,"mode",st.effect);st.plume=(PlumeMode)getInt(b,"plumeMode",st.plume);
   st.brightness=getInt(b,"brightness",st.brightness);st.bodyBrightness=getInt(b,"bodyBrightness",st.bodyBrightness);st.plumeBrightness=getInt(b,"plumeBrightness",st.plumeBrightness);
   st.color.r=getInt(b,"r",st.color.r);st.color.g=getInt(b,"g",st.color.g);st.color.b=getInt(b,"b",st.color.b);
   st.tail=getInt(b,"tail",st.tail);st.cometMs=getInt(b,"cometSpd",st.cometMs);st.rainbowMs=getInt(b,"rainbowSpd",st.rainbowMs);st.sparkleMs=getInt(b,"sparkleSpd",st.sparkleMs);
   clampState();st.dirty=st.displayDirty=true;if(!st.power)allOff();sendJson(stateJson());
 });server.begin();
}
void setupServices(){
 if(WiFi.status()==WL_CONNECTED){if(MDNS.begin(MDNS_NAME))MDNS.addService("http","tcp",80);ArduinoOTA.setHostname(MDNS_NAME);ArduinoOTA.setPassword(OTA_PASS);ArduinoOTA.begin();}
 myLight.onPowerState(onPowerState);myLight.onBrightness(onBrightness);myLight.onAdjustBrightness(onAdjustBrightness);myLight.onColor(onColor);SinricPro.begin(APP_KEY,APP_SECRET);
}
void oled(){
 static uint32_t last=0;if(!st.displayDirty&&millis()-last<500)return;last=millis();st.displayDirty=false;
 display.clearDisplay();display.setTextColor(SSD1306_WHITE);display.setTextSize(1);display.setCursor(0,0);
 display.println("SMART RGB LAMP");display.print("WiFi: ");display.println(WiFi.status()==WL_CONNECTED?"OK":"DOWN");
 display.print("IP: ");display.println(WiFi.status()==WL_CONNECTED?WiFi.localIP().toString():"0.0.0.0");
 display.print("Power: ");display.println(st.power?"ON":"OFF");display.print("Body:100 Plume:37");display.println();display.print("Brightness: ");display.print(st.brightness);display.println("%");display.display();
}
void bootWow(){
 for(uint16_t i=0;i<BODY_COUNT;i+=4){pixels.clear();for(uint16_t k=0;k<=i;k++)pixels.setPixelColor(k,pixels.Color(0,80,130));pixels.show();delay(7);}
 for(uint16_t i=0;i<PLUME_COUNT;i++){pixels.setPixelColor(PLUME_START+i,pixels.Color(90,0,150));pixels.show();delay(5);}allOff();
}
void setup(){
 Serial.begin(BAUD_RATE);Wire.begin(I2C_SDA_PIN,I2C_SCL_PIN);display.begin(SSD1306_SWITCHCAPVCC,0x3C);
 pixels.begin();pixels.clear();pixels.show();randomSeed(micros());bootWow();setupWiFi();setupWeb();setupServices();oled();
}
void loop(){
 server.handleClient();
#if defined(ESP8266)
 MDNS.update();
#endif
 ArduinoOTA.handle();SinricPro.handle();render();oled();yield();
}
