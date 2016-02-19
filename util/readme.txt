File    : sqlite2cpp.py
Purpose : To construct database accessing code in C++ for a given
          table definition defined in file: ../database/tables_v1.sql
Usage   : python sqlite2cpp.py path_to_sql_file
-----------------------------------------------------------------------

File    : sqliteupgrade2cpp.py
Purpose : To construct database upgrade code in C++ for a given
          table upgrade files in folder: ../database
Usage   : python sqlite2cpp.py path_to_sql_file
-----------------------------------------------------------------------

File    : build_db_tables.bat
Purpose : To allow easy installation of DB_Table_xxx.h files
          by using file: sqlite2cpp.py
Usage   : Running batch file from current location will copy
          result files to the destination directory: ../src/db or
          relocate batch file to execute from the destination directory.
------------------------------------------------------------------------

File    : build_db_upgrade.bat
Purpose : To allow easy installation of DB_Upgrade.h file
          by using file: sqliteupgrade2cpp.py
Usage   : Running batch file from current location will copy
          result file to the destination directory: ../src/db or
          relocate batch file to execute from the destination directory.
------------------------------------------------------------------------
         
File    : checkEOL.bat
Purpose : Check if a source files contains unix style line endings (LF)
          or windows style line endings (CR LF).
------------------------------------------------------------------------

To compress mmex binary file use

mpress http://www.matcode.com/mpress.htm
or
upx http://upx.sourceforge.net/