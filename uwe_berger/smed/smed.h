/*---------------------------------------------------------------------------------------------------------------------------------------------------
 *            smed.h --> Small Editor
 *            =======================
 * Copyright (c) 2011 Uwe Berger - bergeruw@gmx.net
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
#ifndef DS1307_H
#define DS1307_H

#define PROG_NAME	"smed - Version 0.2"
#define PROG_AUTHOR	"by Uwe Berger, 2011"
#define TAB_LEN		4

#define FILENAME_MAX_LEN 8

#define ADVANCED_FUNCTIONS

void smed(void);
void load_edit_data(void);
void save_edit_data(void);

#endif
