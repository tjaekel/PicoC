/*
 * Create a Thread on Qt - we need it this way: Pico-C will wait on command line,
 * we have to similate this via a blocking thread, waiting for input
 */

#include "picoc_qtthread.h"
#include "picoc.h"

////#include "TCL_interface.h"
////#include "interface.h"
////#include "command.h"

#include "msSleep.h"

//a macro to sleep a bit
//already defined in interface.h
//#define msSleep(ms) { QMutex _x_mutex(QMutex::NonRecursive); _x_mutex.lock(); QWaitCondition _x_sleep; _x_sleep.wait(&_x_mutex, ms); _x_mutex.unlock(); }

char pico_c_cmd_buffer[LINEBUFFER_MAX];

PicoCThread::PicoCThread(void)
{
    stopped = true;
    //set the blocking semaphore
}

PicoCThread::~PicoCThread(void)
{
    Stop();
}

void PicoCThread::Start(void)
{
    //start
    if (stopped)
    {
        //not yet running
        stopped = false;
        this->start();
    }
}

void PicoCThread::Stop(void)
{
    if (stopped != true)
    {
        stopped = true;
        GpicocAbort = 1;
        //put something into buffer so that thread is unblocked
        pico_c_cmd_buffer[1] = '\0';
        inputLineAvailable.release();

        //wait a bit so that thread can stop
        msSleep(10);
    }
}

int PicoCThread::CheckStopped(void)
{
    return stopped;
}

void PicoCThread::run(void)
{
    while ( ! stopped)
    {
        try {
            //endless loop in thread
            //ATT: we should never come back here, we run endless inside, except we stop from outside
            pico_c_main_interactive(0, NULL);
            //we read commands in blocking line mode inside Pico-C Platform function
        } catch (std::exception &e) {
            qFatal("Error %s", e.what());
        } catch (...) {
            qFatal("Error");
        }
    }
}

void PicoCThread::SendCommandLine(char *s)
{
    //we handle here '\0x0A' and concatinate the lines
    //make sure not to overflow the final buffer
    size_t sLen;
    size_t sLen2;

    sLen = strlen(s);
    if (sLen)
    {
        if (*(s + sLen - 1) == '\\')
        {
            sLen--;
            sLen2 = strlen(pico_c_cmd_buffer);
            if ((sLen + sLen2) > (sizeof(pico_c_cmd_buffer)))
            {
                sLen = sizeof(pico_c_cmd_buffer) - sLen2;
                strncat(pico_c_cmd_buffer, s, sLen);
                inputLineAvailable.release();
            }
            else
            {
                //XXXX ?
                sLen2 = strlen(pico_c_cmd_buffer);
                if ((sLen + sLen2) > (sizeof(pico_c_cmd_buffer)))
                {
                    sLen = sizeof(pico_c_cmd_buffer) - sLen2;
                    strncat(pico_c_cmd_buffer, s, sLen);
                    inputLineAvailable.release();
                }
            }
        }
        else
        {
            sLen2 = strlen(pico_c_cmd_buffer);
            if ((sLen + sLen2) > sizeof(pico_c_cmd_buffer))
            {
                sLen = sizeof(pico_c_cmd_buffer) - sLen2;
            }
            strncat(pico_c_cmd_buffer, s, sLen);
            inputLineAvailable.release();           //release the semaphore
        }
    }
}

char * PicoCThread::GetCommandLine(char *s, int MaxLen)
{
    inputLineAvailable.acquire();           //we block here if nothing available

    if (pico_c_cmd_buffer[0] == '\0')
        return NULL;                        //EOF, empty
    else
        strncpy(s, pico_c_cmd_buffer, MaxLen);
    pico_c_cmd_buffer[0] = '\0';

    return s;
}

PicoCThread *pPicoCThread = NULL;

extern int XExecuteCommand(char *Cmds, TCL_if *pTCL_if);

