/*
#  SZARK - Sterowana Zdalnie lub Autonomicznie Rozwojowa Konstrukcja (Remote-controlled or Autonomic Development Construction)
#  module bridge - converter between RS232 and I2C main bus
#  Michał Słomkowski 2011, 2012
#  www.slomkowski.eu m.slomkowski@gmail.com
*/

#ifndef _MENU_H_
#define _MENU_H_

#define MENU_REFRESH_NUMBER 0x2fff

void menuInit();
void menuSetToMain();
void menuCheckButtons();

#endif

