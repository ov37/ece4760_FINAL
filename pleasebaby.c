/*
 * File:        DotStar test
 * Author:      Bruce Land
 * For use with Sean Carroll's Little Board
 */

////////////////////////////////////
// clock AND protoThreads configure!
// You MUST check this file!
#include "config_1_3_2.h"
// threading library
#include "pt_cornell_1_3_2.h"
#include <stdlib.h>
#include <math.h>


// graphics libraries
//#include "tft_master.h"
//#include "tft_gfx.h"

// APA102 datasheet:
// 32 bits of zeros is a start frame
// 32 bits of ones is a stop frame
// LED frame:
// 111_5bit_global_intensity_8bitBlue_8bitGreen_8bitRed
// so 0xff_00_ff_00 is full intnsity green
#define START_FRAME 0x00000000
#define STOP_FRAME  0xffffffff
#define PIXEL_FRAME(i,r,g,b)(0xe0000000 | (((0x1f & (i)))<<24) | ((0xff & (b))<<16) | ((0xff & (g))<<8) | (0xff & (r)))
//#define PIXEL_FRAME(i,r,g,b)(0xe0000000 | ((i)<<24) | ((b)<<16) | ((g)<<8) | (r))
#define FULL_ON 0x1e
#define HALF_ON 0x0f
#define QUAR_ON 0x07

// number of pixels
#define PixelNum 144
#define EnablePullDownA(bits) CNPUACLR =bits; CNPDASET = bits;
#define DisablePullDownA(bits) CNPDACLR =bits;
#define EnablePullDownB(bits) CNPUBCLR=bits; CNPDBSET=bits;
#define DisablePullDownB(bits) CNPDBCLR =bits;
#define EnablePullUpB(bits) CNPDBCLR=bits; CNPUBSET=bits;
#define DisablePullUpB(bits) CNPUBCLR=bits;

char buffer[60];

//clock = 40MHz, can change
volatile int spiClkDiv = 4;
volatile unsigned int SPI1_DATA;
typedef struct pixel pixel;
struct pixel{
    char red;
    char green;
    char blue;
    char intensity;
};
// and the whole string
pixel pixel_array[PixelNum];




//The actual period of the wave
int generate_period = 40000 ;
int pwm_on_time = 0 ;
// == Timer 2 ISR =====================================================
// just toggles a pin for timeing strobe
void __ISR(_TIMER_2_VECTOR, ipl2) Timer2Handler(void)
{
    // generate a trigger strobe for timing other events
    mPORTBSetBits(BIT_0);
    // clear the timer interrupt flag
    mT2ClearIntFlag();
    mPORTBClearBits(BIT_0);
}



// === display the LEDs ===========================================
// copies the contents of pixel_array to SPI
void write_pixels(void){ 

    // start frame
    WriteSPI2(START_FRAME);
    // wait for end of transaction
    while (SPI2STATbits.SPIBUSY); 
    
    int i;
    //payload
    for (i=0; i<PixelNum; i++){
        WriteSPI2(PIXEL_FRAME(pixel_array[i].intensity, pixel_array[i].red, pixel_array[i].green, pixel_array[i].blue));
        // wait for end of transaction
        while (SPI2STATbits.SPIBUSY); 
    }
    //stop frame
    WriteSPI2(STOP_FRAME);
    // wait for end of transaction
    while (SPI2STATbits.SPIBUSY); 
}

