
#include <Arduino.h>

#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>

#include <ESP8266HTTPClient.h>

#include <WiFiClient.h>


#include "DHTesp.h"

#ifdef ESP32
#pragma message(THIS EXAMPLE IS FOR ESP8266 ONLY!)
#error Select ESP8266 board.
#endif

DHTesp dht;

ESP8266WiFiMulti WiFiMulti;

WiFiServer server(3000);


//INITIALISATION
void setup() {

  Serial.begin(9600); //Création de la liaison série
  Serial.println("ready"); //Premier envoi
  pinMode(A0,INPUT);// déclaration de la pin qui reçoit le signal de la photorésistance

  Serial.println();
  Serial.println();
  Serial.println();

  
  String thisBoard= ARDUINO_BOARD;
  Serial.println(thisBoard);

  dht.setup(16, DHTesp::DHT22); // Connection du DHT à la pin GPIO 16

  for (uint8_t t = 4; t > 0; t--) {
    Serial.printf("[SETUP] WAIT %d...\n", t);
    Serial.flush();
    delay(1000);
  }


  //CONNEXION au wifi + démmarage du serveur qui va servir à la page web locale 
  WiFi.mode(WIFI_STA);
  WiFiMulti.addAP("SFR_8450_5GHZ", "hir68xifj8bpc93uhaz6");
  Serial.println("");
  Serial.println("WiFi connecte.");
  Serial.println("Adresse IP : ");
  Serial.println(WiFi.localIP());
  server.begin();

}







void loop() {
  delay(dht.getMinimumSamplingPeriod());

  //DONNÉES DES CAPTEURS
  //Récupération des valeurs du capteur DHT 22 : humidité et température
  float humidity = dht.getHumidity();
  float temperature = dht.getTemperature();
  //Récupération de la valeur dez la photorésistance
  int lum = analogRead(A0);

  //affichage des valeurs
  Serial.println(dht.getStatusString());
  Serial.print("Humidity: ");
  Serial.println(humidity, 1);
  Serial.print("Temperature: ");
  Serial.println(temperature, 1);
  Serial.print("Luminosite: ");
  Serial.println(lum, 1);
  Serial.print(" ");

  
  delay(2000);

  //Conversion des valeurs en chaine de caractères pour les faire passer par protocole HTTP pouir la BDD
  String lumString;
  lumString = String(lum);
  String tempString;
  tempString = String(temperature);
  String humString;
  humString = String(humidity);
  String url1 = tempString + "&humidite=" + humString + "&luminosite=" + lumString;

  
  // Attendre la connexion WiFi
  if ((WiFiMulti.run() == WL_CONNECTED)) {

    WiFiClient client;

    HTTPClient http;

    Serial.print("[HTTP] begin...\n");
    if (http.begin(client, "http://192.168.1.92/IoT/wifi/data.php?temperature="+ url1)) {  // HTTP


      Serial.print("[HTTP] GET...\n");
      // Début de la connexion et envoie de la requète HTTP
      int httpCode = http.GET();

      // vérification et detection d'erreur 
      if (httpCode > 0) {
        // HTTP header has been send and Server response header has been handled
        Serial.printf("[HTTP] GET... code: %d\n", httpCode);

        // file found at server
        if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
          String payload = http.getString();
          Serial.println(payload);
        }
      } else {
        Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
      }

      http.end();
    } else {
      Serial.printf("[HTTP} Unable to connect\n");
    }
  }

  
  //Wifi pour la page utilisateur
  WiFiClient client = server.available();   // regarder si des clients sont connectés

  if (client) {                             // S'il y a un client,
    Serial.println("New Client.");           //Afficher un message en console 
    String currentLine = "";                
    while (client.connected()) {            
      if (client.available()) {             
        char c = client.read();             
        Serial.write(c);                    
        if (c == '\n') {                    

          if (currentLine.length() == 0) {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println();

            // Affichage sur la page 
            client.print("Temperature : ");
            client.println(tempString);
            client.print("Humindite : ");
            client.println(humString);
            client.print("Luminosite : ");
            client.println(lumString);
            // The HTTP response ends with another blank line:
            client.println();

            
            //Notifications
            if (lum < 200){
              client.print("Il est temps de mettre les plantes au soleil !");
            }
            else if (temperature > 25){
              client.print("Pensez à arroser vos plantes ! ");
            }
            else if (humidity < 40 ){
              client.print("Pensez à arroser vos plantes ! ");
            }
            else if (temperature < 5 ){
              client.print("vos plantes sont en danger rentrez les vite au chaud ! ");
            }

            
            // break out of the while loop:
            break;
          } else {   
            currentLine = "";
          }
        } else if (c != '\r') { 
          currentLine += c;     
        }
      }
    }
    // close the connection:
    client.stop();
    Serial.println("Client Disconnected.");
  }

  delay(10000);
}
