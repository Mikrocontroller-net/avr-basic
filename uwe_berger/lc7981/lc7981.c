#include<util/delay.h>
#include<avr/pgmspace.h>
#include"lc7981.h"
#include"font5x7.h"


/*
LCD-Routines
*/

void write_data_port(char data)
{
	if (!bit_is_set(data, 0))
		LCD_DATA_PORT_D0 &= ~(1 << LCD_DATA_PIN_D0);
	else 
		LCD_DATA_PORT_D0 |= (1 << LCD_DATA_PIN_D0);
	if (!bit_is_set(data, 1))
		LCD_DATA_PORT_D1 &= ~(1 << LCD_DATA_PIN_D1);
	else 
		LCD_DATA_PORT_D1 |= (1 << LCD_DATA_PIN_D1);
	if (!bit_is_set(data, 2))
		LCD_DATA_PORT_D2 &= ~(1 << LCD_DATA_PIN_D2);
	else 
		LCD_DATA_PORT_D2 |= (1 << LCD_DATA_PIN_D2);
	if (!bit_is_set(data, 3))
		LCD_DATA_PORT_D3 &= ~(1 << LCD_DATA_PIN_D3);
	else 
		LCD_DATA_PORT_D3 |= (1 << LCD_DATA_PIN_D3);
	if (!bit_is_set(data, 4))
		LCD_DATA_PORT_D4 &= ~(1 << LCD_DATA_PIN_D4);
	else 
		LCD_DATA_PORT_D4 |= (1 << LCD_DATA_PIN_D4);
	if (!bit_is_set(data, 5))
		LCD_DATA_PORT_D5 &= ~(1 << LCD_DATA_PIN_D5);
	else 
		LCD_DATA_PORT_D5 |= (1 << LCD_DATA_PIN_D5);
	if (!bit_is_set(data, 6))
		LCD_DATA_PORT_D6 &= ~(1 << LCD_DATA_PIN_D6);
	else 
		LCD_DATA_PORT_D6 |= (1 << LCD_DATA_PIN_D6);
	if (!bit_is_set(data, 7))
		LCD_DATA_PORT_D7 &= ~(1 << LCD_DATA_PIN_D7);
	else 
		LCD_DATA_PORT_D7 |= (1 << LCD_DATA_PIN_D7);
}


//Write to LCD
void lcd_write(char CMD,char DATA)
{
	LCD_CON_PORT &= ~(1 << LCD_CON_PIN_CS);	//0
	LCD_CON_PORT &= ~(1 << LCD_CON_PIN_RW);	//0
	LCD_CON_PORT |= (1 << LCD_CON_PIN_RS);	//1	
	write_data_port(CMD);
	_delay_us(0.1); 						// Original: 3
	LCD_CON_PORT |= (1 << LCD_CON_PIN_E);	//1	
	_delay_us(0.8); 						//Original: 10
	LCD_CON_PORT &= ~(1 << LCD_CON_PIN_E);	//0
	_delay_us(5);
	LCD_CON_PORT &= ~(1 << LCD_CON_PIN_RS);	//0
	write_data_port(DATA);
	_delay_us(0.1); 						//Original: 5
	LCD_CON_PORT |= (1 << LCD_CON_PIN_E);	//1	
	_delay_us(0.8);  						//Original: 10
	LCD_CON_PORT &= ~(1 << LCD_CON_PIN_E);	//0
	_delay_us(5);
	LCD_CON_PORT |= (1 << LCD_CON_PIN_CS);	//1	
}

