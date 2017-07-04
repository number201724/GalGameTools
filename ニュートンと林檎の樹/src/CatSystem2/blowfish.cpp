// blowfish.cpp   C++ class implementation of the BLOWFISH encryption algorithm
// _THE BLOWFISH ENCRYPTION ALGORITHM_
// by Bruce Schneier
// Revised code--3/20/94
// Converted to C++ class 5/96, Jim Conger

#include "blowfish.h"
#include "blowfish.h2"	// holds the random digit tables
#include "crt_h.h"
#include "my_mem.h"

#pragma warning(disable:4127)

#define S(x,i) (SBoxes[i][x.w.byte##i])
#define bf_F(x) (((S(x,0) + S(x,1)) ^ S(x,2)) + S(x,3))
#define ROUND(a,b,n) (a.dword ^= bf_F(b) ^ PArray[n])


CBlowFish::CBlowFish ()
{
    PArray = (LPDWORD)m_mem.Alloc(18 * sizeof(DWORD));
    SBoxes = (DWORD (*)[256])m_mem.Alloc(4 * 256 * sizeof(DWORD));
}

CBlowFish::~CBlowFish ()
{
    m_mem.Free(PArray);
    m_mem.Free(SBoxes);
}

	// the low level (private) encryption function
void CBlowFish::Blowfish_encipher (DWORD *xl, DWORD *xr)
{
	union aword  Xl, Xr ;

	Xl.dword = *xl ;
	Xr.dword = *xr ;

	Xl.dword ^= PArray [0];
	ROUND (Xr, Xl, 1)  ; ROUND (Xl, Xr, 2) ;
	ROUND (Xr, Xl, 3)  ; ROUND (Xl, Xr, 4) ;
	ROUND (Xr, Xl, 5)  ; ROUND (Xl, Xr, 6) ;
	ROUND (Xr, Xl, 7)  ; ROUND (Xl, Xr, 8) ;
	ROUND (Xr, Xl, 9)  ; ROUND (Xl, Xr, 10) ;
	ROUND (Xr, Xl, 11) ; ROUND (Xl, Xr, 12) ;
	ROUND (Xr, Xl, 13) ; ROUND (Xl, Xr, 14) ;
	ROUND (Xr, Xl, 15) ; ROUND (Xl, Xr, 16) ;
	Xr.dword ^= PArray [17] ;

	*xr = Xl.dword ;
	*xl = Xr.dword ;
}

	// the low level (private) decryption function
void CBlowFish::Blowfish_decipher (DWORD *xl, DWORD *xr)
{
   union aword  Xl ;
   union aword  Xr ;

   Xl.dword = *xl ;
   Xr.dword = *xr ;

   Xl.dword ^= PArray [17] ;
   ROUND (Xr, Xl, 16);  ROUND (Xl, Xr, 15);
   ROUND (Xr, Xl, 14);  ROUND (Xl, Xr, 13);
   ROUND (Xr, Xl, 12);  ROUND (Xl, Xr, 11);
   ROUND (Xr, Xl, 10);  ROUND (Xl, Xr, 9) ;
   ROUND (Xr, Xl, 8) ;  ROUND (Xl, Xr, 7) ;
   ROUND (Xr, Xl, 6) ;  ROUND (Xl, Xr, 5) ;
   ROUND (Xr, Xl, 4) ;  ROUND (Xl, Xr, 3) ;
   ROUND (Xr, Xl, 2) ;  ROUND (Xl, Xr, 1) ;
   Xr.dword ^= PArray[0];

   *xl = Xr.dword;
   *xr = Xl.dword;
}


	// constructs the enctryption sieve
void CBlowFish::Initialize (BYTE key[], int keybytes)
{
	int  		i, j ;
	DWORD  		data, datal, datar ;
	union aword temp ;

	// first fill arrays from data tables
    memcpy(PArray, bf_P, sizeof(bf_P));
    memcpy(SBoxes, bf_S, sizeof(bf_S));

	j = 0 ;
	for (i = 0 ; i < NPASS + 2 ; ++i)
	{
		temp.dword = 0 ;
		temp.w.byte0 = key[j];
		temp.w.byte1 = key[(j+1) % keybytes] ;
		temp.w.byte2 = key[(j+2) % keybytes] ;
		temp.w.byte3 = key[(j+3) % keybytes] ;
		data = temp.dword ;
		PArray [i] ^= data ;
		j = (j + 4) % keybytes ;
	}

	datal = 0 ;
	datar = 0 ;

	for (i = 0 ; i < NPASS + 2 ; i += 2)
	{
		Blowfish_encipher (&datal, &datar) ;
		PArray [i] = datal ;
		PArray [i + 1] = datar ;
	}

	for (i = 0 ; i < 4 ; ++i)
	{
		for (j = 0 ; j < 256 ; j += 2)
		{
		  Blowfish_encipher (&datal, &datar) ;
		  SBoxes [i][j] = datal ;
		  SBoxes [i][j + 1] = datar ;
		}
	}
}

	// get output length, which must be even MOD 8
