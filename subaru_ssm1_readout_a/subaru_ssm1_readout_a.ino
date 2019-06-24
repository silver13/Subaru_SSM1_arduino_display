
// subaru SSM 1 read and display
// the ecu locations are hardcoded for my particular rom type
// 7232A5 Subaru Liberty 1992 Australia/ UK

// using a 0.96 Oled display i2c communication
// Arduino Mini pro so the built in USB serial does not interfere
// Serial on pins 0, 1, connect Arduino RXD > Ecu TX and viceversa
// Using 1K resistors in series on the serial lines for extra protection
// - the 90's computer might not have the features we take for granted today

// -1 means communication error


#include "U8glib.h"
#include <EEPROM.h>

U8GLIB_SSD1306_128X64 u8g(U8G_I2C_OPT_NONE);

// average for fuel consumption
const byte AVERAGESIZE = 128;

class average
{	
public:
	int step(int);	
	int avg[AVERAGESIZE];
	byte avgcount;
	byte avgfirstcount;
	average()
	{
	avgcount = 0;
	avgfirstcount = 1;
	}

} lpf1 ;

int average::step( int value)
{
 long average = 0; 
 avg[avgcount] = value;
 avgcount++;
 if ( avgcount >= AVERAGESIZE )
  {
    avgfirstcount = 0;
    avgcount = 0;
  } 
 if (!avgfirstcount)
 {
   for ( int i = 0; i < AVERAGESIZE; i++)
   {
    average += avg[i];  
   }
   return average / AVERAGESIZE ;
 } else return 0 ;
 
}

	

// mode save eeprom location
#define EEPROM_MODE 39
#define button_pin 10
// screen offset from the edge to fix a display issue on my screen
#define OFFSET 3

byte rx[3];
byte mode;
byte count = 0;

void setup(void) {

// set baudrate and parity
 Serial.begin(1953);
 Serial.setTimeout(300); 
 UCSR0C = ( UCSR0C &~_BV(UPM00) | _BV(UPM01) ); 
  
pinMode( button_pin, INPUT_PULLUP ); 

mode = EEPROM.read( EEPROM_MODE );

//if ( !digitalRead( button_pin) )
{
//  display rom id
  u8g.firstPage();  
  do {
      
  u8g.setFont(u8g_font_fub11); 
  u8g.setPrintPos(OFFSET, 11); 
   u8g.print("*Online");
   if (ECU_GetROMID(rx))
  {
    u8g.setPrintPos( OFFSET, 25); 
  
    u8g.print(rx[0], 16);
    u8g.print(rx[1], 16);
    u8g.print(rx[2], 16); 
  }
      
  } while( u8g.nextPage() );


delay(2000);
}

}



void loop(void) {

 int readout = 0;  
 byte pressed = 0; 
  
  mode%=12;

  button_save( mode);
  
//mode = 1; // force mode

  if ( mode == 0) readout = ecu_read(0x1310); // 02
  if ( mode == 1) readout = ecu_read(0x1337); // temp
  if ( mode == 2) readout = ecu_read(0x133e); // a/f trim
  if ( mode == 3) readout = ecu_read(0x1335); // vbatt
  if ( mode == 4) readout = ecu_read(0x1336); // speed*2
  if ( mode == 5) readout = ecu_read(0x1307); // AFM *5/256
  if ( mode == 6) readout = ecu_read(0x1329); // thr *5/256
  if ( mode == 7) readout = ecu_read(0x1344); // switches
  if ( mode == 8) readout = ecu_read(0x1328); // timing retard
  if ( mode == 9) readout = ecu_read(0x1306); // inj pulse *256/1000
  if ( mode == 10|| mode == 11)
     {
       // fuel consumption / distance calculation
       int km, rpm = 0;
       readout = ecu_read(0x1306); // inj pulse *256/1000
       km = ecu_read(0x1336);  // km / 2
       rpm = ecu_read(0x1338); // rpm * 25
       
       if ( readout >= 0&& km >= 0 && rpm >= 0 )
       {
         readout = ( readout* rpm )/ (km+2);
         int temp = lpf1.step( readout);
         if (mode == 11) readout = temp;
       }
     }

  
pressed = 0;
  u8g.firstPage();  
  do {
    draw( readout, count );
    if ( buttonread() ) 
    {     
       pressed = 1;
    }
      
  } while( u8g.nextPage() );

if ( pressed ) mode++;  
  count++;
}


