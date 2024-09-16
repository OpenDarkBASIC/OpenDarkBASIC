#include "odb-util/backtrace.h"
#include <process.h>
#include <Windows.h>
#include <DbgHelp.h>
#include <stdio.h>

#define ODBUTIL_BACKTRACE_FUNC_LEN 1024

static HANDLE hProcess;

/* ------------------------------------------------------------------------- */
int
backtrace_init(void)
{
    /*if (SymInitialize(hProcess, NULL, TRUE) != TRUE)
        return -1;*/
    SymInitialize(hProcess, NULL, TRUE);
    return 0;
}

/* ------------------------------------------------------------------------- */
void
backtrace_deinit(void)
{
    /* XXX: This crashes for some reason?
    SymCleanup(hProcess);*/
}

/* ------------------------------------------------------------------------- */
char**
backtrace_get(int* size)
{
    char** result;
    char** current_ptr;
    char* current_str;
    void* stack[ODBUTIL_MEM_BACKTRACE_SIZE];
    char sym_buf[sizeof(SYMBOL_INFO) + (ODBUTIL_BACKTRACE_FUNC_LEN - 1) * sizeof(TCHAR)];
    WORD frames_traced = CaptureStackBackTrace(0, ODBUTIL_MEM_BACKTRACE_SIZE, stack, NULL);

    result = malloc(
        sizeof(char*) * frames_traced +  /* String table */
        sizeof(char)  * frames_traced * ODBUTIL_BACKTRACE_FUNC_LEN);
    current_ptr = result;
    current_str = (char*)(result + frames_traced);

    SYMBOL_INFO* sym = (SYMBOL_INFO*)sym_buf;
    sym->MaxNameLen = ODBUTIL_BACKTRACE_FUNC_LEN;
    sym->SizeOfStruct = sizeof(SYMBOL_INFO);

    DWORD displacement;
    IMAGEHLP_LINE64 line;
    for (int i = 0; i < frames_traced; ++i)
    {
        DWORD64 address = (DWORD64)(stack[i]);
        SymFromAddr(hProcess, address, NULL, sym);
        *current_ptr = current_str;
        current_ptr++;
        if (SymGetLineFromAddr64(hProcess, address, &displacement, &line) == TRUE)
            current_str += snprintf(current_str, ODBUTIL_BACKTRACE_FUNC_LEN, "%d: (0x%llx+0x%x) %s:%d", i, sym->Address, displacement, sym->Name, line.LineNumber) + 1;
        else
            //current_str += sprintf(current_str, "%d: (0x%llx) SymGetLineFromAddr64() failed: %d", i, address, GetLastError()) + 1;
            current_str += sprintf(current_str, "%d: (0x%llx) ...", i, address) + 1;

    }
    
    *size = (int)frames_traced;
    return result;
}

/* ------------------------------------------------------------------------- */
void
backtrace_free(char** bt)
{
    free(bt);
}
