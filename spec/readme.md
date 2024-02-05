# Daisy pinout
## Inputs
Daisy presents 6 digital inputs working at 24V:
```
#define I0 GPIO_NUM_34  
#define I2 GPIO_NUM_32  
#define I3 GPIO_NUM_33  
#define I4 GPIO_NUM_36  
#define I5 GPIO_NUM_39  
```
They can be used for example to control sensors, buttons and switches.
## Outputs
Daisy presents 7 digital outputs working at 24V:
```
#define Q0_PWM GPIO_NUM_25  
#define Q1_PWM GPIO_NUM_26  
#define Q2 GPIO_NUM_27 
#define Q3 GPIO_NUM_12  
#define Q4 GPIO_NUM_13  
#define Q5 GPIO_NUM_2   
#define Q6 GPIO_NUM_0   
```
They can be used for example to control motors, lights and relè.
## Load cells
The main dinstinct feature of Daisy are its 2 high precision Sigma-Delta ADCs, each featuring 2 PGA channels, for a total of 4 possible load cells controlled at the same time.
```
#define ADC1_DATA GPIO_NUM_35
#define ADC1_CLOCK GPIO_NUM_4
#define ADC2_DATA GPIO_NUM_14
#define ADC2_CLOCK GPIO_NUM_15
```
![Example Image]('../images/Load cells wiring.png')
Using the software provided in the folder "lib" of this repo its possible to manage up to 4 load cells with high precision at 80 samples per second.
## Features
Daisy also provides an RS485 channel for serial protocol communication (ex: Modbus), a Can channel for Can-based protocols (Ex: CanOpen) and a RTC powered by an onboard battery.
```
#define RS485_TX GPIO_NUM_17    //uart 2 tx
#define RS485_RX GPIO_NUM_16    //uart 2 rx
#define RS485_EN GPIO_NUM_5
```
