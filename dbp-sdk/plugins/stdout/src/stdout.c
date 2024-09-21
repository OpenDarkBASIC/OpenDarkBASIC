#include <stdio.h>
#include <Windows.h>

#define API __declspec(dllexport)

API void ReceiveCoreDataPtr(void* glob) {}
API void PreDestructor(void) {}
API void Destructor(void) {}

API void print_stdout_str(LPSTR pString)
{
    puts(pString);
    fflush(stdout);
}
API void print_stdout_dword(DWORD value)
{
    printf("%u" "\n", value);
    fflush(stdout);
}
API void print_stdout_float(DWORD value)
{
    float f = *(float*)&value;
    printf("%f\n", f);
    fflush(stdout);
}

API void printc_stdout_str(LPSTR pString)
{
    printf("%s", pString);
    fflush(stdout);
}
API void printc_stdout_dword(DWORD value)
{
    printf("%u", value);
    fflush(stdout);
}
API void printc_stdout_float(DWORD value)
{
    float f = *(float*)&value;
    printf("%f", f);
    fflush(stdout);
}
