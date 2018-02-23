#include <Nextion.h>
#include <NextionPage.h>
#include <NextionCheckbox.h>
#include <NextionText.h>
#include <NextionPicture.h>
#include <Adafruit_BMP085_U.h>
#include <SoftwareSerial.h>
#include <i2c.h>
#include <i2c_BMP280.h>
#include <EEPROM.h>
#include <NextionVariableNumeric.h>
#include <NextionVariableString.h>




// Konfigurationsparameter für eigenes Flugzeug
// Nur in diesem Bereich sind bei Nutzung der vorgesehenen Sensoren Anpassungen notwendig
int qnh = 1013;                     // Startwert QNH
char headername[] = "Main";         // Überschrift im Hauptmenü
int tankvolumen = 45;               // Tankvolumen ganzzahlig in l - ausfliegbar
int fuel_min_red = 30;              // Tankvolumen ganzzahlig roter Bereich in l ausfliegbar verbleibend
int fuel_min_yellow = 70;           // Tankvolumen ganzzahlig gelber Bereich in l ausfliegbar verbleibend

float voltage_min_red = 11.5;       // Untergrenze Bordspannung roter Bereich. Separierung mit Punkt statt Komma (float)
float voltage_min_yellow = 12;      // Untergrenze Bordspannung gelber Bereich. Separierung mit Punkt statt Komma (float)
float voltage_max_red = 14.5;       // Obergrenze Bordspannung roter Bereich. Separierung mit Punkt statt Komma (float)
float voltage_max_yellow = 14;      // Obergrenze Bordspannung gelber Bereich. Separierung mit Punkt statt Komma (float)

int wtemp_min_red = 20;             // Untergrenze Wassertemperatur roter Bereich in °C ganzzahlig.
int wtemp_min_yellow = 40;          // Untergrenze Wassertemperatur gelber Bereich in °C ganzzahlig.
int wtemp_max_red = 110;            // Obergrenze Wassertemperatur roter Bereich in °C ganzzahlig.
int wtemp_max_yellow = 90;          // Obergrenze Wassertemperatur gelber Bereich in °C ganzzahlig.

int oeltemp_min_red = 45;           // Untergrenze Öltemperatur roter Bereich in °C ganzzahlig.
int oeltemp_min_yellow = 80;        // Untergrenze Öltemperatur gelber Bereich in °C ganzzahlig.
int oeltemp_max_red = 110;          // Obergrenze Öltemperatur roter Bereich in °C ganzzahlig.
int oeltemp_max_yellow = 95;        // Obergrenze Öltemperatur gelber Bereich in °C ganzzahlig.

float oelpress_min_red = 1.5;       // Untergrenze Öldruck roter Bereich in °C. Separierung mit Punkt statt Komma (float)
float oelpress_min_yellow = 2;      // Untergrenze Öldruck gelber Bereich in °C. Separierung mit Punkt statt Komma (float)
float oelpress_max_red = 4;         // Obergrenze Öldruck roter Bereich in °C. Separierung mit Punkt statt Komma (float)
float oelpress_max_yellow = 3;      // Obergrenze Öldruck gelber Bereich in °C. Separierung mit Punkt statt Komma (float)


// Technische Parameter für Implementierug und Einbau sowie Tests (Anpassung bei Abweichung vom Std notwendig)
float voltagemultiplier = 15;        // Eingangsmultiplikator zur Spannung auf Basis Maximum 5V (abhängig von Widerstandskombination). Separierung mit Punkt statt Komma (float)
float fuelmultiplier = 4.35;         // Eingangsmultiplikator zur Benzinanzeige (abhängig von Widerstandskombination und verbautem Geber). Separierung mit Punkt statt Komma (float)
int fuelcutter = 1000;               // Eingangsabzug eines 1023er Signals zur Benzinanzeige (abhängig von Widerstandskombination und verbautem Geber). Ganzzahlig


// Sonstige Variablen, nicht zur Anpassung für die Konfiguration des Flugzeugs
int activepage;
float qnhf;
int logdebuglevel;                  // Wird bei jedem Start über die Daten des EEPROM überschrieben
int gaugescalernumber = 4;
int gaugetxtdesc = 9;
int gaugetxtval = 10;
int refreshcounter = 0;
int dscounter = 1;
int startpagenumber=9;



