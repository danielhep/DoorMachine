const char* host = "api.timezonedb.com";

void processNetwork() {
  WiFiClient client = server.available();
  if (client)
  {
    Serial.println("\n[Client connected]");
    unsigned long connectionTimeout = millis() + 6000;
    while (client.connected())
    {
      // read line by line what the client (web browser) is requesting
      if (client.available())
      {
        String line = client.readStringUntil('\r');
        Serial.print(line);
        // wait for end of client's request, that is marked with an empty line
        if (line.length() == 1 && line[0] == '\n')
        {
          client.print(prepareHtmlPage());
          break;
        }
      }
      if (millis() > connectionTimeout)
        break;
    }
    delay(1); // give the web browser time to receive the data

    // close the connection:
    client.stop();
    Serial.println("[Client disonnected]");
  }

  // Get the time every 5 seconds
  if (millis() > getTimeTimer)
  {
    Serial.println("Getting time...");
    getTime();
    getTimeTimer = millis() + 100000;
  }
}
/*
  void sendData() {
  WiFiClient client;
  Serial.print("[Sending data...");
  const char* host = "data.sparkfun.com";
  if (client.connect(host, 80)) {
    Serial.print("connected... ");
    client.print(String("GET /input/
                      }
                      }
*/
void getTime() {
  WiFiClient client;
  StaticJsonBuffer<200> jsonBuffer;
  char json[200];

  Serial.printf("\n[Connecting to % s ... ", host);
  if (client.connect(host, 80))
  {
    Serial.println("connected]");

    Serial.println("[Sending a request]");
    client.print(String("GET /v2/get-time-zone?key=WJW2Z4K0DWHF&format=json&by=zone&zone=PST&fields=timestamp") + " HTTP/1.1\r\n" +
                 "Host: " + host + "\r\n" 
                 "Content-Type: text/html" +
                 "Connection: close\r\n" +
                 "\r\n"
                );
    Serial.println("[Response:]");
    while (client.connected())
    {
      if (client.available())
      {
        String line = client.readStringUntil('\n');
        if (line[0] == '{') {
          Serial.println(line);
          line.toCharArray(json, 200);
        }
      }
    }
    client.stop();
    Serial.println("\n[Disconnected]");

    JsonObject& root = jsonBuffer.parseObject(json);
    if (!root.success()) {
      Serial.println("parseObject() failed");
      return;
    }

    Serial.println(root["timestamp"].as<long>());
    long time = root["timestamp"].as<long>();
    currentTime = time / 3600 % 24;
    if (currentTime <= 7 || currentTime >= 23)
      nightMode = true;
    else
      nightMode = false;
    setDefault(60000);
    Serial.println("Timestamp : " + String(time));
    Serial.println("Current time in hours : " + String(currentTime));
  }
  else
  {
    Serial.println("connection failed!]");
    client.stop();
  }
}

String prepareHtmlPage() {
  String htmlPage =
    String("HTTP/1.1 200 OK\r\n") +
    "Content-Type: text/html\r\n" +
    "Connection: close\r\n" +  // the connection will be closed after completion of the response
    //"Refresh: 5\r\n" +  // refresh the page automatically every 5 sec
    "\r\n" +
    " <!DOCTYPE HTML> " +
    "<html>" +
    "Peephole touch count:  " + String(peepholeTouchCount) + " <br /> " +
    "Night mode: " + String(nightMode) +
    " <br /> Edens North #148" +
    " </ html> " +
    "\r\n";
  return htmlPage;
}

