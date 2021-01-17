#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <ESPmDNS.h>


const char* ssid = "xxxx";
const char* password = "yyyy";

const char* webpage = "<!DOCTYPE html><head></head><body><script>\
        function worker() {\
            var obj = fetch('/scale').\
                then(res => res.json()).\
                then( (out) => {document.getElementById('gewicht').innerHTML= Math.round((out.val-650)/(2369-650)*1974) } ).\
                catch( (err) => console.log('Error:', err));\
            window.setTimeout( worker, 1000);\
        }\
        window.onload = function () {\
            worker();\
        }\
    </script>\
    <div id=\"gewicht\" style=\"font-size:50vw\">xx</div>\
</body>";

WebServer server(80);
 
char buf[100];

void handleRoot() {
  server.send(200, "text/html", webpage);
}

#define NUM 200
int16_t samples[NUM];

void handleScale() {
  int sum = 0;
  for (int i = 0; i < NUM; i++) {
    samples[i] = analogRead(35);
    sum += samples[i];
    delay(1);
  }
  float favg = float(sum) / NUM;
  int avg = sum/NUM;
  int sig = 0;
  for (int i = 0; i < NUM; i++) {
    sig += (samples[i]-avg)*(samples[i]-avg);
  }
  sig /= NUM;
  
  sprintf(buf, "{ \"val\": %f, \"sig\": %d }\n", favg, sig);

  Serial.print( buf);
  server.send(200, "application/json", buf);
}



void handleNotFound() {
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
}

void setup(void) {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  if (MDNS.begin("esp32")) {
    Serial.println("MDNS responder started");
  }

  server.on("/", handleRoot);
  server.on("/scale", handleScale);
  server.onNotFound(handleNotFound);

  server.begin();
  Serial.println("HTTP server started");
}

void loop(void) {
  server.handleClient();
}
