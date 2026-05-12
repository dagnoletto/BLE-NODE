# BLE-NODE
This project is a Bluetooth Low Energy (BLE) sensor platform running over the STM32F091VCT6 microcontroller. The MCU is connected to a BLUENRG-M0 module.
The connection between the two is realized through an SPI where the MCU employes the Host Controller Interface (HCI) to talk directly to the Link Layer of the module. 
The higher layer protocols shall be implemented on the host side (microcontroller) as required. This gives full control and onership of the software as well as allows users 
to implement only the required parts and to optimize them.
