/* mbed Microcontroller Library
 * Copyright (c) 2018 ARM Limited
 * SPDX-License-Identifier: Apache-2.0
 */

#include "Sabertooth.h"
#include "mbed.h"


const uint8_t fullSpeed = 127;
const uint8_t searchSpeed = 100;
const uint8_t mediumSpeed = 50;
const uint8_t slowSpeed = 20;

// > calibrated line -> black
// < calibrated line -> white
const int calibratedLine = 500;               
const int maxLineTime = 1000;                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                        

DigitalIn sensor1(p20, PullUp);
DigitalIn sensor2(p19, PullUp);
DigitalIn sensor3(p18, PullUp);
DigitalIn sensor4(p17, PullUp);
DigitalIn sensor5(p16, PullUp);

DigitalIn start(p6);

Timer t;
DigitalInOut line1(p11);
DigitalInOut line2(p12);
int resLine1;
int resLine2;

DigitalIn sensors[] = {sensor1,sensor2,sensor3,sensor4,sensor5};

DigitalOut myled(LED1);
DigitalOut myled2(LED2);
Sabertooth s(p9, 128, 9600);
Serial pc(USBTX, USBRX,NULL,19200);


int ReadEnemy(){
    int sensorValues = 0;
    //read the enemy sensors data and store it in sensorValues
    for(int i = 0; i < 5; i++){
        //sensorValues[i] = sensors[i].read();
        sensorValues |= sensors[i].read()<<i;   
    }
    return sensorValues;
}

void ReadLine(){
    //TODO revisit septimiu's version of the reading
    /*line1.output();
    line2.output();

    line1 = 1;
    line2 = 1;

    wait_us(10);

    line1.input();
    line2.input();

    t.start();
    while(int time = t.read_us() < maxLineTime){
        
        if(line1 == 0 && time < resLine1)
            resLine1 = t.read_us();
        if(line2 == 0 && time < resLine2)
            resLine2 = t.read_us();
    }
    t.stop();
    t.reset();*/

    line1.output();
    line1.mode(PullUp);
    wait_us(5);
    line1.mode(PullNone);
    line1 = 1;
    wait_us(10);
    

    t.start();
    line1.input();
    

    while(line1 == 1 &&  t.read_us() < maxLineTime);
    resLine1 = t.read_us();
    t.stop();
    t.reset();
    

    line2.output();
    line2.mode(PullUp);
    wait_us(5);
    line2.mode(PullNone);
    line2 = 1;
    wait_us(10);

    t.start();
    line2.input();

    while(line2 == 1 &&  t.read_us() < maxLineTime);
    resLine2 = t.read_us();
    t.stop();
    t.reset();
}

//int sensorValues[5];
uint8_t sensorValues;


int main() {
    for (int i = 0; i < 3; i++) {
        myled2 = 1;
        wait(0.5);
        myled2 = 0;
        wait(0.5);
    }
    s.setTimeout(0);
    s.setRamping(8);
    s.go(0,0);
    myled = 1;
    


    bool run;

    do{

        run = start.read();

        ReadLine();
        int sensorValues = ReadEnemy();
        pc.printf("values %d %d %d %d %d ", sensorValues & 1<<0,sensorValues & 1<<1,sensorValues & 1<<2,sensorValues & 1<<3,sensorValues & 1<<4);
        pc.printf("line %d \t%d\r\n", resLine1,resLine2);
    }while(!run);


    while (1) {
        //reading the sensors
        int sensorValues = ReadEnemy();

        //read line sensor values 
        ReadLine();

        //line is on the left
        if(resLine1 < calibratedLine){
                s.go(searchSpeed,-searchSpeed);
                wait_us(10);
        }

        //line is on the right
        else if(resLine2 < calibratedLine){
                s.go(-searchSpeed,searchSpeed);
                wait_us(10);
        }

        else{

            //decisions
        
            switch(sensorValues){
                //blana fata
                //(000)11011
                case 0x1B:  s.go(fullSpeed,fullSpeed);break;
                
                //mediu stanga
                //(000)10111
                case 0x17:  s.go(mediumSpeed,fullSpeed);break;
                
                //extrema stanga
                //(000)01111
                case 0x0F:  s.go(slowSpeed,fullSpeed);break;

                //mediu dreapta
                //(000)11101
                case 0x1D:  s.go(fullSpeed,mediumSpeed);break;

                //extrema dreapta
                //(000)11110
                case 0x1E:  s.go(fullSpeed,slowSpeed);break;


                default:    s.go(searchSpeed,searchSpeed);
            }
        }
        

    }

}