void draw( int readout, byte count) { 
  
  u8g.setFont(u8g_font_fub11); 
  u8g.setPrintPos( OFFSET, 11); 
  
  u8g.print(rx[0], 16);
  u8g.print(rx[1], 16);
  u8g.print(rx[2], 16); 
      
  if( count%2 == 1) u8g.print("*");
  if( mode == 0) u8g.print(".");
  else u8g.print("  ");
  u8g.setPrintPos(OFFSET, 33);
  u8g.setFont(u8g_font_fub20);
  
  switch(mode)
  {
    case 0:       
      if (readout >= 0) 
      {
       // u8g.print( (float) readout*9.76);
        u8g.print( (float) readout*19.52);
        u8g.print(" mV");
      }
      else u8g.print(readout);
      u8g.setFont(u8g_font_fub11); 
      u8g.setPrintPos(OFFSET, 46);
      u8g.print("O2");
    break;
    
    case 1:      
      if (readout >= 0) 
      {
        u8g.print(readout - 50);
        u8g.print("C");
      }
      else u8g.print(readout);
      u8g.setFont(u8g_font_fub11); 
      u8g.setPrintPos(OFFSET, 46);
      u8g.print("Temp");
    break;
    
    case 2:
   
      if (readout >= 0) 
      {
        u8g.print(readout - 128);
        u8g.print("%");
      }
      else u8g.print(readout);
      u8g.setFont(u8g_font_fub11); 
      u8g.setPrintPos(OFFSET, 46);
      u8g.print("AFcorr");
    break;
    
     case 3:
      
      if (readout >= 0) 
      {
        u8g.print( (float)readout*0.08);
        u8g.print("V");
      }
      else u8g.print(readout);
      u8g.setFont(u8g_font_fub11); 
      u8g.setPrintPos(OFFSET, 46);
      u8g.print("Batt");
    break;
    
     case 4:
      
      if (readout >= 0) 
      {
        u8g.print( readout*2);
        u8g.print("Kmh");
      }
      else u8g.print(readout);
      u8g.setFont(u8g_font_fub11); 
      u8g.setPrintPos(OFFSET, 46);
      u8g.print("Spd");
    break;
    
    case 5:
      
      if (readout >= 0) 
      {
        u8g.print( (float) readout *5.0 / 256);
        u8g.print(" V");
      }
      else u8g.print(readout);
      u8g.setFont(u8g_font_fub11); 
      u8g.setPrintPos(OFFSET, 46);
      u8g.print("AFM");
    break;
    
    case 6:
      
      if (readout >= 0) 
      {
        u8g.print( (float)  readout*5/256);
        u8g.print(" V");
      }
      else u8g.print(readout);
      u8g.setFont(u8g_font_fub11); 
      u8g.setPrintPos(OFFSET, 46);
      u8g.print("THR");
    break;
    
    case 7:    
      if ( readout > 0)
      {
        if (readout & B00000010) 
        {
          u8g.print( "K");
         
        }else  u8g.print(" ");
        
        if (readout & B10000000) 
        {
          u8g.print( "I");
         
        }else  u8g.print(" ");
        
        if (readout & B00010000) 
        {
          u8g.print( "F");
         
        }else  u8g.print(" ");
        if (readout & B00000100) 
        {
          u8g.print( "X");
         
        }else  u8g.print(" ");
        
      }else  u8g.print("-");
      
     // else u8g.print(readout);
      u8g.setFont(u8g_font_fub11); 
      u8g.setPrintPos(OFFSET, 46);
      u8g.print("SW");
    break;

    case 8:
      
      if (readout >= 0) 
      {
        u8g.print( readout);
        u8g.print(" %");
      }
      else u8g.print(readout);
      u8g.setFont(u8g_font_fub11); 
      u8g.setPrintPos(OFFSET, 46);
      u8g.print("KNOCK");
    break;
    
    case 9:
      
      if (readout >= 0) 
      {
        u8g.print( readout/4);
        u8g.print(" mS");
      }
      else u8g.print(readout);
      u8g.setFont(u8g_font_fub11); 
      u8g.setPrintPos(OFFSET, 46);
      u8g.print("INJ");
    break;
    
    case 10:
    case 11:  
      if (readout >= 0) 
      {
        u8g.print( readout);
        //u8g.print("Fuel");
      }
      else u8g.print(readout);     
      
      u8g.setFont(u8g_font_fub11); 
      u8g.setPrintPos(OFFSET, 46);
      u8g.print("ECO");
      if ( mode == 11 )u8g.print(" LPF");
    break;
    
    default:  
      u8g.setFont(u8g_font_fub11); 
      u8g.setPrintPos(OFFSET, 46);
      u8g.print("NONE");
    break;
    
  }

}


