/*---------------------------------------------------------------------------------------------------------------------------------------------------
 *            smed.c --> Small Editor
 *            =======================
 * Copyright (c) 2011 Uwe Berger - bergeruw@gmx.net
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#ifdef unix
#include <unistd.h>
#else
#include <avr/io.h>
#include <util/delay.h>
#endif

#include "smed.h"
#include "mcurses/mcurses.h"
#include "uart/usart.h"


static struct cursor_t{
	uint8_t x;
	uint8_t y;
} cursor;

static struct {
	int current_line;
	int current_pos;
	int max_lines;
	int max_pos;
} edit_buf;


// forwards
static void cursor_down(void);
static void cursor_up(void);
static void cursor_left(void);
static void cursor_right(void);


//--------------------------------------
//----- Begin of plattform -------------
//--------------------------------------
//--------------------------------------

#if ACCESS_VIA_FILE

char file_name[FILENAME_MAX+1];
FILE *fd;
uint8_t *edit_data;
//--------------------------------------
static void fflush_stdin(void) {
	while ( getchar() != '\n' );
}
//--------------------------------------
long fsize(FILE *fs) {
	long o, r;
	o = ftell(fs);
	fseek(fs, 0, SEEK_END);
	r = ftell(fs);
	fseek(fs, o, SEEK_SET);
	return r;
}
//--------------------------------------
static uint8_t get_edit_data(long int idx) {
	return edit_data[idx];
}
//--------------------------------------
static void set_edit_data(uint8_t c) {
	edit_data[edit_buf.current_pos]=c;
}
//--------------------------------------
static void insert_edit_data(uint8_t c) {
	// Platz schaffen
	edit_buf.max_pos++;
	edit_data=realloc(edit_data, edit_buf.max_pos+1);
	// ab akt. Position eine Stelle nach hinten kopieren
	memmove(&edit_data[edit_buf.current_pos+1], 
			&edit_data[edit_buf.current_pos], 
			edit_buf.max_pos - edit_buf.current_pos);
	// akt. Position neu setzen
	set_edit_data(c);
}
//--------------------------------------
static void delete_edit_data(void) {
	// ab akt. Position +1 eine Stelle nach links verschieben	
	edit_buf.max_pos--;
	memmove(&edit_data[edit_buf.current_pos], 
			&edit_data[edit_buf.current_pos+1], 
			edit_buf.max_pos - edit_buf.current_pos);
	// Platz verkleinern
	edit_data=realloc(edit_data, edit_buf.max_pos);
}
//--------------------------------------
void load_edit_data(void) {
	long int i=0;
	edit_buf.max_pos=fsize(fd);
	edit_buf.max_lines=0;
	while (i<edit_buf.max_pos) {
		edit_data[i]=fgetc(fd);
		if (get_edit_data(i) == '\n') edit_buf.max_lines++;
		i++;		
	}		
}
//--------------------------------------
void save_edit_data(void) {
	long int i;
	for (i=0; i<edit_buf.max_pos; i++) fputc(get_edit_data(i), fd); 	
}

#elif ACCESS_VIA_DF

#include "df/spi.h"
#include "df/df.h"
#include "df/fs.h"
#include "df/common.h"

char file_name[FILENAME_MAX_LEN+1];
uint8_t *edit_data;

extern fs_t fs;
extern fs_inode_t prog_inode;

//--------------------------------------
static uint8_t get_edit_data(long int idx) {
	return edit_data[idx];
}
//--------------------------------------
static void set_edit_data(uint8_t c) {
	edit_data[edit_buf.current_pos]=c;
}
//--------------------------------------
static void insert_edit_data(uint8_t c) {
	// Platz schaffen
	edit_buf.max_pos++;
	edit_data=realloc(edit_data, edit_buf.max_pos+1);
	// ab akt. Position eine Stelle nach hinten kopieren
	memmove(&edit_data[edit_buf.current_pos+1], 
			&edit_data[edit_buf.current_pos], 
			edit_buf.max_pos - edit_buf.current_pos);
	// akt. Position neu setzen
	set_edit_data(c);
}
//--------------------------------------
static void delete_edit_data(void) {
	// ab akt. Position +1 eine Stelle nach links verschieben	
	edit_buf.max_pos--;
	memmove(&edit_data[edit_buf.current_pos], 
			&edit_data[edit_buf.current_pos+1], 
			edit_buf.max_pos - edit_buf.current_pos);
	// Platz verkleinern
	edit_data=realloc(edit_data, edit_buf.max_pos);
}
//--------------------------------------
void load_edit_data(void) {
	long int i=0;
	edit_buf.max_pos=fs_size(&fs, prog_inode);
	fs_read(&fs, prog_inode, edit_data, 0, edit_buf.max_pos);
	edit_buf.max_lines=0;
	while (i<edit_buf.max_pos) {
		if (get_edit_data(i) == '\n') edit_buf.max_lines++;
		i++;		
	}
}
//--------------------------------------
void save_edit_data(void) {
	fs_write(&fs, prog_inode, edit_data, 0, edit_buf.max_pos);
}

#else

#define FILENAME_MAX_LEN 8
char file_name[FILENAME_MAX_LEN+1];

//--------------------------------------
static uint8_t get_edit_data(long int idx) {
	return 0;
}
//--------------------------------------
static void set_edit_data(uint8_t c) {
}
//--------------------------------------
static void insert_edit_data(uint8_t c) {
}
//--------------------------------------
static void delete_edit_data(void) {
}
//--------------------------------------
void load_edit_data(void) {
}
//--------------------------------------
void save_edit_data(void) {
}

#endif
//--------------------------------------
//------- End of plattform -------------
//--------------------------------------


//--------------------------------------
//------ Begin of smed -----------------
//--------------------------------------
//--------------------------------------
static void my_itoa(int n, char s[]) {
	int i, j, sign;
	char c;
	if ((sign = n) < 0) n = -n;
	i = 0;
	do {
		s[i++] = n % 10 + '0';
	} while ((n /= 10) > 0);
	if (sign < 0) s[i++] = '-';
	s[i] = '\0';
	for (i = 0, j = strlen(s)-1; i<j; i++, j--) {
		c = s[i];
		s[i] = s[j];
		s[j] = c;
	}
}
//--------------------------------------
static void title_line(void) {
	uint8_t col;
	move (0, 0);
	attrset (A_REVERSE);
	for (col = 0; col < COLS; col++) addch (' ');
    mvaddstr_P(0, 1, PSTR(PROG_NAME));
	attrset (A_BOLD | A_REVERSE);
    mvaddstr(0, COLS/2-strlen(file_name)/2, file_name);
    attrset (A_NORMAL);
}
//--------------------------------------
static void status_line(void) {
	char buf[4];
	uint8_t col;
	move (LINES-1, 0);
	attrset (A_REVERSE);
	for (col = 0; col < COLS; col++) addch (' ');
    mvaddstr(LINES-1, COLS-strlen(PROG_AUTHOR)-1, PROG_AUTHOR);
    mvaddstr_P(LINES-1, 1, PSTR("ROW:"));
    my_itoa(edit_buf.current_line+1, buf);
    mvaddstr(LINES-1, 5, buf);
    mvaddstr_P(LINES-1, 11, PSTR("COL:"));
    my_itoa(cursor.x+1, buf);
    mvaddstr(LINES-1, 15, buf);
    mvaddstr_P(LINES-1, 21, PSTR("POS:"));
    my_itoa(edit_buf.current_pos+1, buf);
    mvaddstr(LINES-1, 25, buf);
    mvaddstr_P(LINES-1, 31, PSTR("ALL:"));
    my_itoa(edit_buf.max_pos, buf);
    mvaddstr(LINES-1, 35, buf);
    attrset (A_NORMAL);
}
//--------------------------------------
static int search_line_begin(int line) {
	int i = 0;
	int l = 0;
	while ((l<line) && (i<edit_buf.max_pos)) {
		if (get_edit_data(i) == '\n') l++;
		i++;	
	}
	return i;
}
//--------------------------------------
static void show_line(int line) {
	struct cursor_t t;
	int i = search_line_begin(line);
	edit_buf.current_pos = i;
	t.x = 0;
	t.y = cursor.y;
	while ((get_edit_data(i) != '\n') && (t.x < COLS-1)) {
		if (isprint(get_edit_data(i)) != 0 || 
			isspace(get_edit_data(i)) == ' ' ) {
			mvaddch(t.y, t.x, get_edit_data(i));
			if (t.x < COLS-1)t.x++;
			i++;
		}		
	}
}
//--------------------------------------
static void show_screen_at_line(int line) {
	struct cursor_t t;
	clear();
	int i = search_line_begin(line);
	edit_buf.current_pos = i;
	t.x = 0;
	t.y = 1;
	while ((t.y < LINES-1) && (i < edit_buf.max_pos)) {
		if (isprint(get_edit_data(i)) != 0 || 
			isspace(get_edit_data(i)) == ' ' ) {
			mvaddch(t.y, t.x, get_edit_data(i));
			if (t.x < COLS-1) t.x++;
		} else {
			if (get_edit_data(i)=='\n')	{
				t.y++;
				t.x = 0;
			}
		}
		i++;
	}
}
//--------------------------------------
static uint8_t get_line_len(int line) {
	uint8_t r = 0;
	int i = search_line_begin(line);
	while ((get_edit_data(i++) != '\n') && (i<edit_buf.max_pos)) r++;
	return r;
}
//--------------------------------------
static void calculate_cursor_x(void) {
	int temp;
	temp=get_line_len(edit_buf.current_line);
	if (cursor.x > temp) cursor.x = temp;
}
//--------------------------------------
static void calculate_current_pos(void) {
	edit_buf.current_pos=search_line_begin(edit_buf.current_line)+cursor.x;
}
//--------------------------------------
static void cursor_down(void) {
	if ((cursor.y < LINES-2) && (edit_buf.current_line < edit_buf.max_lines-1)) {
		cursor.y++;
		edit_buf.current_line++;
	} else {
		if (edit_buf.current_line < edit_buf.max_lines-1) {
			edit_buf.current_line++;
			setscrreg(1, LINES-2);
			scroll();
			show_line(edit_buf.current_line);
		}
	}
	calculate_cursor_x();
	calculate_current_pos();
}
//--------------------------------------
static void cursor_up(void) {
	if (cursor.y > 1) {
		cursor.y--;
		edit_buf.current_line--;
	} else {
		if ((edit_buf.current_line > 0)) {
			edit_buf.current_line--;
			setscrreg(1, LINES-2);
			insertln();
			show_line(edit_buf.current_line);
		}
	}
	calculate_cursor_x();
	calculate_current_pos();
}
//--------------------------------------
static void cursor_left(void) {
	if (cursor.x > 0) {
		cursor.x--;
	} else {
		if (edit_buf.current_line > 0) {
			cursor_up();
			cursor.x = get_line_len(edit_buf.current_line);
		}
	}		
	calculate_current_pos();
}
//--------------------------------------
static void cursor_right(void) {
	if ((cursor.x < COLS-1) && (cursor.x < get_line_len(edit_buf.current_line))) {
		cursor.x++;
	} else {
		if (edit_buf.current_line < edit_buf.max_lines-1) {
			cursor.x = 0;
			cursor_down();
		}
	}
	calculate_current_pos();
}
//--------------------------------------
void smed(void) {
	
	uint8_t ch, ct;
	uint8_t i;
	int temp_y;
	
	initscr ();
	// Oberflaeche initialisieren/zeichnen
	edit_buf.current_line = 0;
	edit_buf.current_pos = 0;
	show_screen_at_line(edit_buf.current_line);
	cursor.x = 0;
	cursor.y = 1;	
	// bei leerer Datei schon mal ein \n in Data-Buffer einfuegen
	if (edit_buf.max_pos == 0) {	
		insert_edit_data('\n');
		edit_buf.max_lines=1;
		cursor_right();
	}
	title_line();
	status_line();
	move(cursor.y, cursor.x);
	
	// Tastatur-Loop
	while(1) {
		
		ch = getch ();
		switch (ch)
			{
			
			case KEY_DOWN:	
				cursor_down();	
				break;
				
			case KEY_UP:	
				cursor_up();	
				break;
				
			case KEY_LEFT:	
				cursor_left();	
				break;
				
			case KEY_RIGHT:	
				cursor_right();	
				break;
				
			case KEY_NPAGE:
				if ((edit_buf.current_line+LINES-2) < edit_buf.max_lines) {
					edit_buf.current_line = edit_buf.current_line + LINES - 2;
					show_screen_at_line(edit_buf.current_line - cursor.y + 1);
				} else {
					edit_buf.current_line = edit_buf.max_lines;
					show_screen_at_line(edit_buf.current_line);
					cursor.y = 1;
				}
				calculate_cursor_x();
				calculate_current_pos();
				break;

			case KEY_PPAGE:
				if ((edit_buf.current_line-LINES+2) > 0) {
					edit_buf.current_line = edit_buf.current_line - LINES + 2;
					show_screen_at_line(edit_buf.current_line - cursor.y + 1);
				} else {
					edit_buf.current_line = 0;
					show_screen_at_line(edit_buf.current_line);
					cursor.y = 1;
				}
				calculate_cursor_x();
				calculate_current_pos();
				break;

			case KEY_CR:
				insert_edit_data('\n');
				edit_buf.max_lines++;
				clrtoeol();
				cursor_down();
				move(cursor.y, cursor.x);
				setscrreg(cursor.x, LINES-2);
				insertln();
				show_line(edit_buf.current_line);
				cursor.x = 0;
				calculate_current_pos();
				break;
				
			case KEY_TAB:
				for (i=0; i<TAB_LEN; i++) {
					insert_edit_data(' ');
					insch(' ');
					cursor_right();
				}
				break;

			case KEY_DC:
				if ((edit_buf.max_pos > 0) && 
					(edit_buf.current_pos < edit_buf.max_pos) &&
					(edit_buf.current_pos > 0)) {
					ct = get_edit_data(edit_buf.current_pos);
					delch();
					delete_edit_data();
					if (ct == '\n') {
						edit_buf.max_lines--;
						setscrreg(cursor.y+1, LINES-2);
						scroll();
						show_line(edit_buf.current_line);
						if ((edit_buf.current_line + LINES - 2 - cursor.y) < edit_buf.max_lines) {
							temp_y = cursor.y;
							cursor.y = LINES - 2;
							show_line(edit_buf.current_line + LINES - 2 - temp_y);
							cursor.y = temp_y;
						}
					}						
					calculate_current_pos();		
				}
				break;

			case KEY_BACKSPACE:
				if (edit_buf.current_pos > 0) {
					cursor_left();
					move(cursor.y, cursor.x);
					ct = get_edit_data(edit_buf.current_pos);
					delch();
					delete_edit_data();
					if (ct == '\n') {
						edit_buf.max_lines--;
						setscrreg(cursor.y+1, LINES-2);
						scroll();
						show_line(edit_buf.current_line);
						if ((edit_buf.current_line + LINES - 2 - cursor.y) < edit_buf.max_lines) {
							temp_y = cursor.y;
							cursor.y = LINES - 2;
							show_line(edit_buf.current_line + LINES - 2 - temp_y);
							cursor.y = temp_y;
						}
					}						
					calculate_current_pos();		
				}
				break;

			case KEY_IC:
				insert_edit_data(' ');
				insch(' ');
				break;
				
#ifdef ADVANCED_FUNCTIONS
			// mal so als Idee, was man noch relativ einfach
			// implementieren koennte und was auch sinnvoll waere
			// (Funktionstasten mc-like...)
			case KEY_HOME:
				cursor.x = 0;
				calculate_current_pos();		
				break;

			case KEY_END:
				if (get_line_len(edit_buf.current_line) < COLS) 
					cursor.x = get_line_len(edit_buf.current_line);
				else
					cursor.x = COLS-1;
				calculate_current_pos();		
				break;

			case KEY_F(3):	
				// Block markieren...
				break;

			case KEY_F(5):	
				// Block kopieren...
				break;

			case KEY_F(6):	
				// Block verschieben...
				break;

			case KEY_F(8):	
				// Block oder Zeile loeschen...
				break;
#endif

			case KEY_F(9):	
				endwin(); 
				clear(); 
				return; 
				break;
				
			default: // druckbares Zeichen?
				if (isprint(ch) != 0 || isspace(ch) != 0) {
					insert_edit_data(ch);
					insch(ch);
					cursor_right();
				}
				break;
			}
		title_line();
		status_line();
		move(cursor.y, cursor.x);
	}
}
//--------------------------------------
//---------End of smed------------------
//--------------------------------------


