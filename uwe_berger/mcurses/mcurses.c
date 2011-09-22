/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * @file mcurses.c - mcurses lib
 *
 * Copyright (c) 2011 Frank Meyer - frank(at)fli4l.de
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

#ifdef unix
#include <termio.h>
#include <fcntl.h>
#define PROGMEM
#define PSTR(x)                                 (x)
#define pgm_read_byte(s)                        (*s)
#else
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include "uart/usart.h"
#endif

#include "mcurses.h"

#define SEQ_CSI                                 PSTR("\033[")                   // code introducer
#define SEQ_CLEAR                               PSTR("\033[2J")                 // clear screen
#define SEQ_CLRTOBOT                            PSTR("\033[J")                  // clear to bottom
#define SEQ_CLRTOEOL                            PSTR("\033[K")                  // clear to end of line
#define SEQ_DELCH                               PSTR("\033[P")                  // delete character
#define SEQ_NEXTLINE                            PSTR("\033E")                   // goto next line (scroll up at end of scrolling region)
#define SEQ_INSERTLINE                          PSTR("\033[L")                  // insert line
#define SEQ_DELETELINE                          PSTR("\033[M")                  // delete line
#define SEQ_ATTRSET                             PSTR("\033[0")                  // set attributes, e.g. "\033[0;7;1m"
#define SEQ_ATTRSET_REVERSE                     PSTR(";7")                      // reverse
#define SEQ_ATTRSET_UNDERLINE                   PSTR(";4")                      // underline
#define SEQ_ATTRSET_BLINK                       PSTR(";5")                      // blink
#define SEQ_ATTRSET_BOLD                        PSTR(";1")                      // bold
#define SEQ_ATTRSET_DIM                         PSTR(";2")                      // dim
#define SEQ_ATTRSET_FCOLOR                      PSTR(";3")                      // forground color
#define SEQ_ATTRSET_BCOLOR                      PSTR(";4")                      // background color
#define SEQ_INSERT_MODE                         PSTR("\033[4h")                 // set insert mode
#define SEQ_REPLACE_MODE                        PSTR("\033[4l")                 // set replace mode
#define SEQ_RESET_SCRREG                        PSTR("\033[r")                  // reset scrolling region
#define SEQ_LOAD_G1                             PSTR("\033)0")                  // load G1 character set
#define SEQ_CURSOR_VIS                          PSTR("\033[?25")                // set cursor visible/not visible

static uint8_t                                  mcurses_scrl_start = 0;         // start of scrolling region, default is 0
static uint8_t                                  mcurses_scrl_end = LINES - 1;   // end of scrolling region, default is last line
static uint8_t                                  mcurses_nodelay;                // nodelay flag
uint8_t                                         mcurses_cury;                   // current y position of cursor, public (getyx())
uint8_t                                         mcurses_curx;                   // current x position of cursor, public (getyx())

#ifdef unix

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * PHYIO: init, done, putc, getc, nodelay, flush for UNIX or LINUX
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
static struct termio                            mcurses_oldmode;
static struct termio                            mcurses_newmode;

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * PHYIO: init
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
static void
mcurses_phyio_init (void)
{
    int     fd;

    fd = fileno (stdin);

    (void) ioctl (fd, TCGETA, &mcurses_oldmode);
    (void) ioctl (fd, TCGETA, &mcurses_newmode);

    mcurses_newmode.c_lflag &= ~ICANON;                                         // switch off canonical input
    mcurses_newmode.c_lflag &= ~ECHO;                                           // switch off echo
    mcurses_newmode.c_iflag &= ~ICRNL;                                          // switch off CR->NL mapping
    mcurses_newmode.c_oflag &= ~TAB3;                                           // switch off TAB conversion
    mcurses_newmode.c_cc[VINTR] = '\377';                                       // disable VINTR VQUIT
    mcurses_newmode.c_cc[VQUIT] = '\377';                                       // but don't touch VSWTCH
    mcurses_newmode.c_cc[VMIN] = 1;                                             // block input:
    mcurses_newmode.c_cc[VTIME] = 0;                                            // one character
    (void) ioctl (fd, TCSETAW, &mcurses_newmode);
}

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * PHYIO: done
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
static void
mcurses_phyio_done (void)
{
    int     fd;

    fd = fileno (stdin);

    (void) ioctl (fd, TCSETAW, &mcurses_oldmode);
}

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * PHYIO: putc
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
static void
mcurses_phyio_putc (uint8_t ch)
{
    putchar (ch);
}

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * PHYIO: getc
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
static uint8_t
mcurses_phyio_getc (void)
{
    uint8_t ch;

    ch = getchar ();

    return (ch);
}

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * PHYIO: set/reset nodelay
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
static void
mcurses_phyio_nodelay (uint8_t flag)
{
    int     fd;
    int     fl;

    fd = fileno (stdin);

    if ((fl = fcntl (fd, F_GETFL, 0)) >= 0)
    {
        if (flag)
        {
            fl |= O_NDELAY;
        }
        else
        {
            fl &= ~O_NDELAY;
        }
        (void) fcntl (fd, F_SETFL, fl);
        mcurses_nodelay = flag;
    }
}

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * PHYIO: flush output
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
static void
mcurses_phyio_flush_output ()
{
    fflush (stdout);
}

