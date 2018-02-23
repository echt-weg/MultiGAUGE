==========================
Allgemeines
==========================
MultiGAUGE ist ein Open Source Projekt für ein günstiges Instrument zum experimentellem Einsatz in Flugzeugen.
Wichtig: Das Projekt befindet sich im frühen Alpha-Status und ist auch in der finalen Fassung niemals als alleiniges Anzeigesystem ausgelegt.
Das Gerät entspricht, auch wenn die Software einen stabilen Status errreichen wird, nicht den gesetzlichen  Anforderungen an zertifizierte Instrumente.
Eine Störung anderer Geräte kann nicht ausgeschlossen werden.

Ein Einsatz in Flugzeugen, Luftsportgeräten o.ä. ist im aktuellen Status nicht ratsam (sowie möglicherweise nicht erlaubt).

Die Materialkosten für alle Komponenten werden in der finalen Verson 50 EUR nicht übersteigen.

==========================
Funktionsumfang
==========================
Die Software hat aktuell folgende Funktionen:
-Anzeige Temperatur und aktueller Luftdruck (Alpha, funktionsfähig)
-Höhenmesser mit Angabe Höhe und Flugfläche (Alpha, ungetestet)
-Anzeige Bordspannung  (Alpha, funktionsfähig)
-Anzeige Füllstand-Tank für VDO-Tauchrohrgeber (Alpha, funktionsfähig)
-Anzeige Kühlwasser- und Öltemperatur (Alpha, nicht funktionsfähig. Weitere Hardware nötig!)

==========================
Teileliste für Prototypen (für Softwareversion 0.1x)
==========================
-1x Arduino Nano V3 oder Clone
-1x Nextion 2.4" Display
-1x GY-91 Sensorboard 
-2x Widerstände 1/4W 4.3k
-1x Widerstand 1/4W 10k
-Spannungsregler-Modul (basierend auf LM2596S)
-Kleinteile (Lötzinn, einige Kabel etc.)
-Gehäuse für Display, dient später auch als gesamtes Gehäuse

==========================
Anschlusschema MultiGAUGE
==========================


Bordnetz		ARDUINO NANO		Nextion-Display	   GY-91	
				GND-----------------GND----------------GND
				VIN-----------------+5V
				D10-----------------TX	
				D11-----------------RX
				3.3------------------------------------VIN
				A4-------------------------------------SDA
				A5-------------------------------------SCL
+12V--[R4.3K]---A0----------------------------------------------------Tankgeber Signal
+12V--------------------------
							 |
GND--[R4.3K]----A1--[R10K]----
GND-------------------------------------------------------------------Tankgeber Masse


