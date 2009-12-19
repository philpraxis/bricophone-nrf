//
#include <Nordic\reg9e5.h>

#define HFREQ 1                     // 0=433MHz, 1=868/915MHz
#define POWER 3                     // 0=min power...3 = max power

#if 1
void Delay400us(volatile unsigned char n)
{
    unsigned char i;
    while(n--)
        for(i=0;i<35;i++)
            ;
}
#endif

unsigned char SpiReadWrite(unsigned char b)
{
    EXIF &= ~0x20;                  // Clear SPI interrupt
    SPI_DATA = b;                   // Move byte to send to SPI data register
    while((EXIF & 0x20) == 0x00)    // Wait until SPI hs finished transmitting
        ;
    return SPI_DATA;
}

#if 1
void TransmitPacket(unsigned char b)
{
    RACSN = 0;
    SpiReadWrite(WTP);
    SpiReadWrite(b);
    RACSN = 1;
    TRX_CE = 1;
    Delay400us(1);
    TRX_CE = 0;
}
#endif

void PutChar(char c)
{
    while(!TI)
        ;
    TI = 0;
    SBUF = c;
}

void Transmitter(void)
{
    TXEN = 1;
    for(;;)
    {
		char c;

		while (!RI)
			;			
		RI = 0;
		c = SBUF;
		TransmitPacket(c);
		PutChar(c);
    }
}

void Init(void)
{
    unsigned char tmp;
    unsigned char cklf;

	//serial port

    TH1 = 243;                      // 19200@16MHz (when T1M=1 and SMOD=1)
    CKCON |= 0x10;                  // T1M=1 (/4 timer clock)
    PCON = 0x80;                    // SMOD=1 (double baud rate)
    //SCON = 0x52;                    // Serial mode1, enable receiver
	SCON = 0x72;                    // Serial mode1, rx+tx
    TMOD = 0x20;                    // Timer1 8bit auto reload 
    TR1 = 1;                        // Start timer1
    P0_ALT |= 0x06;                 // Select alternate functions on pins P0.1 and P0.2
    P0_DIR |= 0x02;                 // P0.1 (RxD) is input

    SPICLK = 0;                     // Max SPI clock
    SPI_CTRL = 0x02;                // Connect internal SPI controller to Radio

    // Switch to 16MHz clock:
    RACSN = 0;
    SpiReadWrite(RRC | 0x09);
    cklf = SpiReadWrite(0) | 0x04;
    RACSN = 1;
    RACSN = 0;
    SpiReadWrite(WRC | 0x09);
    SpiReadWrite(cklf);
    RACSN = 1;

#if 1
	//RF
    //P0_ALT = 0x00;
    //P0_DIR = 0xFF;

    //SPICLK = 0;                     // Max SPI clock
    //SPI_CTRL = 0x02;                // Connect internal SPI controller to Radio

    // Configure Radio:
    RACSN = 0;
    SpiReadWrite(WRC | 0x03);       // Write to RF config address 3 (RX payload)
    SpiReadWrite(0x01);             // One byte RX payload
    SpiReadWrite(0x01);             // One byte TX payload
    RACSN = 1;

    RACSN = 0;
    SpiReadWrite(RRC | 0x01);       // Read RF config address 1
    tmp = SpiReadWrite(0) & 0xf1;   // Clear the power and frequency setting bits
    RACSN = 1;

    RACSN = 0;
    SpiReadWrite(WRC | 0x01);      // Write RF config address 1
    // Change power defined by POWER and to 433 or 868/915MHz defined by HFREQ above:
    SpiReadWrite(tmp | (POWER <<2) | (HFREQ << 1));
    RACSN = 1;
#endif
}

void main(void)
{
    Init();
    Transmitter();
}
