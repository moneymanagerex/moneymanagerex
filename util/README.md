+ **[sqlite2cpp.py]**

  To construct database accessing code in C++ for a given table definition
  defined in file `../database/tables.sql`
  ```
  python sqlite2cpp.py path_to_tables_file
  ```

+ **[sqliteupgrade2cpp.py]**

  To construct database upgrade code in C++ for a given table upgrade files
  in folder `../database/incremental_upgrade`
  ```
  python sqliteupgrade2cpp.py path_to_database_version_NN_files
  ```

+ **[build_db_tables.bat]**

  To allow easy installation of `Table_xxx.h` files by using file:
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

+ **[Doxyfile]**

  Configuration file for [doxygen] tool used to generate the documentation
  for future developers from the source code.

  To generate HTML documentation into new html subdirectory simply run
  `doxygen` in this directory then open `./html/index.html` with any DHTML,
  Javascript and CSS enabled web browser.

  To see all available doxygen options and their descriptions update Doxyfile
  with `doxygen -u`.

+ To compress mmex Windows executable file use [mpress] or [upx].

[sqlite2cpp.py]: sqlite2cpp.py
[sqliteupgrade2cpp.py]: sqliteupgrade2cpp.py
[build_db_tables.bat]: build_db_tables.bat
[build_db_upgrade.bat]: build_db_upgrade.bat
[checkEOL.bat]: checkEOL.bat
[update-po-files.sh]: update-po-files.sh
[Doxyfile]: Doxyfile
[doxygen]: http://www.stack.nl/~dimitri/doxygen/index.html
[mpress]: http://www.matcode.com/mpress.htm
[upx]: http://upx.sourceforge.net/