cmake_minimum_required (VERSION 3.2)
project ("odb-runtime-dbp")

add_library (odb-runtime-dbp SHARED
    "src/globstruct.h"
    "src/RuntimeDBP.cpp"
)

add_library (odb-runtime-dbp-prelude STATIC
    "src/Prelude.cpp"
)

install (
    TARGETS
        odb-runtime-dbp
        odb-runtime-dbp-prelude
    DESTINATION
        .
)
