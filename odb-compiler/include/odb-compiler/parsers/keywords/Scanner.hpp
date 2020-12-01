#pragma once

#include <cstdint>
#include <cstdio>

namespace odb {
    namespace kw {
        class Driver;
    }
}

typedef void* kwscan_t;
typedef struct yy_buffer_state* YY_BUFFER_STATE;
typedef union KWSTYPE KWSTYPE;
typedef struct KWLTYPE KWLTYPE;
typedef struct kwpstate kwpstate;

extern int kwlex_init(kwscan_t* ptr_yy_globals);
extern int kwlex_init_extra(odb::kw::Driver* kw_user_defined, kwscan_t* ptr_yy_globals);
extern int kwlex_destroy(kwscan_t yyscanner);
extern void kwset_in(FILE* _in_str, kwscan_t kwscanner);
extern YY_BUFFER_STATE kw_scan_bytes(const char *bytes, int len , kwscan_t kwscanner);
extern void kw_delete_buffer(YY_BUFFER_STATE b , kwscan_t kwscanner);
extern int kwlex(KWSTYPE * kwlval_param, KWLTYPE* yylloc_param, kwscan_t kwscanner);
odb::kw::Driver* kwget_extra(kwscan_t kwscanner);