void ECU_Stop() {
  byte txbuf[4]={0x12,0x00,0x00,0x00};
  
  Serial.write(txbuf[0]);
  Serial.write(txbuf[1]);
  Serial.write(txbuf[2]);
  Serial.write(txbuf[3]);
  
  delay(50);
  
  Serial.flush();
}

// read form ecu
// negative means error

int ecu_read(unsigned int addr) {
byte txbuf[4]={0x78, addr>>8 , addr&0xFF ,0x00};
  
  ECU_Stop();
  while( Serial.read() >= 0);
  
  Serial.write(txbuf[0]);
  Serial.write(txbuf[1]);
  Serial.write(txbuf[2]);
  Serial.write(txbuf[3]);
  
  delay(50);
  
  Serial.flush(); // wait to complete tx
char response[3]={0};

int num = Serial.readBytes( response, 3);
rx[0] = response[0];
rx[1] = response[1];
rx[2] = response[2];

if ( num != 3 ) return -1;
if( response[0] == (addr>>8)&0xff && response[1] == (char) addr&0xFF ) return (byte) response[2];
return -2;
}

// read ecu rom id
// for example (hex) 72 32 A5
boolean ECU_GetROMID(byte * buffer) {
  char readCmd[4] ={0x78,0x00,0x00,0x00};
  char romidCmd[4]={0x00,0x46,0x48,0x49};
  char romid[3]   ={0};

  ECU_Stop();
  while( Serial.read() >= 0) ;

//  Serial.write(readCmd[0]);
//  Serial.write(readCmd[1]);
//  Serial.write(readCmd[2]);
//  Serial.write(readCmd[3]);
  ecu_read(0x1337);
  
  int retries = 0;
  while (retries<8) {

    
    Serial.write(romidCmd[0]);
    Serial.write(romidCmd[1]);
    Serial.write(romidCmd[2]);
    Serial.write(romidCmd[3]);
    
    int nbytes = Serial.readBytes(romid,3);
    
    if ((nbytes == 3) &&(romid[0]!=0x00))
      break;
    ++retries;
  }
 // ECU_Stop();
  
  buffer[0] = romid[0];
  buffer[1] = romid[1];
  buffer[2] = romid[2];
  
  if (romid[0] == 0x00) {
    return false;
  }
  
  return true;
}




void button_save( byte in)
{
static unsigned long lasttime = millis();

if (  EEPROM.read( EEPROM_MODE) != in)
{
 if ( millis() - lasttime > 10000 )
 {
   EEPROM.write( EEPROM_MODE , in);
   lasttime = millis();
 }  
}

}


byte buttonread()
{
  byte on = 0;
  byte button = digitalRead( button_pin);
  static byte lastbutton = 0;
  
  if ( button != lastbutton && button == 0) on = 1;
  lastbutton = button; 
  return on;
}


