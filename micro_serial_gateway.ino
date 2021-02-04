/*
  Software serial multple serial test

 Receives from the hardware serial, sends to software serial.
 Receives from software serial, sends to hardware serial.

 The circuit:
 * RX is digital pin 10 (connect to TX of other device)
 * TX is digital pin 11 (connect to RX of other device)

 Note:
 Not all pins on the Mega and Mega 2560 support change interrupts,
 so only the following can be used for RX:
 10, 11, 12, 13, 50, 51, 52, 53, 62, 63, 64, 65, 66, 67, 68, 69

 Not all pins on the Leonardo and Micro support change interrupts,
 so only the following can be used for RX:
 8, 9, 10, 11, 14 (MISO), 15 (SCK), 16 (MOSI).

 created back in the mists of time
 modified 25 May 2012
 by Tom Igoe
 based on Mikal Hart's example

 This example code is in the public domain.

 */
/*
#include <SoftwareSerial.h>
SoftwareSerial softSerial(2, 3); // RX, TX
*/

#define R_SENSE 0.11f // Match to your driver
                      // SilentStepStick series use 0.11
                      // UltiMachine Einsy and Archim2 boards use 0.2
                      // Panucatt BSD2660 uses 0.1
                      // Watterott TMC5160 uses 0.075

// using TX 9 and RX 8
#include <AltSoftSerial.h>
AltSoftSerial softSerial;

#include <TMCStepper.h>
#include <TMCStepper_UTILITY.h>

using namespace TMC2209_n;

// Create driver that uses SoftwareSerial for communication
TMC2209Stepper *driver;

struct tmcConfig_t {
  char name[4];
  uint32_t gconf;
  uint32_t ihold_irun;
  uint32_t tpowerdown;
  uint32_t tpwmthrs;
  uint32_t chopconf;
  uint32_t pwmconf;
  uint32_t tcoolthrs;
  uint16_t coolconf;
};

typedef tmcConfig_t TmcConfig;

TmcConfig tmcDefault = { 
    "4 ",
    0x1c0,                                // gconf
    ( 4UL | ( 16UL << 8) | ( 2UL << 16)),  // ihold_irun: hold-run-hdelay
    20,                                   // tpowerdown
    90,                                   // tpwmthrs
    0x169102d4,                           // chopconf, 4msteps
    0xc80d0e24,                           // pwmconf
    0,                                    // tcoolthrs off
    0,                                    // coolconf
  };

TmcConfig tmcDefaultZ = { 
    "2 ",
    0x1c0,                                // gconf
    ( 4UL | (16UL << 8) | ( 2UL << 16)),  // ihold_irun: hold-run-hdelay
    20,                                   // tpowerdown
    90,                                   // tpwmthrs
    0x179102d4,                           // chopconf, 2msteps
    0xc80d0e24,                           // pwmconf
    0,  /*150, */                         // tcoolthrs on
    0,  /*0x2468, */                      // coolconf
  };

TmcConfig* tmc[4] = { &tmcDefault, &tmcDefault, &tmcDefault, &tmcDefaultZ };

void setup() {
  // Open serial communications towards host
  Serial.begin(19200);

  // open softserial towards drivers
  driver = new TMC2209Stepper( &softSerial, R_SENSE, 0);
  softSerial.begin(19200);

  // initialize structures
  driver->defaults();

  // set up all 4 drivers
  bool rc = setupTmc2209();

  Serial.print( "status: ");
  Serial.println( rc ? "err" : "ok");
  softSerial.end();

  // set the data rate for the SoftwareSerial port
  softSerial.begin(19200);
}

void loop() { // run over and over
  // pass through data
  if ( Serial.available()) {
    softSerial.write(Serial.read());
  }
  
  if( softSerial.available()) {
    Serial.write(softSerial.read());
  }
}

/*-----------------------------------------
* TMC settings
*/

bool setupTmc2209() {
  bool err = false;
  
  for( byte i=0; i < 4; i++) {
    driver->devaddr( i);
    err |= setTmc2209( tmc[ i]);
  }

  return err;
}

bool setTmc2209( TmcConfig* tmc) {
  driver->GCONF( tmc->gconf); 
  driver->IHOLD_IRUN( tmc->ihold_irun);
  driver->TPOWERDOWN( tmc->tpowerdown);
  driver->TPWMTHRS( tmc->tpwmthrs);
  driver->CHOPCONF( tmc->chopconf);
  driver->PWMCONF( tmc->pwmconf);
  driver->TCOOLTHRS( tmc->tcoolthrs);
  driver->COOLCONF( tmc->coolconf);

  return driver->CRCerror;
}