#else

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * PHYIO: init, done, putc, getc for AVR
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * PHYIO: init
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
static void
mcurses_phyio_init (void)
{

}

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * PHYIO: done
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
static void
mcurses_phyio_done (void)
{
}

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * PHYIO: putc
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
static void
mcurses_phyio_putc (uint8_t ch)
{
	usart_write_char(ch);
}

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * PHYIO: getc
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
static uint8_t
mcurses_phyio_getc (void)
{
	return usart_receive_char();
}

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * PHYIO: set/reset nodelay
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
static void
mcurses_phyio_nodelay (uint8_t flag)
{
    mcurses_nodelay = flag;
}

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * PHYIO: flush output
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
static void
mcurses_phyio_flush_output ()
{
    while (usart_tx_not_empty())                                                     // tx buffer empty?
    {
        ;                                                                       // no, wait
    }
}


#endif // !unix

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * INTERN: put a character (raw)
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
static void
mcurses_putc (uint8_t ch)
{
    mcurses_phyio_putc (ch);
}

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * INTERN: put a string (raw)
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
static void
mcurses_puts (char * str)
{
    while (*str)
    {
        mcurses_putc (*str++);
    }
}

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * INTERN: put a string from flash (raw)
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
static void
mcurses_puts_P (const char * str)
{
    uint8_t ch;

    while ((ch = pgm_read_byte(str)) != '\0')
    {
        mcurses_putc (ch);
        str++;
    }
}

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * INTERN: put an integer number (raw)
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
static void
mcurses_puti (uint8_t i)
{
    char buf[4];
#ifdef unix
    sprintf (buf, "%d", i);
#else
    itoa (i, buf, 10);
#endif
    mcurses_puts (buf);
}

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * INTERN: addch or insch a character
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
#define CHARSET_G0      0
#define CHARSET_G1      1

