External libraries' sources here.

wxSQLite3 and sqlite3 sources from:
http://svn.code.sf.net/p/wxcode/code/trunk/wxCode/components/wxsqlite3/

(ensures wxSQLite3 and sqlite3 are in sync)
--------------------------------------------

Current versions: wxSQLite3 Version: 3.0.3
                  SQLite Version: 3.7.17
===============================================================================                
Copy file
from: wxsqlite3\Readme.txt
to  : MoneyManagerEX\trunk\mmex\lib\wxsqlite\Readme.txt 

Copy all files
From: wxsqlite3\src
to  : MoneyManagerEX\trunk\mmex\lib\wxsqlite

From: wxsqlite3\include\wx
to  : MoneyManagerEX\trunk\mmex\lib\wxsqlite\wx 

from: wxsqlite3\sqlite3\secure\src
to  : MoneyManagerEX\trunk\mmex\lib\sqlite
===============================================================================                
Source files for: wxSQLite3 Version 3.0.3
  wxsqlite3.cpp

Headers of for: wxSQLite3 - wx:
  wxsqlite3.h
  wxsqlite3def.h
  wxsqlite3dyn.h
  wxsqlite3opt.h

Source files for: wxSQLite3 - sqlite3 interface
  codec.c
  codec.h
  codecext.c
  extensionfunctions.c
  rijndael.c
  rijndael.h
  sha2.c
  sha2.h
  sqlite3secure.c
  sqlite3.def

Source files for: sqlite3 Version 3.7.17
  sqlite3.c
  sqlite3.h
  sqlite3ext.h
  sqlite3dyn.h
