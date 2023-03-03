/*
 * platform_unix.c
 *
 * Created: 11/27/2011 5:43:25 PM
 *  Author: tjaekel
 */ 

#ifdef WITH_SCRIPTS
#ifdef WIN32
#include <stdio.h>
#endif
#endif

#ifdef WIN32
#include <conio.h>
#endif

#include "picoc.h"
#ifndef WIN32
#include "UART.h"
#endif

/* read a file into memory */
/* a source file we need to clean up */
static unsigned char *CleanupText = NULL;

/* deallocate any storage */
void PlatformCleanup(void)
{
    /* just in case we have used malloc on PlatformReadFile() */
    if (CleanupText != NULL)
        free(CleanupText);
}

/* get a line of interactive input */
char *PlatformGetLine(char *Buf, int MaxLen)
{
    /*
     * ATT: we wait here until something available
     * If we return here with NULL or empty string - we cannot proceed
     * on additional function body definition.
     * Everything had to be on one line, but if blocking read here,
     * we can run endless. Therefore put this into a task or wait for
     * characters received.
     */

#ifndef WIN32
	UART_getString(Buf, MaxLen);
#else
    fgets(Buf, MaxLen, stdin);
    return Buf;
#endif
}

/* get a character of interactive input */
int PlatformGetCharacter(void)
{
    /*
     * NOT USED - instead we expect input via PlatformGetLine
     */
#ifndef WIN32
    return (int)UART_getChar();
#else
	return _fgetchar();
#endif
}

/* write a character to the console */
void PlatformPutc(unsigned char OutCh, union OutputStreamInfo *Stream)
{
    (void)Stream;

#ifndef WIN32
     UART_putChar(OutCh);
#else
    //_fputchar(OutCh);
    put_character(OutCh);
#endif
}

#ifdef WITH_SCRIPTS
#ifndef WIN32
static unsigned char scriptBuf[MAX_SCRIPT_SIZE];
#endif
#endif

unsigned char *PlatformReadFile(const char *FileName)
{
#ifdef WITH_SCRIPTS
	unsigned char *SourceStr;
#ifndef WIN32
	/* read a file via TeraTerm */
	int c;
	////SourceStr = malloc(MAX_SCRIPT_SIZE);
	SourceStr = scriptBuf;
	if (SourceStr)
	{
		do
		{
			/* use function without echo */
			do
				c = UART_getChar_nW();
			while (c == 0);

			if (c != 0x1A)
				*SourceStr++ = (unsigned char)c;
		} while (c != 0x1A);	/* until CTRL-Z */
		*SourceStr = '\0';
	}

	return scriptBuf;
#else
	FILE *fp;
	fpos_t fileEnd;

	fp = fopen(FileName, "r");
	if ( ! fp)
	{
        ////Tprintf("*E: file '%s' does not exist\n", FileName);
        PlatformPrintf("*E: file '%s' does not exist\n", FileName);
		return 0;
	}

	fseek(fp, 0, SEEK_END);
	fgetpos(fp, &fileEnd);
	fseek(fp, 0, SEEK_SET);

	SourceStr = (unsigned char *)malloc((long)fileEnd + 1);

	if (SourceStr)
	{
		/*
		 * Attention: Windows substitutes 0x0D 0x0A into single 0x0A
		 * the file size is larger as really read bytes, get the result
		 * from fread in order to know how many bytes are really there!
		 */
		fileEnd = fread(SourceStr, sizeof(char), (size_t)fileEnd, fp);
		fclose(fp);
		SourceStr[fileEnd] = '\0';

		return SourceStr;
	}
	return 0;
#endif
#else
	return 0;
#endif
}

/* read and scan a file for definitions */
void PlatformScanFile(const char *FileName)
{
    unsigned char *SourceStr;
    unsigned char *OrigCleanupText;
	long strLen;

    SourceStr = PlatformReadFile(FileName);
    if (SourceStr)
    {
    	OrigCleanupText = CleanupText;
    	if (CleanupText == NULL)
        {
    		CleanupText = SourceStr;
        }

		strLen = strlen((char *)SourceStr);
    	Parse(FileName, (char *)SourceStr, strLen, TRUE);
#ifdef WIN32
    	free(SourceStr);
#endif

    	if (OrigCleanupText == NULL)
    		CleanupText = NULL;
    }
}

/* mark where to end the program for platforms which require this */
jmp_buf ExitBuf;

/* exit the program */
void PlatformExit(void)
{
	//endless loop - reinit and start over
    longjmp(ExitBuf, 1);
}

/* added for Qt integration */
/* collect single characters and process it when NEWLINE seen */
#define OUTPUT_BUFFER_SIZE  1024

#ifdef WIN32
static unsigned char sOutputPrintBuffer[OUTPUT_BUFFER_SIZE];
static int sOutputBufferIndex = 0;
#endif

void put_character(unsigned char c)
{
#ifdef WIN32
    _fputchar(c);
#else
    sOutputPrintBuffer[sOutputBufferIndex++] = c;
    sOutputPrintBuffer[sOutputBufferIndex++] = '\0';

    if ((c == '\n') || (sOutputBufferIndex = (OUTPUT_BUFFER_SIZE - 1)))
    {
        XprintStringStr((char *)sOutputPrintBuffer);
        sOutputBufferIndex = 0;
    }
#endif
}

char GdisplayMessageBuffer[OUTPUT_BUFFER_SIZE];

void put_string(char *s)
{
#ifdef WIN32
    puts(s);
#else
    XprintStringStr(s);
#endif
}

int picoc_CheckForAbort()
{
#ifdef WIN32
    if (_kbhit())           //any character will break script, endless loop
    {
        GpicocAbort = 1;
        longjmp(ExitBuf, 1);
        return 1;           //not exeucted
    }
#endif
    return 0;
}
