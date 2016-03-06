simple ORM to access database

Creating New Database Tables
--------------------------------------------------------------------------
1. New database table definitions are addded to the submodule: database
   The definitions are added to the tables_v1.sql
2. Use the files in the /util directory to construct the database files
   that reside in /src/db
   a) Windows: Use the build_db_xx.bat to generate and relocate the
      generated files to the correct directory.
   b) Other OS: Refer to the readme.txt file in the /util directory to
      generate the files.
3. Create associated Model_xx.h and Model_xx.cpp files and add them to
   the /src/model directory.

   a) Model_Pragma.h and Model_Pragma.cpp found in the /util directory
      can be used as a template to help in the construction of the new
	  Model_xx.h and Model_xx.cpp files for the associated DB_Table_xx.h
	  files.
	  
Updates to the Database Table
--------------------------------------------------------------------------
Every time a change is made to the DB structure or changes to the initial
data such as category or currency changes, this change must be done in
the submodule: database

1. Every time a change is made for any new release, 
   a new file: database_version_N+1 should be created.
2. Both build_db_tables.bat and build_db_upgrade.bat should be run to
   recreate the .h files