//Initialize LCD
void lcd_init(int mode)
{
	// Ausgaenge
	LCD_CON_DDR |= (1 << LCD_CON_PIN_RS);
	LCD_CON_DDR |= (1 << LCD_CON_PIN_RW);
	LCD_CON_DDR |= (1 << LCD_CON_PIN_E);
	LCD_CON_DDR |= (1 << LCD_CON_PIN_CS);
	LCD_CON_DDR |= (1 << LCD_CON_PIN_RESET);

	LCD_CON_PORT &= ~(1 << LCD_CON_PIN_RS);		//0
	LCD_CON_PORT &= ~(1 << LCD_CON_PIN_RW);		//0
	LCD_CON_PORT &= ~(1 << LCD_CON_PIN_E);		//0
	LCD_CON_PORT &= ~(1 << LCD_CON_PIN_RESET);	//0
	_delay_us(100);
	LCD_CON_PORT |= (1 << LCD_CON_PIN_RESET);	//1	
	_delay_ms(10);
	
	// Ausgaenge
	LCD_DATA_DDR_D0 |= (1 << LCD_DATA_PIN_D0);
	LCD_DATA_DDR_D1 |= (1 << LCD_DATA_PIN_D1);
	LCD_DATA_DDR_D2 |= (1 << LCD_DATA_PIN_D2);
	LCD_DATA_DDR_D3 |= (1 << LCD_DATA_PIN_D3);
	LCD_DATA_DDR_D4 |= (1 << LCD_DATA_PIN_D4);
	LCD_DATA_DDR_D5 |= (1 << LCD_DATA_PIN_D5);
	LCD_DATA_DDR_D6 |= (1 << LCD_DATA_PIN_D6);
	LCD_DATA_DDR_D7 |= (1 << LCD_DATA_PIN_D7);

	if(mode==1){
		lcd_write(LCD_CMD_MODE,0x32);//Grafikmodus
		lcd_write(LCD_CMD_CHAR_PITCH,LCD_CHAR_PITCH_HP_8);
	} else {
		lcd_write(LCD_CMD_MODE,0x3c);//Textmodus
		lcd_write(LCD_CMD_CHAR_PITCH,LCD_CHAR_PITCH_HP_8|0x70);
	}

	lcd_write(LCD_CMD_NUM_CHARS,18);
	lcd_write(LCD_CMD_TIME_DIVISION,80);
	lcd_write(LCD_CMD_DISPLAY_START_LA,0x00);
	lcd_write(LCD_CMD_DISPLAY_START_HA,0x00);
	lcd_write(LCD_CMD_CURSOR_LA,0x00);
	lcd_write(LCD_CMD_CURSOR_HA,0x00);

}

void lcd_clear()
{
	lcd_write(LCD_CMD_CURSOR_LA,0x00);
	lcd_write(LCD_CMD_CURSOR_HA,0x00);
	int i;
	for(i=0;i<1601;i++)
	{
		lcd_write(LCD_CMD_WRITE_DATA,0x00);
	}
	lcd_write(LCD_CMD_CURSOR_LA,0x00);
	lcd_write(LCD_CMD_CURSOR_HA,0x00);
}

void lcd_clear_rect(int x,int y,int w, int h)
{
	int tx,ty;
	w=(w/8);
	x=(x/8);
	for(ty=0;ty<h;ty++)
	{
		int adr=(y*20)+x;
		lcd_write(LCD_CMD_CURSOR_LA,adr&0xff);
		lcd_write(LCD_CMD_CURSOR_HA,(adr>>8)&0xff);
		for(tx=0;tx<w;tx++)
		{
			lcd_write(LCD_CMD_WRITE_DATA,0x00);
		}
		y++;
	}
}

void lcd_goto(int x, int y)//Grafikmodus
{
	int adr=(y*20)+(x/8);
	lcd_write(LCD_CMD_CURSOR_LA,adr&0xff);
	lcd_write(LCD_CMD_CURSOR_HA,(adr>>8)&0xff);
}

void lcd_pset(int x,int y)//Grafikmodus
{
	lcd_goto(x,y);
	lcd_write(LCD_CMD_SET_BIT,x&0x07);
}

void lcd_pclear(int x,int y)//Grafikmodus
{
	lcd_goto(x,y);
	lcd_write(LCD_CMD_CLEAR_BIT,x&0x07);
}

