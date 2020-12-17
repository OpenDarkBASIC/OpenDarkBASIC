import pathlib
import re

bfiles = list()
bfiles += list(pathlib.Path("bison").rglob("*.c"))
bfiles += list(pathlib.Path("bison").rglob("*.h"))
bfiles += list(pathlib.Path("bison").rglob("*.y"))

ufiles = list()
ufiles += list(pathlib.Path("bison-util").rglob("*.c"))
ufiles += list(pathlib.Path("bison-util").rglob("*.h"))
ufiles += list(pathlib.Path("bison-util").rglob("*.y"))

for path in bfiles + ufiles:
    contents = open(path, "rb").read().decode("utf-8")
    for match in re.findall(r'#include [<"](.*)[>"]', contents):
        for b in bfiles:
            if match == b.name:
                contents = re.sub(rf'#include [<"]{match}[>"]', rf'#include "bison/{match}"', contents)
                break
        for u in ufiles:
            if match == u.name:
                contents = re.sub(rf'#include [<"]{match}[>"]', rf'#include "bison-util/{match}"', contents)
                break
    open(path, "wb").write(contents.encode("utf-8"))