static void
mcurses_addch_or_insch (uint8_t ch, uint8_t insert)
{
    static uint8_t  charset = 0xff;
    static uint8_t  insert_mode = FALSE;

    if (ch >= 0x80 && ch <= 0x9F)
    {
        if (charset != CHARSET_G1)
        {
            mcurses_putc ('\016');                                              // switch to G1 set
            charset = CHARSET_G1;
        }
        ch -= 0x20;                                                             // subtract offset to G1 characters
    }
    else
    {
        if (charset != CHARSET_G0)
        {
            mcurses_putc ('\017');                                              // switch to G0 set
            charset = CHARSET_G0;
        }
    }

    if (insert)
    {
        if (! insert_mode)
        {
            mcurses_puts_P (SEQ_INSERT_MODE);
            insert_mode = TRUE;
        }
    }
    else
    {
        if (insert_mode)
        {
            mcurses_puts_P (SEQ_REPLACE_MODE);
            insert_mode = FALSE;
        }
    }

    mcurses_putc (ch);
    mcurses_curx++;
}

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * INTERN: set scrolling region (raw)
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
static void
mysetscrreg (uint8_t top, uint8_t bottom)
{
    if (top == bottom)
    {
        mcurses_puts_P (SEQ_RESET_SCRREG);                                      // reset scrolling region
    }
    else
    {
        mcurses_puts_P (SEQ_CSI);
        mcurses_puti (top + 1);
        mcurses_putc (';');
        mcurses_puti (bottom + 1);
        mcurses_putc ('r');
    }
}

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * move cursor (raw)
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
static void
mymove (uint8_t y, uint8_t x)
{
    mcurses_puts_P (SEQ_CSI);
    mcurses_puti (y + 1);
    mcurses_putc (';');
    mcurses_puti (x + 1);
    mcurses_putc ('H');
}

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * MCURSES: initialize
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
void
initscr (void)
{
    mcurses_phyio_init ();
    mcurses_puts_P (SEQ_LOAD_G1);                                               // load graphic charset into G1
    clear ();
    move (0, 0);
}

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * MCURSES: add character
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
void
addch (uint8_t ch)
{
    mcurses_addch_or_insch (ch, FALSE);
}

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * MCURSES: add string
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
void
addstr (char * str)
{
    while (*str)
    {
        mcurses_addch_or_insch (*str++, FALSE);
    }
}

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * MCURSES: add string
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
void
addstr_P (const char * str)
{
    uint8_t ch;

    while ((ch = pgm_read_byte(str)) != '\0')
    {
        mcurses_addch_or_insch (ch, FALSE);
        str++;
    }
}

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * MCURSES: set attribute(s)
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
void
attrset (uint16_t attr)
{
    uint8_t         idx;

    mcurses_puts_P (SEQ_ATTRSET);

    idx = (attr & F_COLOR) >> 8;

    if (idx >= 1 && idx <= 8)
    {
        mcurses_puts_P (SEQ_ATTRSET_FCOLOR);
        mcurses_putc (idx - 1 + '0');
    }

    idx = (attr & B_COLOR) >> 12;

    if (idx >= 1 && idx <= 8)
    {
        mcurses_puts_P (SEQ_ATTRSET_BCOLOR);
        mcurses_putc (idx - 1 + '0');
    }

    if (attr & A_REVERSE)
    {
        mcurses_puts_P (SEQ_ATTRSET_REVERSE);
    }
    if (attr & A_UNDERLINE)
    {
        mcurses_puts_P (SEQ_ATTRSET_UNDERLINE);
    }
    if (attr & A_BLINK)
    {
        mcurses_puts_P (SEQ_ATTRSET_BLINK);
    }
    if (attr & A_BOLD)
    {
        mcurses_puts_P (SEQ_ATTRSET_BOLD);
    }
    if (attr & A_DIM)
    {
        mcurses_puts_P (SEQ_ATTRSET_DIM);
    }
    mcurses_putc ('m');
}

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * MCURSES: move cursor
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
void
move (uint8_t y, uint8_t x)
{
    mcurses_cury = y;
    mcurses_curx = x;
    mymove (y, x);
}

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * MCURSES: delete line
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
void
deleteln (void)
{
    mysetscrreg (mcurses_scrl_start, mcurses_scrl_end);                         // set scrolling region
    mymove (mcurses_cury, 0);                                                   // goto to current line
    mcurses_puts_P (SEQ_DELETELINE);                                            // delete line
    mysetscrreg (0, 0);                                                         // reset scrolling region
    move (mcurses_cury, mcurses_curx);                                          // restore position
}

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * MCURSES: insert line
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
void
insertln (void)
{
    mysetscrreg (mcurses_cury, mcurses_scrl_end);                               // set scrolling region
    mymove (mcurses_cury, 0);                                                   // goto to current line
    mcurses_puts_P (SEQ_INSERTLINE);                                            // insert line
    mysetscrreg (0, 0);                                                         // reset scrolling region
    mymove (mcurses_cury, mcurses_curx);                                        // restore position
}

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * MCURSES: scroll
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
void
scroll (void)
{
    mysetscrreg (mcurses_scrl_start, mcurses_scrl_end);                         // set scrolling region
    mymove (mcurses_scrl_end, 0);                                               // goto to last line of scrolling region
    mcurses_puts_P (SEQ_NEXTLINE);                                              // next line
    mysetscrreg (0, 0);                                                         // reset scrolling region
    mymove (mcurses_cury, mcurses_curx);                                        // restore position
}

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * MCURSES: clear
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
void
clear (void)
{
    mcurses_puts_P (SEQ_CLEAR);
}

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * MCURSES: clear to bottom of screen
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
void
clrtobot (void)
{
    mcurses_puts_P (SEQ_CLRTOBOT);
}

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * MCURSES: clear to end of line
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
void
clrtoeol (void)
{
    mcurses_puts_P (SEQ_CLRTOEOL);
}

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * MCURSES: delete character at cursor position
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
void
delch (void)
{
    mcurses_puts_P (SEQ_DELCH);
}

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * MCURSES: insert character
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
void
insch (uint8_t ch)
{
    mcurses_addch_or_insch (ch, TRUE);
}

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * MCURSES: set scrolling region
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
void
setscrreg (uint8_t t, uint8_t b)
{
    mcurses_scrl_start = t;
    mcurses_scrl_end = b;
}

