var CryptoJS = require("crypto-js");
var request  = require("request");

// ESP8266 IP address
var ESP8266_IP = "192.168.1.228";                       // Change the IP address for the device running the decryption functions.

// The AES Key must be valid hex numbers.
var AESKey = "2B7E151628AED2A6ABF7158809CF4F3C";

// The IV must have 16 bytes and be a valid hex number.
//var IV     = "000102030405060708090A0B0C0D0E0F";
var IV     = "ABCDFF450405060708090A0B0C0D0E0F";   // IV is an HEX string of numbers

function encryptMessage( messagePlain ) {

    messagePlain = messagePlain + '\0';                 // Add null terminator to string so that printf functions know the string end.

    try {
        var key = CryptoJS.enc.Hex.parse(AESKey);       // Parse the key from the HEX string to CryptoJS internal format
        var ivHex = CryptoJS.enc.Hex.parse( IV );       // Parse the IV vector from the HEX string to CryptoJS internal format.

        //var txtBase64 = new Buffer(messagePlain).toString('base64')   // This form is to be deprecated. do not use it. 
        var txtBase64 = Buffer.from( messagePlain).toString('base64');

        // Encrypt data
        var encrypted = CryptoJS.AES.encrypt(txtBase64, key , { iv: ivHex, mode: CryptoJS.mode.CBC, padding: CryptoJS.pad.Pkcs7 });

        //var ivB64 = new Buffer( ivHex.toString() ).toString('base64');
        var ivB64 = Buffer.from( ivHex.toString() ).toString('base64');     // Encode the IV to Base64 so we can transmit it on the URL safely

        // The CryptoJS function already outputs data in Base64 format.
        var encData =  encrypted.toString();

        console.log( "Debug info: ");
        console.log( "  Texto:      " +  txtBase64 );
        console.log( "  Crypto:     " + encrypted.toString() );
        console.log( "  Crypto len: " + encData.length);
        console.log( "  IV HEX:     " + ivHex );
        console.log( "  IV B64:     " + ivB64 );
        
        // Post data to the ESP8266 or ESP32.
        var url = "http://" + ESP8266_IP + "/info?"+ encData +"&"+ ivB64;
        console.log("url: ",url)

        request(url, function (error, response, body) {
   
        });

        console.log("Sanity checking...");
        // Decrypt
        var bytes  = CryptoJS.AES.decrypt( encrypted.toString(), key , { iv: ivHex} );
        var plaintext = bytes.toString(CryptoJS.enc.Utf8);

        console.log("Decrypted message UTF8 decoded: ", plaintext);

    }catch(error) {
        console.log("Exception catch: " + error)
    } 
}

// Generate random IVs.  This must be used if we really want to make communications secure.
function randomIV() {
   var result           = '';
   var characters       = 'ABCDEF0123456789';
   var charactersLength = characters.length;
   for ( var i = 0; i < 32; i++ ) {
      result += characters.charAt(Math.floor(Math.random() * charactersLength));
   }
   console.log("Random IV: " + result );
   return result;
}

console.log("Encrypt messages with fixed IV: ");
encryptMessage("PrimalCortex Test 1");
console.log("");
console.log("");
encryptMessage("a rather long test that spawns multiple AES128 blocks to check if everything is ok. It is!");

console.log("");
console.log("Send a sample Json object:" );
var json = {};
json.sampleID = 45;
json.sensor = "Test Sensor";
json.urn    = "urn:level:water";
json.value  = 12;
json.pump   = "On";
json.temp   = 23.2;

var js = JSON.stringify( json );
encryptMessage( js );

// Test with Random IV's
console.log("");
console.log("Testing with random IVs: " );
console.log("");
IV = randomIV();
encryptMessage("Random 1");

console.log("");
IV = randomIV();
encryptMessage("Random 2");