DWORD CBlowFish::GetOutputLength (DWORD lInputLong)
{
	DWORD 	lVal ;

	lVal = lInputLong % 8 ;	// find out if uneven number of bytes at the end
	if (lVal != 0)
		return lInputLong + 8 - lVal ;
	else
		return lInputLong ;
}

	// Encode pIntput into pOutput.  Input length in lSize.  Returned value
	// is length of output which will be even MOD 8 bytes.  Input buffer and
	// output buffer can be the same, but be sure buffer length is even MOD 8.
DWORD CBlowFish::Encode (BYTE * pInput, BYTE * pOutput, DWORD lSize)
{
	DWORD 	lCount, lOutSize, lGoodBytes ;
	BYTE	*pi, *po ;
	int		i, j ;
	int		SameDest = (pInput == pOutput ? 1 : 0) ;

    (SameDest);
    lSize &= 0xFFFFFFF8;
    pi = pInput;
	lOutSize = GetOutputLength (lSize) ;
	for (lCount = 0 ; lCount < lOutSize ; lCount += 8)
	{
		if (1 || SameDest)	// if encoded data is being written into input buffer
		{
		 	if (lCount < lSize - 7)	// if not dealing with uneven bytes at end
		 	{
		 	 	Blowfish_encipher ((DWORD *) pInput,
		 	 		(DWORD *) (pInput + 4)) ;
		 	}
		 	else		// pad end of data with null bytes to complete encryption
		 	{
//				po = pInput + lSize ;	// point at byte past the end of actual data
                po = pi + lCount;
				j = (int) (lOutSize - lSize) ;	// number of bytes to set to null
				for (i = 0 ; i < j ; i++)
					*po++ = 0 ;
		 	 	Blowfish_encipher ((DWORD *) pInput,
		 	 		(DWORD *) (pInput + 4)) ;
		 	}
		 	pInput += 8 ;
		}
		else if (0)		// output buffer not equal to input buffer, so must copy
		{               // input to output buffer prior to encrypting
		 	if (lCount < lSize - 7)	// if not dealing with uneven bytes at end
		 	{
		 		pi = pInput ;
		 		po = pOutput ;
		 		for (i = 0 ; i < 8 ; i++)
// copy bytes to output
		 			*po++ = *pi++ ;
		 	 	Blowfish_encipher ((DWORD *) pOutput,	// now encrypt them
		 	 		(DWORD *) (pOutput + 4)) ;
		 	}
		 	else		// pad end of data with null bytes to complete encryption
		 	{
		 		lGoodBytes = lSize - lCount ;	// number of remaining data bytes
		 		po = pOutput ;
		 		for (i = 0 ; i < (int) lGoodBytes ; i++)
		 			*po++ = *pInput++ ;
		 		for (j = i ; j < 8 ; j++)
		 			*po++ = 0 ;
		 	 	Blowfish_encipher ((DWORD *) pOutput,
		 	 		(DWORD *) (pOutput + 4)) ;
		 	}
		 	pInput += 8 ;
		 	pOutput += 8 ;
		}
	}
	return lOutSize ;
 }

	// Decode pIntput into pOutput.  Input length in lSize.  Input buffer and
	// output buffer can be the same, but be sure buffer length is even MOD 8.
void CBlowFish::Decode (BYTE * pInput, BYTE * pOutput, DWORD lSize)
{
	DWORD 	lCount ;
	BYTE	*pi, *po ;
	int		i ;
	int		SameDest = (pInput == pOutput ? 1 : 0) ;

    (SameDest);
    lSize &= 0xFFFFFFF8;
	for (lCount = 0 ; lCount < lSize ; lCount += 8)
	{
		if (1 || SameDest)	// if encoded data is being written into input buffer
		{
	 	 	Blowfish_decipher ((DWORD *) pInput, (DWORD *) (pInput + 4)) ;
		 	pInput += 8 ;
		}
		else if (0)		// output buffer not equal to input buffer
		{               // so copy input to output before decoding
	 		pi = pInput ;
	 		po = pOutput ;
	 		for (i = 0 ; i < 8 ; i++)
	 			*po++ = *pi++ ;
	 	 	Blowfish_decipher ((DWORD *) pOutput, (DWORD *) (pOutput + 4)) ;
		 	pInput += 8 ;
		 	pOutput += 8 ;
		}
	}
}

#pragma warning(default:4127)