#include <Adafruit_NeoPixel.h>
#include <SoftwareSerial.h>

//
SoftwareSerial soft( 10, 11 );

const int neoPixelPin = 3;
const int pixelLength = 30;
const int availableFrames = 8;

String serialData = "";

int currentFrame = 0;
int animationDelay = 1000;
float currentTimeInS = 0;
int frameDelay = 10;

Adafruit_NeoPixel neoPixel = Adafruit_NeoPixel( pixelLength, neoPixelPin, NEO_GRB + NEO_KHZ800 );

byte ledFramesMatrix[ availableFrames ][ pixelLength ][ 3 ];


//
void setup() {
    Serial.begin( 9600 );
    soft.begin( 9600 );

    soft.println( '{"ready":"true"}' );

    neoPixel.begin();

    initLedMatrixData();
    resetNeoPixel();
}

//
void loop() {
    char tmpChar;
    String tmpSerialData = "";
    String tmpSoftData = "";

    matrixFrameAnimation();

    checkForNewSerialData();
}

void resetNeoPixel () {
    for ( int row = 0; row < pixelLength; row++ ) {
        neoPixel.setPixelColor( row, neoPixel.Color( 0, 0, 0 ) );     
    }

    neoPixel.show();
}

void initLedMatrixData () {
    for ( int column = 0; column < availableFrames; column++ ) {
        for ( int row = 0; row < pixelLength; row++ ) {
            ledFramesMatrix[ column ][ row ][ 0 ] = 1;
            ledFramesMatrix[ column ][ row ][ 1 ] = 1;
            ledFramesMatrix[ column ][ row ][ 2 ] = 1;
        }
    }
}

void updateLedMatrixWithString ( String data ) {
    int colI = data.substring( 0, 2 ).toInt();
    int rowI = data.substring( 3, 5 ).toInt();
    byte  rI = data.substring( 6, 9 ).toInt();
    byte  gI = data.substring( 10,13).toInt();
    byte  bI = data.substring( 14,17).toInt();

    Serial.println( "Set Pixel [" + (String)colI + "][" + (String)rowI + "] to " + (String)rI + ", " + (String)gI + ", " + (String)bI );

    ledFramesMatrix[ colI ][ rowI ][ 0 ] = rI;
    ledFramesMatrix[ colI ][ rowI ][ 1 ] = gI;
    ledFramesMatrix[ colI ][ rowI ][ 2 ] = bI;
}

void matrixFrameAnimation () {
    if ( millis() > currentTimeInS * 1000 + animationDelay ) {
        currentTimeInS = millis() / 1000;

        for ( int row = 0; row < pixelLength; row++ ) {
            neoPixel.setPixelColor( row, neoPixel.Color( ledFramesMatrix[ currentFrame ][ row ][ 0 ], ledFramesMatrix[ currentFrame ][ row ][ 1 ], ledFramesMatrix[ currentFrame ][ row ][ 2 ] ) );
        }

        neoPixel.show();

        if ( currentFrame < availableFrames - 1 ) {
            currentFrame++;
        } else {
            currentFrame = 0;
        }
    }
}

// fetching data from serial / softserial / bluetooth
void checkForNewSerialData () {
    char tmpChar;

    String tmpSerialData = "";
    String tmpSoftData = "";

    while ( Serial.available() ) {
        tmpChar = (char)Serial.read();
        
        if ( tmpChar < 127 && tmpChar >= 0 ) {
            tmpSerialData += tmpChar;
        }

        delay( 5 );
    }

    if ( tmpSerialData != "" ) {
        //
    }

    while( soft.available() ) {
        tmpChar = (char)soft.read();
        
        if ( tmpChar >= 0 && tmpChar < 127 ) {
            tmpSoftData += tmpChar;
            
            delay( 5 );
        }  
    }

    if ( tmpSoftData != "" ) {
        serialData = tmpSoftData;

        newSerialDataAvailable();
    }
}

String getValueFromKey ( String data, char key ) {
    int keyIndexStart = data.indexOf( key );

    if ( keyIndexStart > -1 ) {
        return data.substring( keyIndexStart + 1, data.length() );
    }

    return "";
}

String specifyKeyFromString ( String data ) {
    return data.substring( 0, data.indexOf( '=' ) );
}

void newSerialDataAvailable () {
    String key = specifyKeyFromString( serialData );

    if ( key == "s" ) {
        String value = getValueFromKey( serialData, 's=' );

        animationDelay = value.toInt();

        Serial.println( "Set Speed To: " + value );
    } else if ( key == "r" ) {
        String value = getValueFromKey( serialData, 'r=' );

        if ( value == "true" ) {
            initLedMatrixData();
            resetNeoPixel();

            Serial.println( "Reset Pixel Strip: " + value );
        } else {
            Serial.println( value );
        }

        updateLedMatrixWithString( value );
    } else if ( key == "p" ) {
        String value = getValueFromKey( serialData, 'p=' );

        soft.println( "p_success" );

        updateLedMatrixWithString( value );
    } else if ( key == "a" ) {
        String value = getValueFromKey( serialData, 'a=' );

        soft.println( value );

        Serial.println( "AT-Command: " +  value );
    } else {
        Serial.println( "New Serial Data: " + serialData );
    }
}
