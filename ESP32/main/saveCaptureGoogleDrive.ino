//https://github.com/gsampallo/esp32cam-gdrive


void saveCapturedImageGithub(){
    /* Need to package: 
    curl   -X PUT   -H "Accept: application/vnd.github+json"   \
    -H "Authorization: Bearer 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"  \
    https://api.github.com/repos/anglerfishbot/AnglerfishGallery/contents/test3   -\
    d '{"message":"my commit message","committer":{"name":"anglerfishbot","email":"benedictdied@gmail.com"},"content":"bXkgbmV3IGZpbGUgY29udGVudHM="}' 
    */

    
}

void saveCapturedImageGDrive() {
  Serial.println("Connect to " + String(myDomain));
  Serial.println("Connect to " + String(myDomain));
  WiFiClientSecure clientSecure;
  clientSecure.setInsecure();   //run version 1.0.5 or above

  if (clientSecure.connect(myDomain, 443)) {
    Serial.println("Connection successful");

    camera_fb_t * fb = NULL;
    fb = esp_camera_fb_get();
    if (!fb) {
      Serial.println("Camera capture failed");
      delay(1000);
      ESP.restart();
      return;
    }

    char *input = (char *)fb->buf;
    char output[base64_enc_len(3)];
    String imageFile = "";
    for (int i = 0; i < fb->len; i++) {
      base64_encode(output, (input++), 3);
      if (i % 3 == 0) imageFile += urlencode(String(output));
    }
    String Data = myFilename + mimeType + myImage;

    esp_camera_fb_return(fb);

    Serial.println("Send a captured image to Google Drive.");

    clientSecure.println("POST " + myScript + " HTTP/1.1");
    clientSecure.println("Host: " + String(myDomain));
    clientSecure.println("Content-Length: " + String(Data.length() + imageFile.length()));
    clientSecure.println("Content-Type: application/x-www-form-urlencoded");
    clientSecure.println();

    clientSecure.print(Data);
    int Index;
    for (Index = 0; Index < imageFile.length(); Index = Index + 1000) {
      clientSecure.print(imageFile.substring(Index, Index + 1000));
    }

    Serial.println("Waiting for response.");
    long int StartTime = millis();
    while (!clientSecure.available()) {
      Serial.print(".");
      delay(100);
      if ((StartTime + waitingTime) < millis()) {
        Serial.println();
        Serial.println("No response.");
        //If you have no response, maybe need a greater value of waitingTime
        break;
      }
    }
    Serial.println();
    while (clientSecure.available()) {
      Serial.print(char(clientSecure.read()));
    }
  }
  else {
    Serial.println("Connection to " + String(myDomain) + " failed.");
  }
  clientSecure.stop();
}

//https://github.com/zenmanenergy/ESP8266-Arduino-Examples/
String urlencode(String str)
{
  String encodedString = "";
  char c;
  char code0;
  char code1;
  char code2;
  for (int i = 0; i < str.length(); i++) {
    c = str.charAt(i);
    if (c == ' ') {
      encodedString += '+';
    } else if (isalnum(c)) {
      encodedString += c;
    } else {
      code1 = (c & 0xf) + '0';
      if ((c & 0xf) > 9) {
        code1 = (c & 0xf) - 10 + 'A';
      }
      c = (c >> 4) & 0xf;
      code0 = c + '0';
      if (c > 9) {
        code0 = c - 10 + 'A';
      }
      code2 = '\0';
      encodedString += '%';
      encodedString += code0;
      encodedString += code1;
      //encodedString+=code2;
    }
    yield();
  }
  return encodedString;
}
