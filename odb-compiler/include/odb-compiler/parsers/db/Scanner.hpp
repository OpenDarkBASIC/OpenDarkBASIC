/*!
 * @brief The header file generated by FLEX is just downright broken and doesn't
 * compile. This file forward declares stuff that FLEX's auto generated header
 * would normally declare.
 */

#pragma once

#include <cstdint>
#include <cstdio>

namespace odb {
    namespace db {
        class Driver;
    }
}

typedef void* dbscan_t;
typedef struct yy_buffer_state* YY_BUFFER_STATE;
typedef union DBSTYPE DBSTYPE;
typedef struct DBLTYPE DBLTYPE;
typedef struct dbpstate dbpstate;

int dblex_init(dbscan_t* ptr_yy_globals);
int dblex_init_extra(odb::db::Driver* db_user_defined, dbscan_t* ptr_yy_globals);
int dblex_destroy(dbscan_t yyscanner);
void dbset_in(FILE* _in_str, dbscan_t dbscanner);
YY_BUFFER_STATE db_scan_bytes(const char *bytes, int len, dbscan_t dbscanner);
void db_delete_buffer(YY_BUFFER_STATE b , dbscan_t dbscanner);
int dblex(DBSTYPE* dblval_param, DBLTYPE* yylloc_param, dbscan_t dbscanner);
odb::db::Driver* dbget_extra(dbscan_t dbscanner);
char* dbget_text(dbscan_t yyscanner);
DBSTYPE* dbget_lval(dbscan_t scanner);
