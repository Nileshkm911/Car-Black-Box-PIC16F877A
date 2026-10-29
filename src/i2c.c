#include "common.h"
#include "main.h"

void init_i2c(unsigned long baud)
{
    /* Configure SCL (RC3) and SDA (RC4) as inputs for MSSP peripheral */
    TRISC3 = 1;
    TRISC4 = 1;

    /* Set I2C Master Mode (SSPM<3:0> = 1000) and enable MSSP (SSPEN = 1) */
    SSPCON = 0x28;
    SSPCON2 = 0x00;
    
    /* Set Baud Rate Generator: Fosc / (4 * (SSPADD + 1)) = baud */
    SSPADD = (unsigned char)(FOSC / (4 * baud)) - 1;
    
    /* Slew rate control disabled for standard speed (100 kHz) */
    SSPSTAT = 0x80;
}

static void i2c_wait_for_idle(void)
{
    /* Wait while Start, Repeated Start, Stop, Receive, or Acknowledge sequence is in progress, or Transmit is active */
    while ((SSPCON2 & 0x1F) || (SSPSTAT & 0x04));
}

void i2c_start(void)
{
    i2c_wait_for_idle();
    SSPIF = 0;
    SEN = 1;            // Initiate Start condition
    while (!SSPIF);     // Hardware sets SSPIF when Start condition completes
    SSPIF = 0;
}

void i2c_rep_start(void)
{
    i2c_wait_for_idle();
    SSPIF = 0;
    RSEN = 1;            // Initiate Repeated Start condition (keeps address pointer in DS1307)
    while (!SSPIF);      // Hardware sets SSPIF when Repeated Start completes
    SSPIF = 0;
}

void i2c_stop(void)
{
    i2c_wait_for_idle();
    SSPIF = 0;
    PEN = 1;            // Initiate Stop condition
    while (!SSPIF);     // Hardware sets SSPIF when Stop condition completes
    SSPIF = 0;
}

unsigned char i2c_read(unsigned char ack)
{
    unsigned char data;
    
    i2c_wait_for_idle();
    SSPIF = 0;
    RCEN = 1;           // Enable receive mode
    while (!SSPIF);     // Hardware sets SSPIF when 8 bits received
    SSPIF = 0;
    data = SSPBUF;      // Read received byte from buffer
    
    i2c_wait_for_idle();
    SSPIF = 0;
    ACKDT = (ack == 1) ? 0 : 1; // 0 = Send ACK, 1 = Send NACK
    ACKEN = 1;          // Initiate Acknowledge sequence
    while (!SSPIF);     // Hardware sets SSPIF when Acknowledge sequence completes
    SSPIF = 0;
    
    return data;
}

int i2c_write(unsigned char data)
{
    i2c_wait_for_idle();
    SSPIF = 0;
    SSPBUF = data;      // Load data into buffer to start transmission
    while (!SSPIF);     // Hardware sets SSPIF when 9th clock cycle + ACK are complete
    SSPIF = 0;
    
    return !ACKSTAT;    // Return 1 if ACK received from slave, 0 if NACK
}
