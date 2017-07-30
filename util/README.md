+ **[sqlite2cpp.py]**

  To construct database accessing code in C++ for a given table definition
  defined in file `../database/tables_v1.sql`
  ```
  python sqlite2cpp.py path_to_sql_file
  ```

+ **[sqliteupgrade2cpp.py]**

  To construct database upgrade code in C++ for a given table upgrade files
  in folder `../database`
  ```
  python sqlite2cpp.py path_to_sql_file
  ```

+ **[build_db_tables.bat]**

  To allow easy installation of `DB_Table_xxx.h` files by using file:
  `sqlite2cpp.py`

  Running batch file from current location will copy result files to the
  destination directory `../src/db` or relocate batch file to execute from
  the destination directory.

+ **[build_db_upgrade.bat]**

  To allow easy installation of `DB_Upgrade.h` file by using file:
  `sqliteupgrade2cpp.py`

  Running batch file from current location will copy result file to the
  destination directory `../src/db` or relocate batch file to execute from
  the destination directory.

+ **[checkEOL.bat]**

  Check if a source files contains unix style line endings (LF) or Windows
  style line endings (CR LF).

+ **[update-po-files.sh]**

  Extracts all translateable strings from source files and updates .po and
  .pot files in `../po` directory with new or changed strings to be
  translated.

+ To compress mmex executable file use [mpress] or [upx].

[sqlite2cpp.py]: sqlite2cpp.py
[sqliteupgrade2cpp.py]: sqliteupgrade2cpp.py
[build_db_tables.bat]: build_db_tables.bat
[build_db_upgrade.bat]: build_db_upgrade.bat
[checkEOL.bat]: checkEOL.bat
[update-po-files.sh]: update-po-files.sh
[mpress]: http://www.matcode.com/mpress.htm
[upx]: http://upx.sourceforge.net/