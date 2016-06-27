# RFID reader  

Study project.  

Full RFID system with web interface.  

Main microcontroller	- STM32F100RB  
Web module		- esp8266  
RFID module		- SeeedStudio RDM 125KHz  
2.8' LCD display	- ILI9341 driver  
Touchpanel		- xpt2046 driver  
SD Card			- included in LCD display  
  
Features implemented:  
* displaing actual time, display turn off after time interval. Turn on by touch
* Sensing RFID cards  
* Specific respons to single RFID card displayed on LCD. Response bases on saved permission list
* Making and saving history of sensed cards (with date of this event)  
* Read only access to history from web  
* Read and write access to permission list from web  
