﻿#include "AES.h"
#include "base64.h"

// For now it is for running on the ESP8266
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <ESP8266HTTPClient.h>
#include <WiFiUdp.h>
#include <SimpleTimer.h>

// CHANGE THE FOLLOWING PARAMETERS: THE AP Credentials and the NodeJs serve
String      SSID = "ap";
String      PASS = "ap";

// Node server IP
String      NODEServer_Address = "192.168.1.68";
String      NODEServer_Port    = "8087";

WiFiServer  server(80);
AES         aes;
AES         aesDecript;

// The necessary encryption information: First the pre-shared key.
byte        key[] = { 0x2B, 0x7E, 0x15, 0x16, 0x28, 0xAE, 0xD2, 0xA6, 0xAB, 0xF7, 0x15, 0x88, 0x09, 0xCF, 0x4F, 0x3C };

// The unitialized Initialization vector
byte        my_iv[N_BLOCK] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

// Working variables for hold data
char        b64data[200];
byte        cipher[1000];
byte        cipherDecrypt[1000];
byte        iv [N_BLOCK] ;

// Timer to send sample data at intervals
SimpleTimer timer;

// ESP8266 generate hardware random based numbers.
uint8_t getrnd() {
    uint8_t really_random = *(volatile uint8_t *)0x3FF20E44;
    return really_random;
}

// Generate a random initialization vector
void gen_iv(byte  *iv) {
    for (int i = 0 ; i < N_BLOCK ; i++ ) {
        iv[i]= (byte) getrnd();
    }
}

// Execute HTTP Post request to the Node Server
void sendData( String data, String iv)  {
    HTTPClient http;
    String url = "http://" + NODEServer_Address + ":" + NODEServer_Port + "/setdata";

    http.begin( url );
    http.addHeader( "content-type" , "application/json");

    // Post the message to the server
    http.POST("{\"iv\":\""+iv+"\",\"data\":\""+data+"\"}");
    http.end();
}

// Simple way to obtain values from strings.
String getValue(String data, char separator, int index)
{
    int found = 0;
    int strIndex[] = { 0, -1 };
    int maxIndex = data.length() - 1;

    for (int i = 0; i <= maxIndex && found <= index; i++) {
        if (data.charAt(i) == separator || i == maxIndex) {
            found++;
            strIndex[0] = strIndex[1] + 1;
            strIndex[1] = (i == maxIndex) ? i+1 : i;
        }
    }
    return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
}


// ====== ENCRYPT =====

void encryptData(String message) {  // OK
    char b64dataIV[64];

    // Generate random IV
    byte ivByteArray[16];                               // The IV's always have the same size used for AES: 16 bytes
    gen_iv( ivByteArray );
    
    b64_encode( b64dataIV, (char *)ivByteArray, N_BLOCK);
    String iv = String(b64dataIV);
    Serial.println ("IV B64: " + iv);
   
    // encrypt message  
    int b64len = b64_encode(b64data, (char *)message.c_str(),message.length()); 
    aes.do_aes_encrypt((byte *)b64data, b64len , cipher, key, 128, ivByteArray);
    
    // Encode the encrypted data in Base64 so that it can be safely transmitted 
    b64_encode(b64data, (char *)cipher, aes.get_size() );
    String data = String(b64data);

    Serial.println("------- Sending data:");
    Serial.println (" Data: " + data);

    Serial.print (" aes.get_size: ");
    Serial.println (aes.get_size());
    Serial.print (" b64data.length: ");
    Serial.println (data.length());
    Serial.println("");
    // Send data to the other server/device
    sendData( data, iv );
}

// Aux functions for handling HEX strings and convert them to bytes array.
byte nibble( char c )
{
    if ('0' <= c && c <= '9') return (byte)(c - '0');
    if ('A' <= c && c <= 'F') return (byte)(c - 'A' + 10);
    if ('a' <= c && c <= 'f') return (byte)(c - 'a' + 10);
    return 0;
}