void read_pixeldata(char* r, char* g, char* b ){
    //Test if receive data available
    
    //wait for chip select
 //while(!DataRdySPI1());
    //test sending one character from pi
  // SPI1_DATA=  ReadSPI1();
  // WriteSPI1(SPI1_DATA);
  // while(SPI1STATbits.SPIBUSY);
 //  int junk =ReadSPI1();
   //test sending spi1 to spi2
   //chip select on level shifter if wanting to run led strip and tft
   // if not use uart communication
      
   //Parallel communication
            
            int enable= PORTReadBits(IOPORT_A,BIT_0);
            int r0 =0;
            int r1 = 0;
            int r2 = 0;
            int g0= 0;
            int g1= 0;
            int g2 = 0;
            int b0= 0;
            int b1= 0;
   
   //int i=0;
   int rcolor = 0;
   int gcolor = 0;
   int bcolor = 0;
  
   while(!enable){
       enable= PORTReadBits(IOPORT_A,BIT_0);
       if(PORTReadBits(IOPORT_B,BIT_1)==0){
            SetDCOC3PWM(0);
       }
   }

         r0 = PORTReadBits(IOPORT_B,BIT_3);
         r1 = PORTReadBits(IOPORT_B,BIT_7);
         r2 = PORTReadBits(IOPORT_B,BIT_13);
         g0= PORTReadBits(IOPORT_B,BIT_10);
         g1= PORTReadBits(IOPORT_B,BIT_2);
         g2 = PORTReadBits(IOPORT_B,BIT_5);
         b0= PORTReadBits(IOPORT_B,BIT_4);
         b1= PORTReadBits(IOPORT_A,BIT_1);
         //0,3,4,5,7,13,10,9,11,13
         
        rcolor = ((r2>0)*4 )+((r1>0)*2) + (r0>0);
        gcolor= ((g2>0)*4 )+((g1>0)*2) + (g0>0);
        bcolor= ((b1>0)*2) + (b0>0);
         
         *r=rcolor *36;
         *g=gcolor*36;
         *b=bcolor*85;
         while(enable){
             enable= PORTReadBits(IOPORT_A,BIT_0);
         };

    
}

// === write a RGBI value to the pixel array =======================
void set_pixel_rgb(int i, char r, char g, char b, char intensity){
    if (i<0 || i>=PixelNum) return ;
    pixel_array[i].intensity = intensity  ;  //enforce max 
    pixel_array[i].red = r   ;
    pixel_array[i].green = g  ;
    pixel_array[i].blue = b ;
}

// === write a HSVI value to the pixel array =======================
void set_pixel_hsv(int i, float h, float s, float v, char intensity){
    float C, X, m, rp, gp, bp ;
    unsigned char r, g, b ;
    // index range check
    if (i<0 || i>=PixelNum) return ;
    // hsv to rgb conversion from
    // http://www.rapidtables.com/convert/color/hsv-to-rgb.htm
    C = v * s;
    //X = C * (1 - abs((int)(h/60)%2 - 1));
    // (h/60) mod 2  = (h/60 - (int)(h/60))
    X = C * (1.0 - fabsf(fmodf(h/60.0, 2.0) - 1.));
    m = v - C;
    if      ((0<=h) && (h<60))   { rp = C; gp = X; bp = 0;}
    else if ((60<=h) && (h<120)) { rp = X; gp = C; bp = 0;}
    else if ((120<=h) && (h<180)){ rp = 0; gp = C; bp = X;}
    else if ((180<=h) && (h<240)){ rp = 0; gp = X; bp = C;}
    else if ((240<=h) && (h<300)){ rp = X; gp = 0; bp = C;}
    else if ((300<=h) && (h<360)){ rp = C; gp = 0; bp = X;}
    else                         { rp = 0; gp = 0; bp = 0;}
    
    r = (unsigned char)((rp+m)*255) ;
    g = (unsigned char)((gp+m)*255) ;
    b = (unsigned char)((bp+m)*255) ;
            
    pixel_array[i].intensity = intensity  ;  //enforce max 
    pixel_array[i].red = r   ;
    pixel_array[i].green = g  ;
    pixel_array[i].blue = b  ;
}

// === thread structures ============================================
// thread control structs
// note that UART input and output are threads
static struct pt pt_timer ;


// === Timer Thread =================================================
// update a 1 second tick counter
int position=0, dir=1;
#define DDS_sample_time 30