void
curs_set (uint8_t visibility)
{
    mcurses_puts_P (SEQ_CURSOR_VIS);

    if (visibility == 0)
    {
        mcurses_putc ('h');
    }
    else
    {
        mcurses_putc ('l');
    }
}


/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * MCURSES: refresh: flush output
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
void
refresh (void)
{
    mcurses_phyio_flush_output ();
}

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * MCURSES: set/reset nodelay
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
void
nodelay (uint8_t flag)
{
    if (mcurses_nodelay != flag)
    {
        mcurses_phyio_nodelay (flag);
    }
}

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * MCURSES: read key
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
#define MAX_KEYS                ((KEY_F1 + 12) - 0x80)

static const char * function_keys[MAX_KEYS] =
{
    "B",                        // KEY_DOWN                 0x80                // Down arrow key
    "A",                        // KEY_UP                   0x81                // Up arrow key
    "D",                        // KEY_LEFT                 0x82                // Left arrow key
    "C",                        // KEY_RIGHT                0x83                // Right arrow key
    "1~",                       // KEY_HOME                 0x84                // Home key
    "3~",                       // KEY_DC                   0x85                // Delete character key
    "2~",                       // KEY_IC                   0x86                // Ins char/toggle ins mode key
    "6~",                       // KEY_NPAGE                0x87                // Next-page key
    "5~",                       // KEY_PPAGE                0x88                // Previous-page key
    "4~",                       // KEY_END                  0x89                // End key
    "Z",                        // KEY_BTAB                 0x8A                // Back tab key
#if 0 // VT400:
    "11~",                      // KEY_F(1)                 0x8B                // Function key F1
    "12~",                      // KEY_F(2)                 0x8C                // Function key F2
    "13~",                      // KEY_F(3)                 0x8D                // Function key F3
    "14~",                      // KEY_F(4)                 0x8E                // Function key F4
    "15~",                      // KEY_F(5)                 0x8F                // Function key F5
#else // Linux console
    "[A",                       // KEY_F(1)                 0x8B                // Function key F1
    "[B",                       // KEY_F(2)                 0x8C                // Function key F2
    "[C",                       // KEY_F(3)                 0x8D                // Function key F3
    "[D",                       // KEY_F(4)                 0x8E                // Function key F4
    "[E",                       // KEY_F(5)                 0x8F                // Function key F5
#endif
    "17~",                      // KEY_F(6)                 0x90                // Function key F6
    "18~",                      // KEY_F(7)                 0x91                // Function key F7
    "19~",                      // KEY_F(8)                 0x92                // Function key F8
    "20~",                      // KEY_F(9)                 0x93                // Function key F9
    "21~",                      // KEY_F(10)                0x94                // Function key F10
    "23~",                      // KEY_F(11)                0x95                // Function key F11
    "24~"                       // KEY_F(12)                0x96                // Function key F12
};

uint8_t
getch (void)
{
    char    buf[4];
    uint8_t ch;
    uint8_t idx;

    refresh ();
    ch = mcurses_phyio_getc ();

    if (ch == 0x7F)                                                             // BACKSPACE on VT200 sends DEL char
    {
        ch = KEY_BACKSPACE;                                                     // map it to '\b'
    }
    else if (ch == '\033')                                                      // ESCAPE
    {
        while ((ch = mcurses_phyio_getc ()) == ERR)
        {
            ;
        }

        if (ch == '\033')                                                       // 2 x ESCAPE
        {
            return KEY_ESCAPE;
        }
        else if (ch == '[')
        {
            for (idx = 0; idx < 3; idx++)
            {
                while ((ch = mcurses_phyio_getc ()) == ERR)
                {
                    ;
                }

                buf[idx] = ch;

                if ((ch >= 'A' && ch <= 'Z') || ch == '~')
                {
                    idx++;
                    break;
                }
            }

            buf[idx] = '\0';

            for (idx = 0; idx < MAX_KEYS; idx++)
            {
                if (! strcmp (buf, function_keys[idx]))
                {
                    ch = idx + 0x80;
                    break;
                }
            }

            if (idx == MAX_KEYS)
            {
                ch = KEY_ESCAPE;
            }
        }
    }

    return ch;
}

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * MCURSES: endwin
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
void
endwin (void)
{
    move (LINES - 1, 0);                                                        // move cursor to last line
    clrtoeol ();                                                                // clear this line
    mcurses_putc ('\017');                                                      // switch to G0 set
    refresh ();                                                                 // flush output
    mcurses_phyio_done ();                                                      // end of physical I/O
}