// Definieren der Sensoren und der Display-Verbindung
BMP280 bmp280;
SoftwareSerial nextionSerial(10, 11); // RX, TX
Nextion nex(nextionSerial);
NextionPicture qnhdown(nex, 3, 4, "p2");
NextionPicture qnhup(nex, 3, 5, "p3");
NextionPicture usbdebugger(nex, 5, 6, "p2");
NextionPicture startpageleft(nex, 5, 8, "p3");
NextionPicture startpageright(nex, 5, 9, "p4");

// Funktion zum Senden der Daten an das Display
void senddata(char message[], int method)
{
  if (method == 1)
  {
    nextionSerial.print(message);
    nextionSerial.write(0xff);
    nextionSerial.write(0xff);
    nextionSerial.write(0xff);
  }
  if (method == 2)
  {
    nextionSerial.print("\"");
    nextionSerial.print(message);
    nextionSerial.print("\"");
    nextionSerial.write(0xff);
    nextionSerial.write(0xff);
    nextionSerial.write(0xff);
  }

}

// Debug-Funktion fürs Logging via serieller Schnittstelle
void debuglog(String message)
{
  if (logdebuglevel > 0)
  {
    Serial.print(">>");
    Serial.println(message);
  }
}


// Einmalige Initalisierung beim Start
void setup()
{
  // Starten der seriellen Verbindung
  Serial.begin(57600);
  nextionSerial.begin(57600);
  nex.init();

  // Auslesen des EEPROM und entsprechendes Setzen der Variablen
  if (EEPROM.read(0) == 100)
  {
    logdebuglevel = 1;
    Serial.print("EEPROM-Daten auf Adresse 0:");
    Serial.print(EEPROM.read(0));
    Serial.println(" haben das Logging aktiviert");
  }
  else
  {
    logdebuglevel = 0;
  }
  delay(20);
  // Auslesen des EEPROM und entsprechendes Setzen der Variablen
  if (EEPROM.read(1) < 10)
  {
    delay(20);

    Serial.print("EEPROM-Daten auf Adresse 1:");
   Serial.print(EEPROM.read(1));
     delay(20);

  startpagenumber=EEPROM.read(1);
  }


  

  debuglog("Suche BMP280: ");
  if (bmp280.initialize())
  {
    debuglog("BMP280 gefunden");
  }
  else
  {
    debuglog("BMP280 vermisst");
    while (1) {}
  }
  delay(100);
  // Erstmessung des BMP280 als Referenz
  bmp280.setEnabled(0);
  bmp280.triggerMeasurement();
  delay(400);
  // Manuelle Initalisierung der ersten Seite und setzen der Parameter
  nextionSerial.print("bauds=57600");
  nextionSerial.write(0xff);
  nextionSerial.write(0xff);
  nextionSerial.write(0xff);
  delay(7);
  nextionSerial.print("bauds=57600");
  nextionSerial.write(0xff);
  nextionSerial.write(0xff);
  nextionSerial.write(0xff);
  delay(7);
  nextionSerial.print("bkcmd=0");
  nextionSerial.write(0xff);
  nextionSerial.write(0xff);
  nextionSerial.write(0xff);
  delay(7);
  nextionSerial.print("page ");
  nextionSerial.print(startpagenumber);
  nextionSerial.write(0xff);
  nextionSerial.write(0xff);
  nextionSerial.write(0xff);
  delay(50);
  // Ergänzen der jeweiligen Callbacks für Aktionsbuttons
  qnhdown.attachCallback(&cbdown);
  qnhup.attachCallback(&cbup);
  usbdebugger.attachCallback(&cbdebug);
 // startpageleft.attachCallback(&cbstartpageleft);
 // startpageright.attachCallback(&cbstartpageright);
}



