#pragma once

#include <cstdint>
#include <cstdio>

namespace odbc {
    namespace db {
        class Driver;
    }
}

typedef void* dbscan_t;
typedef struct yy_buffer_state* YY_BUFFER_STATE;
typedef union DBSTYPE DBSTYPE;

int dblex_init(dbscan_t* ptr_yy_globals);
int dblex_init_extra(odbc::db::Driver* db_user_defined, dbscan_t* ptr_yy_globals);
int dblex_destroy(dbscan_t yyscanner);
void dbset_in(FILE* _in_str, dbscan_t dbscanner);
YY_BUFFER_STATE db_scan_bytes(const char *bytes, int len , dbscan_t dbscanner);
void db_delete_buffer(YY_BUFFER_STATE b , dbscan_t dbscanner);
int dblex(DBSTYPE* dblval_param , dbscan_t dbscanner);
odbc::db::Driver* dbget_extra(dbscan_t dbscanner);