void lcd_xbm(int x,int y,int w,int h,unsigned char bits[],int inverted)//Grafikmodus
{
	int count;
	int cx,cy;

	count=0;
	for(cy=y;cy<y+h;cy++){
		lcd_goto(x,cy);
		for(cx=0;cx<(w/8);cx++)
		{
			if(inverted){
				lcd_write(LCD_CMD_WRITE_DATA,0xff^pgm_read_byte(bits+(count++)));
			}
			else
			{
				lcd_write(LCD_CMD_WRITE_DATA,pgm_read_byte(bits+(count++)));
			}
		}
	}
}

/* signum function */
int sgn(int x){
  return (x > 0) ? 1 : (x < 0) ? -1 : 0;
}
 
// Quelle: http://de.wikipedia.org/wiki/Bresenham-Algorithmus#C-Implementierung
void line(int xstart,int ystart,int xend,int yend)
{
   int x, y, t, dx, dy, incx, incy, pdx, pdy, ddx, ddy, es, el, err;
/* Entfernung in beiden Dimensionen berechnen */
   dx = xend - xstart;
   dy = yend - ystart;
/* Vorzeichen des Inkrements bestimmen */
   incx = sgn(dx);
   incy = sgn(dy);
   if(dx<0) dx = -dx;
   if(dy<0) dy = -dy;
/* feststellen, welche Entfernung größer ist */
   if (dx>dy)
   {
      /* x ist schnelle Richtung */
      pdx=incx; pdy=0;    /* pd. ist Parallelschritt */
      ddx=incx; ddy=incy; /* dd. ist Diagonalschritt */
      es =dy;   el =dx;   /* Fehlerschritte schnell, langsam */
   } else
   {
      /* y ist schnelle Richtung */
      pdx=0;    pdy=incy; /* pd. ist Parallelschritt */
      ddx=incx; ddy=incy; /* dd. ist Diagonalschritt */
      es =dx;   el =dy;   /* Fehlerschritte schnell, langsam */
   }
/* Initialisierungen vor Schleifenbeginn */
   x = xstart;
   y = ystart;
   err = el/2;
   lcd_pset(x,y);
/* Pixel berechnen */
   for(t=0; t<el; ++t) /* t zaehlt die Pixel, el ist auch Anzahl */
   {
      /* Aktualisierung Fehlerterm */
      err -= es; 
      if(err<0)
      {
          /* Fehlerterm wieder positiv (>=0) machen */
          err += el;
          /* Schritt in langsame Richtung, Diagonalschritt */
          x += ddx;
          y += ddy;
      } else
      {
          /* Schritt in schnelle Richtung, Parallelschritt */
          x += pdx;
          y += pdy;
      }
      lcd_pset(x,y);
   }
} /* gbham() */

//Quelle: www.frozeneskimo.com/samsunglcd/
void draw_char(unsigned short x, unsigned short y, char character) {//Grafikmodus
	unsigned char fontIndex, i, j;

	/* The 5x7 character set starts at the '!' character (ASCII index 
	* number 33) so we subtract 32 from the ASCII character to find the 
	* index in the 5x7 font table. */	
	fontIndex = character-32;
	/* If the index is out of bounds, bail out */
	if (fontIndex > 94)
	return;
	
	for (i = 0; i < FONT_WIDTH; i++) {
		for (j = 0; j < FONT_HEIGHT; j++) {
			/* Check if the bit/pixel is set, paint accoringly to 
			* the screen */
			if (pgm_read_byte(&Font5x7[FONT_WIDTH*fontIndex+i]) & (1<<j))
			lcd_pset(x, y+j);
			else	
			lcd_pclear(x, y+j);
		}
		/* Move the LCD cursor through the font width as well */
		x++;
	}
	for(j = 0; j < FONT_HEIGHT; j++){lcd_pclear(x,y+j);}
}

void lcd_puts(int x,int y,char *text)
{
	int xt=x;
	while(*text)
	{
		draw_char(xt,y,(char)*text);
		text++;
		xt+=(FONT_WIDTH+1);
	}

}