byte getByte( char c1 , char c2 ) {
    return ( nibble(c1) << 4 ) | nibble(c2);
}

void hextobyte( char *in , int len , byte *out ) {
    int p = 0;
    for( int i = 0 ; i < len ; i = i + 2 ) {
        out[p] = getByte( in[i] , in[i+1]);        
        p++;
    }
    Serial.println("");
}

// Decrypts incomming data.
void decryptData(String b64data, String IV_base64) { 
    char data_decoded[300];
    char iv_decoded[32];
    byte p_iv[16];

    byte out[300];

    // Data enters encoded in Base64. So decode it.       
    int encrypted_length = b64_decode( data_decoded, (char *)b64data.c_str(), b64data.length());

    // IV is in BASE64 also
    b64_decode( iv_decoded, (char *)IV_base64.c_str(), IV_base64.length());

/*    Serial.println(String( (char *)iv_decoded).c_str());
    unsigned long long my_iv = 0;
    aes.set_IV(my_iv);
    aes.get_IV( f_iv );
    Serial.println("IV to be used: ");
    aes.printArray( f_iv , 16 );
*/
    hextobyte( (char *)iv_decoded, 32, (unsigned char *)p_iv );

//    Serial.println("IV decoded: ");
//    aes.printArray( (byte *)iv_decoded, 16 );
    
    // Decrypt data
    aes.do_aes_decrypt((byte *)data_decoded, encrypted_length, out, key, 128, (byte *)p_iv);

    char message[100];
    b64_decode(message, (char *)out, aes.get_size());

    Serial.print("Received Message: ");
    Serial.println(message);
    Serial.println("");
}

// Send sample data
void    sendData() {
    String switchOnCooler = "true";
    encryptData("{\"testdata\": \""+switchOnCooler+"\"}");
}

// Connects to WIFI
void    WIFI_Connect() {
    int i=0;
    // Connect to WiFi network
    Serial.println();
    Serial.println();
    Serial.printf("Connecting to WIFI: %s\n", SSID.c_str());
    
    WiFi.mode(WIFI_STA);
    WiFi.begin( SSID , PASS);

    while (WiFi.status() != WL_CONNECTED) {
        delay(100);
        Serial.print(".");
        i++;
        if ( i > 15 ) {
            i = 0;                
            Serial.printf("\nConnecting to WIFI: %s\n", SSID.c_str());
        }
    }

    Serial.println("");
    Serial.println("WiFi connected");

    // Start the server
    server.begin();
    
    // Print the IP address
    Serial.print("Use this URL : ");
    Serial.print("http://");
    Serial.print(WiFi.localIP());
    Serial.println("/");
}

// Startup
void setup() {
    Serial.begin(115200);
    delay( 1000 );                              // To allow the serial monitor to pick up the port initial output
    
    aes.set_key( key , sizeof(key));            // Get the globally defined key for encryption
    aesDecript.set_key( key , sizeof(key));     // Get the globally defined key for decryption
    
    // Connect to WIFI.
    WIFI_Connect();
        
    timer.setInterval( 10000 , sendData );         // Sends data every 10s    
}

void loop() { 

   timer.run();                                 // Execute timer events

    // Check for an active client
    WiFiClient client = server.available();
     
    if (!client) {
        return;
    } 
          
    while(!client.available()){  
      delay(1);
      timer.run(); 
    }     
  
    // Read client request
    String request = client.readStringUntil('\r');
    
    client.flush();
    if (request.indexOf("/info") != -1){  // OK
        String parse = getValue(request, '?', 1);
        String data = getValue(parse, '&', 0);
        String ivHttp = getValue(parse, '&', 1);
        String iv = getValue(ivHttp, ' ', 0);
        
        Serial.println("------------------- Data received --------------------");
        Serial.print(" Data: ");
        Serial.println(data);
        Serial.print(" IV: ");
        Serial.println( iv);
        
        decryptData(data, iv);
    }
}
