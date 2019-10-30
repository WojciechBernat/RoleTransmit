/* Remote */


/* Libraries */

#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

/* Directives and Macros */
#define PIPE_ADDRESS_SIZE  5    //Only 5 byte!
#define BUFFER_SIZE        8   //Size of TX and RX buffers


#define UART_SPEED_48    4800   //UART speeds
#define UART_SPEED_96    9600
#define UART_SPEED_288   28800
#define UART_SPEED_115   115200

#define TX_PIN_LED 6            //Pins numbers
#define RX_PIN_LED 5

/* Variables */
boolean RxState = false;
boolean TxState = false;

boolean RxRole = false;
boolean TxRole = true;

uint8_t TxCounter = 0x00;
uint8_t RxCounter = 0x00;
uint8_t ToTxCounter = 0x0A;
uint8_t ToRxCounter = 0x0F;

//uint32_t TimeExecute = 0;   niepotrzebne
uint32_t TxTimeExecute = 0;
uint32_t RxTimeExecute = 0;

uint16_t blinkTime = 200;


/* Arrays */
uint8_t TxBuffer[BUFFER_SIZE];
uint8_t RxBuffer[BUFFER_SIZE];

uint8_t TxAddresses[PIPE_ADDRESS_SIZE] = {0x0A, 0x0A, 0x0A, 0x0A, 0x01};  //TX pipeline address
uint8_t RxAddresses[PIPE_ADDRESS_SIZE] = {0x0B, 0x0B, 0x0B, 0x0B, 0x02};  //RX pipeline address

/* Prototypes */
boolean doubleBlink(uint8_t ledPin_1, uint8_t ledPin_2, uint16_t blinkTime);
void bufferReset(uint8_t *buf, uint8_t bufSize);

RF24 remote(7, 8);

void setup() {
  /* UART init */
  Serial.begin(UART_SPEED_96, SERIAL_8E1);                        //UART 8 bits with EVEN mode - że bit parzystości
  Serial.println("\nRemote application start\nUART init OK\n");
  delay(10);

  /* GPIO init */
  pinMode(TX_PIN_LED, OUTPUT);
  pinMode(RX_PIN_LED, OUTPUT);
  doubleBlink(TX_PIN_LED, RX_PIN_LED, blinkTime);
  Serial.println("\nLEDs init OK \nTX LED pin: 6 \nRX LED pin: 5 \n");
  delay(10);

  /* nRF24L01+ init */
  remote.begin();
  remote.openWritingPipe(TxAddresses);
  remote.openReadingPipe(1, RxAddresses);
  remote.setPALevel(RF24_PA_MIN);
  remote.stopListening();
  Serial.println("\nNRF24 init OK\n Set TX and RX pipeline addresses\n");
  delay(10);

  /* Clean buffers */
  bufferReset(TxBuffer, sizeof(TxBuffer));
  bufferReset(RxBuffer, sizeof(RxBuffer));
  Serial.println("\nTX and RX buffers RESET OK\n");

}

void loop() {

  TxBuffer[0] = map( analogRead(A0), 0, 1023, 0, 255);
  TxBuffer[1] = map( analogRead(A1), 0, 1023, 0, 255);
  TxBuffer[2] = map( analogRead(A2), 0, 1023, 0, 255);
  TxBuffer[3] = 0x00;

  delay(100);
  /* Start transmit */
  //  TxTimeExecute = micros(); //time execute measure
  remote.stopListening();
  if (TxRole) {                                        //if module is in transmitter mode
    digitalWrite(TX_PIN_LED, HIGH);                      //TX LED ON
    TxState = remote.write(TxBuffer, BUFFER_SIZE);      //transmit TxBuffer content and status transmission save
    digitalWrite(TX_PIN_LED, LOW);                       //TX LED OFF
    TxCounter++;
    Serial.println("\n TxState " + String(TxState) + "\n");
  }
  if (TxCounter == ToRxCounter) {
    RxRole = true;
    TxRole = false;
    Serial.println("\nChange role from TX to RX\nTxCounter: " + String(TxCounter));
    TxCounter = 0;
  }
  /* End of transmit */

  /* Start receive */
  if (RxRole) {
    digitalWrite(RX_PIN_LED, HIGH);
    if (remote.available()) {
      remote.read(RxBuffer, BUFFER_SIZE);
      Serial.print("RX Buffer print " + String(RxBuffer[0]));

    }
    RxCounter++;
    Serial.println("\n RxCounter " + String(RxCounter) + "\n");
    digitalWrite(RX_PIN_LED, LOW);
  }
  /* End of receive */
  if (RxCounter == ToTxCounter) {
    RxRole = false;
    TxRole = true;
    Serial.println("Change role from RX to TX\nRxCounter: " + String(RxCounter));
    RxCounter = 0;
  }


}



/* Functions */

/* Blink two LEDs */
boolean doubleBlink(uint8_t ledPin_1, uint8_t ledPin_2, uint16_t blinkTime) {
  if ( ((ledPin_1 < 0) || (ledPin_1 > 13)) || ((ledPin_2 < 0) || (ledPin_2 > 13)) ) {
    return false;
  }
  if ( blinkTime > 10000) {   //jezeli wieksze do 10 sek
    blinkTime = 2000;
  }

  if ( digitalRead(ledPin_1) == HIGH || digitalRead(ledPin_2) == HIGH ) {       // high high
    digitalWrite(ledPin_1, LOW);  digitalWrite(ledPin_2, LOW);
    delay(blinkTime);
    digitalWrite(ledPin_1, HIGH); digitalWrite(ledPin_2, HIGH);
    return true;

  } else if ( digitalRead(ledPin_1) == LOW || digitalRead(ledPin_2) == LOW ) {    //low low
    digitalWrite(ledPin_1, HIGH); digitalWrite(ledPin_2, HIGH);
    delay(blinkTime);
    digitalWrite(ledPin_1, LOW);  digitalWrite(ledPin_2, LOW);
    return true;

  } else if ( digitalRead(ledPin_1) == LOW || digitalRead(ledPin_2) == HIGH) {    //low high
    digitalWrite(ledPin_1, HIGH); digitalWrite(ledPin_2, LOW);
    delay(blinkTime);
    digitalWrite(ledPin_1, LOW);  digitalWrite(ledPin_2, HIGH);
    return true;

  } else if ( digitalRead(ledPin_1) == HIGH || digitalRead(ledPin_2) == LOW ) {   //high low
    digitalWrite(ledPin_1, LOW); digitalWrite(ledPin_2, HIGH);
    delay(blinkTime);
    digitalWrite(ledPin_1, HIGH);  digitalWrite(ledPin_2, LOW);
    return true;
  } else {
    return false;
  }
}

/* Clean arrays functions */
void bufferReset(uint8_t *buf, uint8_t bufSize) {            //Funkcja resetowanai bufora
  for (uint8_t i = 0; i < bufSize; i++) {
    buf[i] = 0;
  }
}
