/*
 * serial.h
 *
 *  Created on: 2017年3月18日
 *      Author: Home
 */

#ifndef SERIAL_H_
#define SERIAL_H_

extern int speed_arr[];
extern int name_arr[];
extern void set_speed(int fd, int speed);
extern int set_parity(int fd,int databits,int stopbits,int parity);
extern int open_dev(char *dev);


#endif /* SERIAL_H_ */