void Datasend(int numbertodo)
{
  char sendestring[100] = "";

  if (numbertodo == 1)
  {
    // Senden deS Headers
    nextionSerial.print("boot.header.txt=\"");
    nextionSerial.print(headername);
    nextionSerial.print("\"");
    nextionSerial.write(0xff);
    nextionSerial.write(0xff);
    nextionSerial.write(0xff);

    // Senden der Startseitenbeschreibung
    nextionSerial.print("boot.setupstartpage.txt=\"");
    if (startpagenumber==1)
    {
       nextionSerial.print("Menu");
    }
    if (startpagenumber==2)
    {
       nextionSerial.print("Temp");
    }
    if (startpagenumber==3)
    {
       nextionSerial.print("Altimeter");
    }
    if (startpagenumber==4)
    {
       nextionSerial.print("Checklist");
    }
    if (startpagenumber==5)
    {
       nextionSerial.print("Setup");
    }
    if (startpagenumber==6)
    {
       nextionSerial.print("Gauges");
    }
    if (startpagenumber==7)
    {
       nextionSerial.print("About");
    }
    if (startpagenumber==8)
    {
       nextionSerial.print("Debug");
    }
    if (startpagenumber==9)
    {
       nextionSerial.print("Warning");
    }
    nextionSerial.print("\"");
    nextionSerial.write(0xff);
    nextionSerial.write(0xff);
    nextionSerial.write(0xff);
    

    

    // Senden des Logmodes
    nextionSerial.print("boot.setuplogging.val=");
    nextionSerial.print(logdebuglevel);
    nextionSerial.write(0xff);
    nextionSerial.write(0xff);
    nextionSerial.write(0xff);


    //Senden von Versions- und Produktdaten
    nextionSerial.print("boot.swname.txt=\"MultiGAUGE\"");
    nextionSerial.write(0xff);
    nextionSerial.write(0xff);
    nextionSerial.write(0xff);
    nextionSerial.print("boot.swver.txt=\"SW-Version 0.1.1\"");
    nextionSerial.write(0xff);
    nextionSerial.write(0xff);
    nextionSerial.write(0xff);

  }

  if (numbertodo == 2)
  {

    // Senden des eingestellten QNH
    sprintf(sendestring, "boot.qnh.val=%d", qnh);
    debuglog(sendestring);
    nextionSerial.print(sendestring);
    nextionSerial.write(0xff);
    nextionSerial.write(0xff);
    nextionSerial.write(0xff);


    // Auslesen der Messdaten vom BMP280
    bmp280.awaitMeasurement();
    float temperature;
    bmp280.getTemperature(temperature);
    float pascal;
    bmp280.getPressure(pascal);
    bmp280.triggerMeasurement();
    pascal = pascal / 100;
    int qfe = (int)pascal;
    int flightlevel = (1013 - qfe) * 30 / 100;
    int altitude = (qnh - qfe) * 30;
    // Senden der aktuellen Temperatur
    nextionSerial.print("boot.temp.txt=\"");
    nextionSerial.print(temperature);
    nextionSerial.print("\"");
    nextionSerial.write(0xff);
    nextionSerial.write(0xff);
    nextionSerial.write(0xff);


    // Senden der aktuellen Temperatur lesbar
    nextionSerial.print("boot.temphuman.txt=\"");
    nextionSerial.print(temperature);
    nextionSerial.print("C\"");
    nextionSerial.write(0xff);
    nextionSerial.write(0xff);
    nextionSerial.write(0xff);


    // Senden des aktuellen QFE
    sprintf(sendestring, "boot.qfe.val=%d", qfe);
    debuglog(sendestring);
    nextionSerial.print(sendestring);
    nextionSerial.write(0xff);
    nextionSerial.write(0xff);
    nextionSerial.write(0xff);


    // Senden des aktuellen QFE lesbar
    sprintf(sendestring, "boot.qfehuman.txt=\"%d hPa\"", qfe);
    debuglog(sendestring);
    nextionSerial.print(sendestring);
    nextionSerial.write(0xff);
    nextionSerial.write(0xff);
    nextionSerial.write(0xff);



    // Senden der aktuellen Höhe
    sprintf(sendestring, "boot.altitude.val=%d", altitude);
    debuglog(sendestring);
    nextionSerial.print(sendestring);
    nextionSerial.write(0xff);
    nextionSerial.write(0xff);
    nextionSerial.write(0xff);


    // Senden der aktuellen Höhe
    sprintf(sendestring, "boot.altitudehuman.txt=\"%dft\"", altitude);
    debuglog(sendestring);
    nextionSerial.print(sendestring);
    nextionSerial.write(0xff);
    nextionSerial.write(0xff);
    nextionSerial.write(0xff);


    // Senden des Flightlevel
    sprintf(sendestring, "boot.flightlevel.val=%d", flightlevel);
    debuglog(sendestring);
    nextionSerial.print(sendestring);
    nextionSerial.write(0xff);
    nextionSerial.write(0xff);
    nextionSerial.write(0xff);



    // Senden des Flightlevel
    sprintf(sendestring, "boot.flhuman.txt=\"FL%d\"", flightlevel);
    debuglog(sendestring);
    nextionSerial.print(sendestring);
    nextionSerial.write(0xff);
    nextionSerial.write(0xff);
    nextionSerial.write(0xff);


    // Senden des Höhenmesserdrehwinkels
    if (altitude <= 3750)
    {
      sprintf(sendestring, "boot.altimeterscale.val=%d", 90 + (altitude / 14));
      debuglog(sendestring);
      nextionSerial.print(sendestring);
      nextionSerial.write(0xff);
      nextionSerial.write(0xff);
      nextionSerial.write(0xff);
    } else
    {
      sprintf(sendestring, "boot.altimeterscale.val=%d", (altitude / 14) - 270);
      debuglog(sendestring);
      nextionSerial.print(sendestring);
      nextionSerial.write(0xff);
      nextionSerial.write(0xff);
      nextionSerial.write(0xff);
    }


  }

  if (numbertodo == 3)
  {
    // Voltmeter Start
    //Mittelwert über 5 Messungen erzeugen, damit Wert sinnvoll verwendert werden kann
    int analogreadervoltage = 0;
    for (int i = 0; i < 5; i++) {
      analogreadervoltage += analogRead(A0);
    }
    analogreadervoltage = trunc(analogreadervoltage / 5);
    long voltagecolor = 9344;
    float voltscale = analogreadervoltage * (voltagemultiplier / 1023.0); //Einlesen der echten Spannung auf Basis des definierten Multiplikators
    int voltperc = 6 * voltscale; // Maximale Spannung mal 6 ergibt sinnvollen Scale-Wert auf 100%, theoretisches Maximum 16,6 V
    if (voltscale >= voltage_max_yellow)
    {
      voltagecolor = 65504;
    }
    if (voltscale >= voltage_max_red)
    {
      voltagecolor = 63488;
    }
    if (voltscale <= voltage_min_yellow)
    {
      voltagecolor = 65504;
    }
    if (voltscale <= voltage_min_red)
    {
      voltagecolor = 63488;
    }
    // Senden der aktuellen Spannung
    nextionSerial.print("boot.voltage.txt=\"");
    nextionSerial.print(voltscale);
    nextionSerial.print("\"");
    nextionSerial.write(0xff);
    nextionSerial.write(0xff);
    nextionSerial.write(0xff);


    // Senden der aktuellen Spannung lesbar
    nextionSerial.print("boot.voltagehuman.txt=\"");
    nextionSerial.print(voltscale);
    nextionSerial.print("V\"");
    nextionSerial.write(0xff);
    nextionSerial.write(0xff);
    nextionSerial.write(0xff);


    // Senden der aktuellen Farbe
    sprintf(sendestring, "boot.voltagecolor.val=%ld", voltagecolor);
    debuglog(sendestring);
    nextionSerial.print(sendestring);
    nextionSerial.write(0xff);
    nextionSerial.write(0xff);
    nextionSerial.write(0xff);


    // Senden der aktuellen Farbe
    sprintf(sendestring, "boot.voltagescale.val=%d", voltperc);
    debuglog(sendestring);
    nextionSerial.print(sendestring);
    nextionSerial.write(0xff);
    nextionSerial.write(0xff);
    nextionSerial.write(0xff);


    // Voltmeter Ende
  }


  if (numbertodo == 4)
  {

    // Benzinanzeige Start
    //Mittelwert über 5 Messungen erzeugen, damit Wert sinnvoll verwendert werden kann
    int analogreaderfuel = 0;
    for (int i = 0; i < 5; i++) {
      analogreaderfuel += analogRead(A1);
    }
    analogreaderfuel = trunc(analogreaderfuel / 5);
    int fuelscale = ((1023 - analogreaderfuel) - fuelcutter) * fuelmultiplier; //Invertieren des analogen Signals, so dass höhere Spannung volleren Tank ergeben und errechnen der Füllung
    long fuelcolor = 9344;
    if (fuelscale <= fuel_min_yellow)
    {
      fuelcolor = 65504;
    }
    if (fuelscale <= fuel_min_red)
    {
      fuelcolor = 63488;
    }
    // Senden des aktuellen Tankinhalts
    sprintf(sendestring, "boot.fuel.val=%d", fuelscale * tankvolumen / 100);
    debuglog(sendestring);
    nextionSerial.print(sendestring);
    nextionSerial.write(0xff);
    nextionSerial.write(0xff);
    nextionSerial.write(0xff);


    // Senden des aktuellen Tankinhalts lesbar
    sprintf(sendestring, "boot.fuelhuman.txt=\"%dl\"", fuelscale * tankvolumen / 100);
    debuglog(sendestring);
    nextionSerial.print(sendestring);
    nextionSerial.write(0xff);
    nextionSerial.write(0xff);
    nextionSerial.write(0xff);


    // Senden der aktuellen Farbe
    sprintf(sendestring, "boot.fuelcolor.val=%ld", fuelcolor);
    debuglog(sendestring);
    nextionSerial.print(sendestring);
    nextionSerial.write(0xff);
    nextionSerial.write(0xff);
    nextionSerial.write(0xff);


    // Senden der aktuellen Skala
    sprintf(sendestring, "boot.fuelscale.val=%d", fuelscale);
    debuglog(sendestring);
    nextionSerial.print(sendestring);
    nextionSerial.write(0xff);
    nextionSerial.write(0xff);
    nextionSerial.write(0xff);


    // Benzinanzeige Ende
  }

  if (numbertodo == 5)
  {
    // Wtemp Start
    int wtempscale = 70; //Dummy-Wert zum Testen
    int wtempperc = 0.75 * wtempscale; // Reduzierung auf 75%
    long wtempcolor = 9344;
    if (wtempscale >= wtemp_max_yellow)
    {
      wtempcolor = 65504;
    }
    if (wtempscale >= wtemp_max_red)
    {
      wtempcolor = 63488;
    }
    if (wtempscale <= wtemp_min_yellow)
    {
      wtempcolor = 65504;
    }
    if (wtempscale <= wtemp_min_red)
    {
      wtempcolor = 63488;
    }
    // Senden des aktuellen Tankinhalts
    sprintf(sendestring, "boot.wtemp.val=%d", wtempscale);
    debuglog(sendestring);
    nextionSerial.print(sendestring);
    nextionSerial.write(0xff);
    nextionSerial.write(0xff);
    nextionSerial.write(0xff);


    // Senden des aktuellen Tankinhalts lesbar
    sprintf(sendestring, "boot.wtemphuman.txt=\"%dC\"", wtempscale);
    debuglog(sendestring);
    nextionSerial.print(sendestring);
    nextionSerial.write(0xff);
    nextionSerial.write(0xff);
    nextionSerial.write(0xff);


    // Senden der aktuellen Farbe
    sprintf(sendestring, "boot.wtempcolor.val=%ld", wtempcolor);
    debuglog(sendestring);
    nextionSerial.print(sendestring);
    nextionSerial.write(0xff);
    nextionSerial.write(0xff);
    nextionSerial.write(0xff);


    // Senden der aktuellen Skala
    sprintf(sendestring, "boot.wtempscale.val=%d", wtempperc);
    debuglog(sendestring);
    nextionSerial.print(sendestring);
    nextionSerial.write(0xff);
    nextionSerial.write(0xff);
    nextionSerial.write(0xff);


    // Wtemp Ende
  }

  if (numbertodo == 6)
  {
    // otemp Start
    int otempscale = 92; //Dummy-Wert zum Testen
    int otempperc = 0.75 * otempscale; // Reduzierung auf 75%
    long otempcolor = 9344;
    if (otempscale >= oeltemp_max_yellow)
    {
      otempcolor = 65504;
    }
    if (otempscale >= oeltemp_max_red)
    {
      otempcolor = 63488;
    }
    if (otempscale <= oeltemp_min_yellow)
    {
      otempcolor = 65504;
    }
    if (otempscale <= oeltemp_min_red)
    {
      otempcolor = 63488;
    }
    // Senden des aktuellen Tankinhalts
    sprintf(sendestring, "boot.otemp.val=%d", otempscale);
    debuglog(sendestring);
    nextionSerial.print(sendestring);
    nextionSerial.write(0xff);
    nextionSerial.write(0xff);
    nextionSerial.write(0xff);


    // Senden des aktuellen Tankinhalts lesbar
    sprintf(sendestring, "boot.otemphuman.txt=\"%dC\"", otempscale);
    debuglog(sendestring);
    nextionSerial.print(sendestring);
    nextionSerial.write(0xff);
    nextionSerial.write(0xff);
    nextionSerial.write(0xff);


    // Senden der aktuellen Farbe
    sprintf(sendestring, "boot.otempcolor.val=%ld", otempcolor);
    debuglog(sendestring);
    nextionSerial.print(sendestring);
    nextionSerial.write(0xff);
    nextionSerial.write(0xff);
    nextionSerial.write(0xff);


    // Senden der aktuellen Skala
    sprintf(sendestring, "boot.otempscale.val=%d", otempperc);
    debuglog(sendestring);
    nextionSerial.print(sendestring);
    nextionSerial.write(0xff);
    nextionSerial.write(0xff);
    nextionSerial.write(0xff);


    // otemp Ende
  }

}


