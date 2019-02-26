#include "../PLIB.h"

// Default Random Number Generator seed. 0x41FE9F9E corresponds to calling LFSRSeedRand(1)
static DWORD dwLFSRRandSeed = 0x41FE9F9E;

APP_CONFIG AppConfig;

DWORD LFSRSeedRand(DWORD dwSeed) 
{
    DWORD dwOldSeed;
    BYTE i;

    // Save original seed to be returned later
    dwOldSeed = dwLFSRRandSeed;

    if (dwSeed == 0u)
    {
        dwSeed = 1;
    }

    // Set the new seed
    dwLFSRRandSeed = dwSeed;

    // Run the LFSR a few times to get rid of obvious start up artifacts for
    // seed values that don't have many set bits.
    for (i = 0; i < 16; i++)
    {
        LFSRRand();
    }

    return dwOldSeed;
}

WORD LFSRRand(void) 
{
    BYTE i;

    for (i = 0; i < 15; i++)
    {
        dwLFSRRandSeed = (dwLFSRRandSeed >> 1) ^ ((0ul - (dwLFSRRandSeed & 1ul)) & 0xD0000001ul);
    }

    return (WORD) dwLFSRRandSeed;
}

DWORD GenerateRandomDWORD(void) 
{
    return ((DWORD) LFSRRand()) | (((DWORD) LFSRRand()) << 16);
}

/*****************************************************************************
  Function:
        BOOL StringToIPAddress(BYTE* str, IP_ADDR* IPAddress)

  Summary:
        Converts a string to an IP address

  Description:
        This function parses a dotted-quad decimal IP address string into an
        IP_ADDR struct.  The output result is big-endian.

  Parameters:
        str - Pointer to a dotted-quad IP address string
        IPAddress - Pointer to IP_ADDR in which to store the result

  Return Values:
        TRUE - an IP address was successfully decoded
        FALSE - no IP address could be found, or the format was incorrect
 ***************************************************************************/
BOOL StringToIPAddress(BYTE* str, IP_ADDR* IPAddress) 
{
    DWORD_VAL dwVal;
    BYTE c, currentByte, numberOfDigit;
    
    dwVal.Val = 0;
    currentByte = 0;
    numberOfDigit = 0;
    
    while((c = *str++))
    {
        if((c == '.') || (c == '-'))
        {
            if(!numberOfDigit || (currentByte >= 3))
            {
                return FALSE;
            }
            numberOfDigit = 0;
            dwVal.Val = 0;
            currentByte++;
        }
        else if(((c - '0') >= 0) && ((c - '0') <= 9))
        {
            c -= '0';
            numberOfDigit++;
            dwVal.Val <<= 8;
            dwVal.v[0] = c;
            if((dwVal.Val > 0x00020505ul) || (numberOfDigit > 3))
            {
                return FALSE;
            }
            IPAddress->v[currentByte] = dwVal.v[2]*((BYTE) 100) + dwVal.v[1]*((BYTE) 10) + dwVal.v[0];
        }
        else
        {
            return FALSE;
        }
    }
    
    if(!currentByte)
    {
        return FALSE;
    }
    
    return TRUE;
}

/*****************************************************************************
  Function:
        BOOL StringToMACAddress(BYTE* str, MAC_ADDR* MACAddress) 

  Summary:
        Converts a string to a MAC address

  Description:
        This function parses a dotted-sixed hexadecimal MAC address string into a
        MAC_ADDR struct.  The output result is big-endian.

  Parameters:
        str - Pointer to a dotted-sixed hexadecimal MAC address string
        MACAddress - Pointer to MAC_ADDR in which to store the result

  Return Values:
        TRUE - a MAC address was successfully decoded
        FALSE - no MAC address could be found, or the format was incorrect
 ***************************************************************************/
BOOL StringToMACAddress(BYTE* str, BYTE* MACAddress) 
{
    WORD_VAL wVal;
    BYTE c, currentByte, numberOfDigit;
    
    wVal.Val = 0;
    currentByte = 0;
    numberOfDigit = 0;
    
    while((c = *str++))
    {
        if(c == ':')
        {
            if(!numberOfDigit || (currentByte >= 5))
            {
                return FALSE;
            }
            numberOfDigit = 0;
            wVal.Val = 0;
            currentByte++;
        }
        else if((((c - '0') >= 0) && ((c - '0') <= 9)) || (((c - 'A') >= 0) && ((c - 'A') <= 5)) || (((c - 'a') >= 0) && ((c - 'a') <= 5)))
        {
            if(((c - '0') >= 0) && ((c - '0') <= 9))
            {
                c -= '0';
            }
            else if(((c - 'A') >= 0) && ((c - 'A') <= 5))
            {
                c -= ('A' - 10);
            }
            else
            {
                c -= ('a' - 10);
            }
            numberOfDigit++;
            wVal.Val <<= 8;
            wVal.v[0] = c;
            if(numberOfDigit > 2)
            {
                return FALSE;
            }
            MACAddress[currentByte] = wVal.v[1]*((BYTE) 16) + wVal.v[0];
        }
        else
        {
            return FALSE;
        }
    }
    
    if(!currentByte)
    {
        return FALSE;
    }
    
    return TRUE;
}

/*****************************************************************************
  Function:
        WORD CalcIPChecksum(BYTE* buffer, WORD count)

  Summary:
        Calculates an IP checksum value.

  Description:
        This function calculates an IP checksum over an array of input data.  The
        checksum is the 16-bit one's complement of one's complement sum of all
        words in the data (with zero-padding if an odd number of bytes are
        summed).  This checksum is defined in RFC 793.

  Precondition:
        buffer is WORD aligned (even memory address) on 16- and 32-bit PICs.

  Parameters:
        buffer - pointer to the data to be checksummed
        count  - number of bytes to be checksummed

  Returns:
        The calculated checksum.

  Internal:
        This function could be improved to do 32-bit sums on PIC32 platforms.
 ***************************************************************************/
WORD CalcIPChecksum(BYTE* buffer, WORD count) 
{
    WORD i = count >> 1; // divide by 2;
    WORD *val = (WORD*) buffer;
    DWORD_VAL sum;
    
    sum.Val = 0;
    while (i--)
    {
        sum.Val += (DWORD) *val++;
    }
    // Add in the sum of the remaining byte, if present
    if(count & 0x1)
    {
        sum.Val += (DWORD)*(BYTE*) val;
    }
    
    sum.Val = (DWORD) sum.w[0] + (DWORD) sum.w[1];  // Do an end-around carry (one's complement arrithmatic)
    sum.w[0] += sum.w[1];       // Do another end-around carry in case if the prior add caused a carry out

    return ~sum.w[0];
}
