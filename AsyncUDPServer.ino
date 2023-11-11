#include "WiFi.h"
#include "AsyncUDP.h"
#include <ArduinoJson.h>


const char * ssid = "redes";
const char * password = "12345678";
long doorStatus = 0;

AsyncUDP udp;

void setup()
{
    Serial.begin(115200);
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    if (WiFi.waitForConnectResult() != WL_CONNECTED) {
        Serial.println("WiFi Failed");
        while(1) {
            delay(1000);
        }
    }
    if(udp.listen(1234)) {
        Serial.print("UDP Listening on IP: ");
        Serial.println(WiFi.localIP());
        udp.onPacket([](AsyncUDPPacket packet) {
            Serial.print("UDP Packet Type: ");
            Serial.print(packet.isBroadcast()?"Broadcast":packet.isMulticast()?"Multicast":"Unicast");
            Serial.print(", From: ");
            Serial.print(packet.remoteIP());
            Serial.print(":");
            Serial.print(packet.remotePort());
            Serial.print(", To: ");
            Serial.print(packet.localIP());
            Serial.print(":");
            Serial.print(packet.localPort());
            Serial.print(", Length: ");
            Serial.print(packet.length());
            Serial.print(", Data: ");
            Serial.write(packet.data(), packet.length());
           
            DynamicJsonDocument doc(1024);
            deserializeJson(doc, packet.data());

            int command = doc["command"] | - 1;
            if (command == -1){ 
                doorStatus = doc["system"]["door"]["status"];
                Serial.print(" Status: ");

                Serial.print(doorStatus);}

            Serial.println();
            //reply to the client
           
           if (command != -1){ 
             char responseStr[40];
             sprintf(responseStr, "{\"system\":{\"door\":{\"status\":%d}}}", doorStatus);
             packet.printf(responseStr);
           }
           else{
            packet.printf("{\"queue\":{\"status\":ok}}");
           }
        });
    }
}

void loop()
{
    delay(1000);
    //Send broadcast
    udp.broadcast("Anyone here?");
}
