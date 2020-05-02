// NodeJs Backend server for receiving AES128 Encrypted data
//
// Version: 1.0.0 - Initial code
//
var SERVERPORT    = process.env.SERVEPORT || 8087;

// The AES encryption/decription key to be used.
var AESKey = '2B7E151628AED2A6ABF7158809CF4F3C';

// Define and call the packages necessary for the building the REST API.
const express       = require('express');
const app	        = express();
const bodyParser    = require('body-parser');
const cors          = require('cors')
const morgan        = require('morgan');
const CryptoJS      = require('crypto-js');

//Configure Express
app.use( bodyParser.urlencoded({ extended: true }) );
app.use( bodyParser.json() );
app.use( cors() );
app.options( '*', cors() );
app.use( morgan('dev') );

var router = express.Router();

//==============================================================================

function    decryptData( data, IV ) {
    var plain_iv = Buffer.from( IV , 'base64').toString('hex');
    var iv       = CryptoJS.enc.Hex.parse( plain_iv );
    var key      = CryptoJS.enc.Hex.parse( AESKey );

    // Decrypt
    var bytes = CryptoJS.AES.decrypt( data, key , { iv: iv} )

    try {
        var plaintext = bytes.toString(CryptoJS.enc.Base64)
        var decoded_b64msg = Buffer.from(plaintext , 'base64').toString('ascii');
        var decoded_msg = Buffer.from( decoded_b64msg , 'base64').toString('ascii');
        console.log("Decrypted message: ", decoded_msg);
    } catch(error) {
        console.log("Decryption error: " + error)
    }
}

//==============================================================================
function    ProcessData( req , res ) {
    
    console.log( "Data request: " , req.body );
    
    decryptData( req.body.data, req.body.iv );
    
    return res.status(200).json({success:true, message: 'success.' });
}    


// This REST end points here are NOT authenticated.
// The entry point is always through /auth first for users.
router.route('/setdata')
    .post( ProcessData )                  // Validate the user 
    .get( function(req, res) {
       console.log("Not implemented");
       return res.status(403).json({success: false, message: 'Not permited.' });
    });

router.route('/')
    .get( function(req, res) {
        res.status(200).json({ info: 'Backend REST Server' });
    });

//==============================================================================
// Start the server
// Our base url is /
app.use('/', router);
app.listen( SERVERPORT );

var datenow = new Date();
console.log("=========== MPGW UI - REST Api Server =============================");
console.log("Server started at " + datenow );
console.log("Api endpoint available at server port: " + SERVERPORT );
