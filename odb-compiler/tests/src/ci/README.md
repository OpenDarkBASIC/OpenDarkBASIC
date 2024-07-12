The CI program scans this directory recursively for all files ending in ```.dba```.

The  filename  encodes  how  the  result should be interpreted. ```c```  means
"compile",  ```r```  means "run", ```y``` means "yes" and ```n``` means  "no".
  + To assert a snippet must compile on  both  odbc  and  dbpc,  the file should
  start with ```cy-```
  +  To assert a snippet must fail to compile on both odbc and  dbpc,  the  file
  should start with ```cn-```
  + To assert a snippet must compile on odbc but  fail  to  compile on dbpc, the
  file should start with ```odbc-```
  + To assert a snippet  must  compile  on dbpc but fail to compile on odbc, the
  file should start with ```dbpc-```

If  a  file  is  asserted  to compile successfully, then we  can  additionally
specify what the runtime behavior should be.
  + To assert a  snippet's  output  must be identical on both odbc and dbpc, the
  file should start with ```cy-ry-```
  + To assert a snippet's output must be  different  between  odbc and dbpc, the
  file should start with ```cy-rn-```

If the snippet only compiles on one compiler, or if  the  runtime  behavior is
different between odbc and dbpc, then we must also specify the expected output
by creating an identically  named  file  ending in either ```.out``` or in the
case of multiple  different  behaviors,  ```.dbpout```  and ```.odbout```. For
example, if our snippet should only  compile  with  odbc, then we would create
the two files:
  + odbc-description of test.dba
  + odbc-description of test.out

If our snippet compiles on both but has  different  runtime  behavior, then we
would create the three files:
  + cy-rn-description of test.dba
  + cy-rn-description of test.dbpout
  + cy-rn-description of test.odbout

Everything  after the initial encoding (and before the file extension)  should
be a description of what the snippet is trying to test. This is usually just a
sentence with spaces between each word.