#ifdef __cplusplus
extern "C" {
#endif

char * picoc_GetCommandLine(char *s, int MaxLen)
{
    if (pPicoCThread)
        return pPicoCThread->GetCommandLine(s, MaxLen);
    else
        return NULL;
}

int picoc_CheckForStopped(void)
{
    return pPicoCThread->CheckStopped();
}

int picoc_CheckForAbort(void)
{
    //we check here if we should abort, kill (stopped set)
    //and let the main Application update on events

    QCoreApplication::processEvents();

    if (pPicoCThread)
    {
        if (pPicoCThread->CheckStopped())
            longjmp(ExitBuf, 1);
        if (GpicocAbort)
        {
            GpicocAbort = 0;
            longjmp(ExitBuf, 2);
        }
    }

    return 0;
}

void picoc_MsSleep(unsigned long ms)
{
    msSleep(ms);
}

int picoc_ExecuteCommand(char *s)
{
    return XExecuteCommand(s, TCLOBJ);
}

int picoc_WriteConsecutive(unsigned long addr, void *val, unsigned long len, unsigned long chip)
{
    int iif = CheckInterface(chip);
    if (iif != -1)
        if (pChipIF[iif])
        {
            return pChipIF[iif]->WriteConsecutive(val, addr, len);
        }

    return 0;
}

int picoc_ReadConsecutive(unsigned long addr, void *val, unsigned long len, unsigned long chip)
{
    int iif = CheckInterface(chip);
    if (iif != -1)
        if (pChipIF[iif])
        {
            return pChipIF[iif]->ReadConsecutive(val, addr, len);
        }

    return 0;
}

int picoc_WBlk(unsigned long addr, void *val, unsigned long len, unsigned long chip)
{
    int iif = CheckInterface(chip);
    if (iif != -1)
        if (pChipIF[iif])
        {
            return pChipIF[iif]->EPWrite(val, addr, len);
        }

    return 0;
}

int picoc_RBlk(unsigned long addr, void *val, unsigned long len, unsigned long chip)
{
    int iif = CheckInterface(chip);
    if (iif != -1)
        if (pChipIF[iif])
        {
            return pChipIF[iif]->EPRead(val, addr, len);
        }

    return 0;
}

int picoc_WFifo(unsigned long addr, void *val, unsigned long len, unsigned long chip)
{
    int iif = CheckInterface(chip);
    if (iif != -1)
        if (pChipIF[iif])
        {
            return pChipIF[iif]->FIFOWrite(val, addr, len);
        }

    return 0;
}

int picoc_RFifo(unsigned long addr, void *val, unsigned long len, unsigned long chip)
{
    int iif = CheckInterface(chip);
    if (iif != -1)
        if (pChipIF[iif])
        {
            return pChipIF[iif]->FIFORead(val, addr, len);
        }

    return 0;
}

int picoc_Noop(unsigned long chip)
{
    int iif = CheckInterface(chip);
    if (iif != -1)
        if (pChipIF[iif])
        {
            return pChipIF[iif]->NOOP();
        }

    return 0;
}

int picoc_GetSPIStatus(unsigned long chip)
{
    int iif = CheckInterface(chip);
    if (iif != -1)
        if (pChipIF[iif])
        {
            return pChipIF[iif]->GetSPIStatus();
        }

    return 0;
}

int picoc_SpiTransaction(unsigned char *tx, unsigned char *rx, int bytes, unsigned long chip)
{
    int iif = CheckInterface(chip);
    if (iif != -1)
        if (pChipIF[iif])
        {
            return pChipIF[iif]->SpiTransaction(tx, rx, bytes);
        }

    return 0;
}

int picoc_I2CRead(unsigned char slaveAddr, unsigned char *data, int bytes, int flags, unsigned long chip)
{
    int iif = CheckInterface(chip);
    if (iif != -1)
        if (pChipIF[iif])
        {
            return pChipIF[iif]->I2C_Read(slaveAddr, data, bytes, flags);
        }

    return 0;
}

int picoc_I2CWrite(unsigned char slaveAddr, unsigned char *data, int bytes, int flags, unsigned long chip)
{
    int iif = CheckInterface(chip);
    if (iif != -1)
        if (pChipIF[iif])
        {
            return pChipIF[iif]->I2C_Write(slaveAddr, data, bytes, flags);
        }

    return 0;
}

void picoc_WriteGPIO(unsigned long val, unsigned long chip)
{
    int iif = CheckInterface(chip);
    if (iif != -1)
        if (pChipIF[iif])
        {
            pChipIF[iif]->WriteGPIO(val);
        }
}

unsigned long picoc_ReadGPIO(unsigned long chip)
{
    int iif = CheckInterface(chip);
    if (iif != -1)
        if (pChipIF[iif])
        {
            return pChipIF[iif]->ReadGPIO();
        }

    return 0;
}

#ifdef __cplusplus
}
#endif