// Callback startpageleft
void cbstartpageleft(NextionEventType type, INextionTouchable *widget)
{
  if (type == NEX_EVENT_PUSH)
  {
    if (startpagenumber>0)
    {
      startpagenumber=startpagenumber-1;
      Datasend(1);
      Serial.println("Neue Startseite konfiguriert");
      //EEPROM.write(1, startpagenumber);
      delay(50);
    }
  }
}

// Callback startpageright
void cbstartpageright(NextionEventType type, INextionTouchable *widget)
{
  if (type == NEX_EVENT_PUSH)
  {
    if (startpagenumber<10)
    {
     startpagenumber=startpagenumber+1;
     Datasend(1);
     Serial.println("Neue Startseite konfiguriert");
    // EEPROM.write(1, startpagenumber);
     delay(50);
  }
  }
}




// Callback QNH down
void cbdown(NextionEventType type, INextionTouchable *widget)
{
  if (type == NEX_EVENT_PUSH)
  {
    debuglog("QNH verringern");
    qnh = qnh - 1;
  
  }
}

// Callback QNH up
void cbup(NextionEventType type, INextionTouchable *widget)
{
  if (type == NEX_EVENT_PUSH)
  {
    debuglog("QNH errhöhen");
    qnh = qnh + 1;
  }
}

// Callback Debug
void cbdebug(NextionEventType type, INextionTouchable *widget)
{
  if (type == NEX_EVENT_PUSH)
  {
    if (logdebuglevel == 1)
    {
      debuglog("Logging wird deaktiviert");
      EEPROM.write(0, 255);
      delay(50);
      logdebuglevel = 0;
    } else
    {
      logdebuglevel = 1;
      EEPROM.write(0, 100);
      delay(50);
      debuglog("Logging ist aktiviert");
    }
  }
}


// Dauerschleife zum Messen und Auswerten
void loop()
{
  delay(70); //Reduzierung der Loop-Geschwindigkeit
  // Empfang von Daten
  nex.poll();
  // Bei jedem Durchlauf einen Datensatz senden
  Datasend(dscounter);
  if (dscounter == 7)
  {
    dscounter = 1;
  }
  else
  {
    dscounter = dscounter + 1;
  }
}
