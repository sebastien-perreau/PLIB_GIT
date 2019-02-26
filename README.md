# PROTOTYPE LIBRARY

**Last release ver. *February 19***

## HARDWARE BASED ON:

Target: ***SPCB*** - (**S**)tandard (**P**)rototype (**C**)alculator (**B**)oard

- BLE PICKIT rev.B/C/D (*Prototype dev.*) or ETH PICKIT
- Ethernet Starter KIT II (*PIC32MX795F512L*)
- PICAdapter rev.D (*Prototype dev.*)


## HOW TO USE THE LIBRARY:

Download the last PLIB project. It is a MPLAB X Library Project with gcc compilation parameters set to 3 (Optimize yet more favoring speed).

First, you have to include ***PLIB.h***, which includes all library's header file, in your project:
* In your project **BLANCK_PROJECT**, open **config.h** and at the top of the file, include the **PLIB.h** following the directory where it is stored. 
* Then include the file **PLIB.h** in your **Header folder** project. Right click on the **Header folder** and **Add Existing Item...**. Search and select the file to include it in your project.

Now you have to include the **Library project** in your project:

* Right click on your **project --> Properties --> Conf: [default] --> Libraries**.
* Click on **Add Library Project...** and select **PLIB.X** (*the MPLABX IDE library project*).
* Remove previous library if present.

## LIBRARY STATUS

<img width="732" alt="capture d ecran 2019-02-22 a 22 07 43" src="https://user-images.githubusercontent.com/44413525/53271466-d084ef00-36ee-11e9-9968-9bb2659e3510.png">