static PT_THREAD (protothread_timer(struct pt *pt)){
    PT_BEGIN(pt);
      static int i ;
      static char r,g,b,intensity;
      int first = 1;
      int j = 0;
      
      for(i=0; i<PixelNum; i++){
        set_pixel_rgb(i,0,0,0,intensity);
      }
      write_pixels();

      while(1) {
        // yield time 
        //PT_YIELD_TIME_msec(DDS_sample_time) ;

        if( (PORTReadBits(IOPORT_B,BIT_1)==0) && !first){
            pwm_on_time = 0;
            SetDCOC3PWM(pwm_on_time);
            while(1) {
             first = 1;   
            }
        }
        for(i=0; i<PixelNum; i++){

            //v intensity = QUAR_ON ;
            r = 0;
            g = 0;
            b = 0;
            read_pixeldata(&r, &g, &b);
            intensity = 1;
            set_pixel_rgb(i,r,g,b,intensity);
            
        }
        
      //  read_pixeldata();
        if(PORTReadBits(IOPORT_B,BIT_1)>0) {
            if(j>3) {
                first = 0;
                pwm_on_time = 15000;
                SetDCOC3PWM(pwm_on_time);
            }
            write_pixels();
            j++;
        }
        else{
            pwm_on_time = 0;
            SetDCOC3PWM(pwm_on_time);
            while(1);
        }
        
        
       
      } // END WHILE(1)
  PT_END(pt);
} // timer thread

// === Main  ======================================================
void main(void) {
    ANSELA = 0; ANSELB = 0;
    
    // === Config timer and output compares to make pulses ========
  // set up timer2 to generate the wave period
  OpenTimer2(T2_ON | T2_SOURCE_INT | T2_PS_1_1, generate_period);
  ConfigIntTimer2(T2_INT_ON | T2_INT_PRIOR_2);
  mT2ClearIntFlag(); // and clear the interrupt flag

  // set up compare3 for PWM mode
  OpenOC3(OC_ON | OC_TIMER2_SRC | OC_PWM_FAULT_PIN_DISABLE , pwm_on_time, pwm_on_time); //
  // OC3 is PPS group 4, map to RPB9 (pin 18)
  PPSOutput(4, RPB9, OC3);
    
  // === config threads ==========
  // turns OFF UART support and debugger pin, unless defines are set
  PT_setup();

  // === setup system wide interrupts  ========
  INTEnableSystemMultiVectoredInt();

    // divide Fpb by 2, configure the I/O ports. Not using SS in this example
    // 16 bit transfer CKP=1 CKE=1
    // possibles SPI_OPEN_CKP_HIGH;   SPI_OPEN_SMP_END;  SPI_OPEN_CKE_REV
    // For any given peripherial, you will need to match these
    SpiChnOpen(2, SPI_OPEN_ON | SPI_OPEN_MODE32 | SPI_OPEN_MSTEN | SPICON_CKP, 4);
    // SCK2 is pin 26 
    // SDO2 (MOSI) is in PPS output group 2, could be connected to RB5 which is pin 14
    PPSOutput(2, RPB8, SDO2);
    
    //Parallel communication
     //0,3,4,5,7,13,10,9,11,13
    mPORTBSetPinsDigitalIn(BIT_0 | BIT_1| BIT_2| BIT_3 | BIT_7 | BIT_13| BIT_4 | BIT_5 |BIT_10  | BIT_9| BIT_11);// | BIT_5 |BIT_7| BIT_10| BIT_11| BIT_13| BIT_14| BIT_0| BIT_2);
    EnablePullDownB(BIT_0 | BIT_1| BIT_2| BIT_3 | BIT_7 | BIT_13| BIT_4 | BIT_5 |BIT_10 | BIT_9 | BIT_11);// | BIT_5 |BIT_7| BIT_10| BIT_11| BIT_13| BIT_14| BIT_0| BIT_2);
    mPORTASetPinsDigitalIn(BIT_0 | BIT_1 | BIT_2 | BIT_3);
    EnablePullDownA(BIT_0 | BIT_1 | BIT_2 | BIT_3);
    
  // init the threads
  PT_INIT(&pt_timer);
  

  
  // round-robin scheduler for threads
  while (1){
      PT_SCHEDULE(protothread_timer(&pt_timer));
  }
} // main

// === end  ======================================================

