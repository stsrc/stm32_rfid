# RFID reader  

Study project.  

Idea of project was to create full RFID system with web interface.  

Main microcontroller	- STM32F100RB  
Web module		- esp8266  
RFID module		- SeeedStudio RDM 125KHz  
2.8' LCD display	- ILI9341 driver  
Touchpanel		- xpt2046 driver  
SD Card			- included in LCD display  
  
Features implemented:  
* Sensing RFID cards  
* Specific respons to single RFID card which bases on saved permission  
* Making and saving history of sensed cards (with date of this event)  
* Read only access to history from web  
* Read and write access to permission list from web  
