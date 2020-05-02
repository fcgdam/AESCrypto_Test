# AESCrypto_Test

This code tests the communication of encrypted data from a NodeJS based program to an ESP8266/ESP32 running a web server that receives AES128 CBC PKCS#7 encrypted message and decrypts it onboard.

This is a follow-up on the [comment section of my blog post regarding ESP8266/NodeJs and encryption](https://primalcortex.wordpress.com/2016/06/17/esp8266-logging-data-in-a-backend-aes-and-crypto-js/).

The original post discusses encrypting data on the device and send it to a NodeJs server.

This code provides a sample for the other way around, from NodeJs to the device. The initial testing code is from Wordpress user Edu.

# Flashing the firmware on the ESP8266.

Install PlatformIO, and just run *pio run -t upload* to flash the board.

But before doing that three things need to be changed: the SSID and Password of the WIFI access point must be modified and the IP address of the target NodeJs Server. Change it at the top of the *src/main.cpp* file.

After connection, take note of the IP.

# Testing NodeJS to ESP8266.

On the *Node_Client* folder there is the test program. Change the server IP address of the ESP8266 at the top of the file *main.js*.

To run the code run *npm install* once, and then the code can be run any number of times with *node main.js* command.

# Testing ESP8266 to NodeJS

On the *Node_Server* folder there is the server compontent that waits for data sent from the ESP8266, decrypts it and shows it on the console.
The default listening port is 8087.

To run the code just run *npm install* once, and then the code can be run with *node server.js*. The server should start to listen for incoming data.
Note that the IP of the server must be previously defined on the ESP8266 software.

# Sample Output:

The following is a sample output on the ESP serial port when receiving encrypted messages sent by NodeJS program.

```
------------------- Data received --------------------
Data: 42G9YQloRQooNZY66PE70QyJ2WyAMjAxoNEKGkZ3ORs=
IV: YWJjZGZmNDUwNDA1MDYwNzA4MDkwYTBiMGMwZDBlMGY=
==================================================
==================   DECRYPT   ===================
==================================================
Received Message: prubea primalcortex
------------------- Data received --------------------
Data: w/9CVXlM/Co6+CCYXFYRHbll8XK75nZKQkbKQBH3mC+lMRz9VHzHYdZBTcxodspaQFwtFuq7ZrFO3iZ2bMWgNQ==
IV: YWJjZGZmNDUwNDA1MDYwNzA4MDkwYTBiMGMwZDBlMGY=
==================================================
==================   DECRYPT   ===================
==================================================
Received Message: teste comprido e ainda mais comprido
------------------- Data received --------------------
Data: XDYJE7ogZxyy0ihEPy0iPM9SdcMRr+etCR4j2Oq3Eu7C+p6FP9Ba8WkKrlaZckd9S0dV5CqIXOY1Pt8vxaOISCKcD3abIADLY0z+ovRYTscZKuNP+to+vth4iQpWNrqGfnrnBD3bBJuglmrIdrRcqXXgGslgtMmgnXV5UNQX2rY=
IV: YWJjZGZmNDUwNDA1MDYwNzA4MDkwYTBiMGMwZDBlMGY=
==================================================
==================   DECRYPT   ===================
==================================================
Received Message: {"sensor":"Test Sensor","urn":"urn:level:water","value":12,"pump":"On","temp":23.2}

```

# Final notes:

The NodeJs provides the IV as an HEX string not as a byte array to the device. This makes decryption fail since the IV is NOT a byte array. We must decode the IV from Base64 and then from HEX string format to byte array so that we are able to use it.

The testing NodeJs code also tests with random IVs. This prevents that the same message has the same output, but does not prevent replay attacks. A further improvement of the code should check for some sequence number or other type of replay attack mitigation strategies.
