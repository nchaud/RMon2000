# RMonV3
Remote-Systems Monitoring Module (V3)

# Folder Structure
- Docs contains Technical Specs of components
- Docs/_Calcs houses spreadsheets with detail the calculations used by rMonV3
- RM is the software, which is developed using the Atmel Studio 7.0 IDE
- 3DPrintedCaseFile contains exactly that
- Sketch_PCB_Files has the design of the system from a PCB_printing perspective, using DipTrace as the IDE 

# Software
For testing, note the following flags :-
- IS_GSM_MOCK					true	//Set to true to mock GSM module interactivity
- IS_GPS_MOCK					true	//Set to true to mock GPS module interactivity

- INITIALISE_MODULE_ID		0			//0 implies don't initialise it - modules start with id 1
- IS_BASIC_MEM_TEST			false   	//Smoke test new module's EEPROM is physically present and working basics

//All Extended* tests define conditional code only compiled in when set - i.e. only run on the PC
- IS_EXTENDED_SHOW_100_BYTES	false	//Prints first 100 bytes
- IS_EXTENDED_SHOW_MEM			false		//Prints everything on this module nicely for review - TODO: A Basic?
- IS_EXTENDED_MEM_TEST			false		//Test reading signals and reading/writing to memory
- IS_EXTENDED_GSM_TEST			true		//Test for gprs and sending over web
- IS_EXTENDED_TYPES_TEST		false	//Test for RMv3 types are good, esp flags
