#ifndef LCD_H
#define LCD_H


//
// Hardwarekonfiguration
//
#define LCD_DATA_DDR DDRB
#define LCD_DATA_PORT PORTB
#define LCD_DATA_PIN PINB

#define LCD_DATA_DDR_D0		DDRB
#define LCD_DATA_PORT_D0	PORTB
#define LCD_DATA_PIN_D0		PB0

#define LCD_DATA_DDR_D1		DDRB
#define LCD_DATA_PORT_D1	PORTB
#define LCD_DATA_PIN_D1		PB1

#define LCD_DATA_DDR_D2		DDRD
#define LCD_DATA_PORT_D2	PORTD
#define LCD_DATA_PIN_D2		PD2

#define LCD_DATA_DDR_D3		DDRD
#define LCD_DATA_PORT_D3	PORTD
#define LCD_DATA_PIN_D3		PD3

#define LCD_DATA_DDR_D4		DDRD
#define LCD_DATA_PORT_D4	PORTD
#define LCD_DATA_PIN_D4		PD4

#define LCD_DATA_DDR_D5		DDRD
#define LCD_DATA_PORT_D5	PORTD
#define LCD_DATA_PIN_D5		PD5

#define LCD_DATA_DDR_D6		DDRD
#define LCD_DATA_PORT_D6	PORTD
#define LCD_DATA_PIN_D6		PD6

#define LCD_DATA_DDR_D7		DDRD
#define LCD_DATA_PORT_D7	PORTD
#define LCD_DATA_PIN_D7		PD7

#define LCD_CON_DDR			DDRC
#define LCD_CON_PORT		PORTC
#define LCD_CON_PIN_RS		PC0
#define LCD_CON_PIN_RW		PC1
#define LCD_CON_PIN_E		PC2
#define LCD_CON_PIN_CS		PC3
#define LCD_CON_PIN_RESET	PC4

//
// LC7981-Kommandos
//
#define	LCD_CMD_MODE				0x00
#define LCD_CMD_CHAR_PITCH			0x01
#define LCD_CMD_NUM_CHARS			0x02
#define LCD_CMD_TIME_DIVISION		0x03
#define LCD_CMD_CURSOR_POS			0x04

#define LCD_CMD_DISPLAY_START_LA	0x08
#define LCD_CMD_DISPLAY_START_HA	0x09
#define LCD_CMD_CURSOR_LA			0x0A
#define LCD_CMD_CURSOR_HA			0x0B

#define LCD_CMD_WRITE_DATA			0x0C
#define LCD_CMD_READ_DATA			0x0D

#define LCD_CMD_CLEAR_BIT			0x0E
#define LCD_CMD_SET_BIT				0x0F

/* Bits of the LCD Mode Control Register (DB5-DB0) */
#define LCD_MODE_ON_OFF			32
#define LCD_MODE_MASTER_SLAVE	16
#define LCD_MODE_BLINK			8
#define LCD_MODE_CURSOR			4
#define LCD_MODE_MODE			2
#define LCD_MODE_EXTERNAL		1

/* Possible settings of the character pitch register */
/* Horizontal character pitches of 6, 7, and 8 */
#define LCD_CHAR_PITCH_HP_6	0x05
#define LCD_CHAR_PITCH_HP_7	0x06
#define LCD_CHAR_PITCH_HP_8	0x07

/* To indicate the state of a pixel */
#define PIXEL_ON	0xFF
#define PIXEL_OFF	0x00

//
// Prototypen
//
void lcd_xbm(int x,int y,int w,int h,unsigned char bits[],int inverted);
void lcd_write(char CMD,char DATA);
void lcd_init(int mode);
void lcd_clear();
void lcd_clear_rect(int x,int y,int w, int h);
void lcd_goto(int x, int y);
void lcd_pset(int x,int y);
void lcd_pclear(int x,int y);
void line(int x0, int y0, int x1, int y1);
void draw_char(unsigned short x, unsigned short y, char character);
void lcd_puts(int x,int y,char *text);

#endif
