#include <stdio.h>
#include <iostream>
#include <unwind.h>
#include <dlfcn.h>
#include <cxxabi.h>
#include <stdint.h>
#include <string>
#include <cstring>
#include <cstdlib>
#include "leakInspector.h"

using namespace std;

struct android_backtrace_state
{
    void **current;
    void **end;
};

_Unwind_Reason_Code android_unwind_callback(struct _Unwind_Context* context, 
                                            void* arg)
{
    android_backtrace_state* state = (android_backtrace_state *)arg;
    uintptr_t pc = _Unwind_GetIP(context);
    if (pc) 
    {
        if (state->current == state->end) 
        {
            return _URC_END_OF_STACK;
        } 
        else 
        {
            *state->current++ = reinterpret_cast<void*>(pc);
        }
    }
    return _URC_NO_REASON;
}

struct TraceInfo* getBackTraceInfo(void)
{
    //_my_log("android stack dump");

    const int max = 100;
    void* buffer[max];

    android_backtrace_state state;
    state.current = buffer;
    state.end = buffer + max;

    _Unwind_Backtrace(android_unwind_callback, &state);

    int count = (int)(state.current - buffer);

    // Allocate the memory for structure which collects the traceInfo
    // This should be done only once. Hence keeping it under the static variable.

    struct TraceInfo *traceInfo;
    traceInfo = (struct TraceInfo*)  malloc(sizeof(struct TraceInfo));    

    // Set Max trace length to 5
    if(count > 5)
        count = 5;

    traceInfo->noOfsymbols = count;

    // Allocate memory for count of symbols (At max it is 5)
    traceInfo -> symbols = (char **) malloc(count * sizeof(char*)); 

    for (int idx = 0; idx < count; idx++) 
    {
        const void* addr = buffer[idx];
        const char* symbol = "";

        Dl_info info;
        if (dladdr(addr, &info) && info.dli_sname) 
        {
            symbol = info.dli_sname;
               string s1 = symbol;
        }
       
        int status = 0; 
        char *demangled = __cxxabiv1::__cxa_demangle(symbol, 0, 0, &status); 
    
        /* Find the length of a symbol and allocate the memory accordingly */
        int symbolLength = 0;
        for(int i = 0; demangled != NULL && demangled[i] != '\0'; i++){
            symbolLength++;
        }

        //printf("Symbol length is %d\n", symbolLength);
        // If demanged is NULL, just store the "-". Due to some reason this symbol is not fetched
        if(demangled == NULL){
            traceInfo -> symbols[idx] = (char *) malloc( 1 * sizeof(char));
            traceInfo -> symbols[idx][0] = '-';
        } else {
            // Allocate memory for the symbol and store the symbol
            traceInfo -> symbols[idx] = (char *) malloc( symbolLength * sizeof(char));
            for(int i = 0; i < symbolLength; i++){
                traceInfo -> symbols[idx][i] = demangled[i];
            }
        }

        /*printf("%03d: 0x%p %s \n",
                idx,
                addr,
                (NULL != demangled && 0 == status) ?
                demangled : symbol);*/
        


       // if (NULL != demangled)
         //   free(demangled);        
    }

 return traceInfo;


}

