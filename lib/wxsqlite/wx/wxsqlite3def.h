///////////////////////////////////////////////////////////////////////////////
// Name:        wxsqlite3def.h
// Purpose:     wxWidgets wrapper around the SQLite3 embedded database library.
// Author:      Ulrich Telle
// Modified by:
// Created:     2005-07-14
// Changes:     2005-10-03  - Upgrade to SQLite3 version 3.2.7
//              2005-10-09  - Corrected error in wxSQLite3Table::FindColumnIndex
//              2005-10-30  - Added wxGTK build support
//              2005-11-01  - Corrected wxSQLite3ResultSet::GetInt64.
//                            Added wxSQLite3Table::GetInt64
//              2005-11-09  - Optionally load SQLite library dynamically
//              2006-02-01  - Upgrade to SQLite3 version 3.3.3
//              2006-02-12  - Upgrade to SQLite3 version 3.3.4 (wxMSW only)
//              2006-03-15  - Fixed a bug in wxSQLite3Database::Prepare
//                            Added wxSQLite3Database::IsOpen for convenience
//              2006-06-11  - Upgrade to SQLite3 version 3.3.6
//                            Added support for optional SQLite meta data methods
//              2007-01-11  - Upgrade to SQLite3 version 3.3.10
//                            Added support for BLOBs as wxMemoryBuffer objects
//                            Added support for loadable extensions
//                            Optional support for key based database encryption
//              2007-02-12  - Upgrade to SQLite3 version 3.3.12
//              2007-05-01  - Upgrade to SQLite3 version 3.3.17
//              2007-10-28  - Upgrade to SQLite3 version 3.5.2
//              2007-11-17  - Fixed a bug in wxSQLite3Database::Close
//                            Eliminated several compile time warnings
//              2007-12-19  - Upgrade to SQLite3 version 3.5.4
//                            Fixed a bug in wxSQLite3Database::Begin
//              2008-01-05  - Added support for shared cache mode
//                            Added support for access to original SQL statement
//                            for prepared statements (requires SQLite 3.5.3 or above)
//              2008-04-27  - Upgrade to SQLite3 version 3.5.8
//                            Fixed several minor issues in the build files
//              2008-06-28  - Upgrade to SQLite3 version 3.5.9
//              2008-07-19  - Upgrade to SQLite3 version 3.6.0
//              2008-09-04  - Upgrade to SQLite3 version 3.6.2
//              2008-11-22  - Upgrade to SQLite3 version 3.6.6
//              2008-12-18  - Upgrade to SQLite3 version 3.6.7
//                            Fixed a bug in method wxSQLite3Table::GetDouble
//              2009-01-14  - Upgrade to SQLite3 version 3.6.10
//                            Added savepoint support
//                            Added IsOk methods to some classes
//              2009-02-21  - Upgrade to SQLite3 version 3.6.11
//                            Added user defined function class for REGEXP operator
//                            Added support for SQLite backup/restore API
//              2009-09-12  - Upgrade to SQLite3 version 3.6.18
//                            Fixed a potential memory leak in wxSQLite3Statement class
//              2009-11-07  - Upgrade to SQLite3 version 3.6.20
//              2010-02-05  - Upgrade to SQLite3 version 3.6.22
//              2010-03-11  - Upgrade to SQLite3 version 3.6.23
//              2010-07-25  - Upgrade to SQLite3 version 3.7.0
//              2010-10-10  - Upgrade to SQLite3 version 3.7.3
//              2010-12-11  - Upgrade to SQLite3 version 3.7.4
//              2011-02-09  - Upgrade to SQLite3 version 3.7.5
//              2011-04-17  - Upgrade to SQLite3 version 3.7.6.1
//              2011-06-30  - Upgrade to SQLite3 version 3.7.7.1
//              2011-08-14  - Progress callback for Backup/Restore added
//              2011-10-25  - Upgrade to SQLite3 version 3.7.8
//              2012-01-17  - Upgrade to SQLite3 version 3.7.10
//              2012-10-17  - Upgrade to SQLite3 version 3.7.14.1
//              2013-03-19  - Upgrade to SQLite3 version 3.7.16
//              2013-08-29  - Upgrade to SQLite3 version 3.8.0
//
// Copyright:   (c) Ulrich Telle
// Licence:     wxWindows licence
///////////////////////////////////////////////////////////////////////////////

/// \file wxsqlite3def.h Compile time switches for the wxSQLite3 class

/** \mainpage wxSQLite3

\section intro What is wxSQLite3?

  \b wxSQLite3 is a C++ wrapper around the public domain <a href="http://www.sqlite.org">SQLite 3.x</a> database
  and is specifically designed for use in programs based on the \b wxWidgets library.

  Several solutions already exist to access SQLite databases. To name just a few:

  - <a href="http://sourceforge.net/projects/wxsqlite">wxSQLite</a> :
    This is a wxWidgets wrapper for version 2.8.x of SQLite.
    SQLite version 3.x has a lot more features - which are not supported by this wrapper.

  - <a href="http://www.codeproject.com/database/CppSQLite.asp">CppSQLite</a> :
    Not wxWidgets specific, but with (partial) support for the newer version 3.x of SQLite.

  - <a href="http://wxcode.sf.net">DatabaseLayer</a> :
    This is a database abstraction providing a JDBC-like interface to database I/O.
    In the current version SQLite3, PostgreSQL, MySQL, Firebird, and ODBC database backends
    are supported.

  The component \b wxSQLite3 was inspired by all three mentioned SQLite wrappers.
  \b wxSQLite3 does not try to hide the underlying database, in contrary almost all special features
  of the SQLite3 version 3.x are supported, like for example the creation of user defined
  scalar or aggregate functions.

  Since SQLite stores strings in UTF-8 encoding, the wxSQLite3 methods provide automatic conversion
  between wxStrings and UTF-8 strings. The methods ToUTF8 and FromUTF8 of the wxString class (available
  since wxWidgets 2.8.4) are used for the conversion. Special care has to be taken if external administration
  tools are used to modify the database contents, since not all of these tools operate in Unicode or UTF-8 mode.

\section version Version history

<dl>

<dt><b>3.0.4</b> - <i>August 2013</i></dt>
<dd>
Upgrade to SQLite version 3.8.0<br>
Added support for querying performance characteristics of prepared statements<br>


</dd>
<dt><b>3.0.3</b> - <i>March 2013</i></dt>
<dd>
Upgrade to SQLite version 3.7.16<br>


</dd>
<dt><b>3.0.2</b> - <i>December 2012</i></dt>
<dd>
Upgrade to SQLite version 3.7.15.1<br>
Corrected an internal SQLite data structure to avoid compile time warnings<br>
Changed method wxSQLite3Exception::ErrorCodeAsString to return the error messages provided by SQLite<br>


</dd>
<dt><b>3.0.1</b> - <i>November 2012</i></dt>
<dd>
Upgrade to SQLite version 3.7.14.1<br>
Cleaned up and optimized Finalize methods<br>
Modified wxSQLite3Database::Close to avoid potential memory leaks<br>
Added method wxSQLite3Database::GetWrapperVersion<br>
Added method wxSQLite3Database::IsReadOnly<br>
Added method wxSQLite3Statement::BindUnixDateTime<br>
Added method wxSQLite3ResultSet::GetUnixDateTime<br>
Added method wxSQLite3ResultSet::GetAutomaticDateTime<br>
Fixed a potential memory leak in method wxSQLite3Database::ExecuteUpdate<br>
Added a wxsqlite3.pc file on request of the Fedora Project developers<br>
Replaced assert by wxASSERT in wxSQLite3Transaction constructor<br>


</dd>
<dt><b>3.0.0</b> - <i>January 2012</i></dt>
<dd>
Upgrade to SQLite version 3.7.10<br>
Added method wxSQLite3Database::Vacuum<br>
Added method wxSQLite3Database::GetDatabaseFilename<br>
Added method wxSQLite3Database::ReleaseMemory<br>
Added method wxSQLite3ResultSet::CursorMoved<br>
Added method wxSQLite3Statement::IsBusy<br>
Fixed a bug in method operator= of wxSQLite3StringCollection
causing an endless recursion on assignment<br>
Dropped the concept of SQLite3 pointer ownership in favor of reference
counted pointers allowing much more flexible use of wxSQLite3 classes<br>
Modified SQLite3 encryption extension (defining int64 datatype
for SHA2 algorithm)<br>
Dropped dbadmin sample from build files<br>
Added Premake support for SQLite3 library with encryption support
and for wxSQLite3 (experimental)<br>

</dd>
<dt><b>2.1.3</b> - <i>August 2011</i></dt>
<dd>
Corrected default behaviour for attached databases in case of
an encrypted main database. (Now the attached database uses the same
encryption key as the main database if no explicit key is given.
Previously the attached database remained unencrypted.)<br>
Added an optional progress callback for metheods Backup and Restore<br>
Added method SetBackupRestorePageCount to set the number of pages
to be copied in one cycle of the backup/restore process<br>

</dd>
<dt><b>2.1.2</b> - <i>July 2011</i></dt>
<dd>
Upgrade to SQLite version 3.7.7.1<br>
Modified wxSQLite3Transaction to make it exception safe<br>

</dd>
<dt><b>2.1.1</b> - <i>April 2011</i></dt>
<dd>
Upgrade to SQLite version 3.7.6.1<br>
Added convenience method wxSQLite3Statement::ExecuteScalar<br>
Changed write-ahead log checkpoint method to new version (v2)<br>

</dd>
<dt><b>2.1.0</b> - <i>March 2011</i></dt>
<dd>
Upgrade to SQLite version 3.7.5<br>
Added wxSQLite+, a database administration application written by Fred Cailleau-Lepetit,
as a GUI sample for wxSQLite3. Minor adjustments were applied to make wxSQLite+
compatible with wxWidgets 2.9.x. Please note that wxSQLite+ is under GPL license.<br>

</dd>
<dt><b>2.0.2</b> - <i>December 2010</i></dt>
<dd>
Upgrade to SQLite version 3.7.4<br>
Added support for rebinding a BLOB object to a new row<br>
Added support for determining if an SQL statement writes the database<br>

</dd>
<dt><b>2.0.1</b> - <i>October 2010</i></dt>
<dd>
Upgrade to SQLite version 3.7.3<br>
Added parameter transferStatementOwnership to method wxSQLite3Statement::ExecuteQuery
to allow using the returned result set beyond the life time of the wxSQLite3Statement instance<br>
Eliminated the use of sqlite3_mprintf which caused linker problems when loading SQLite dynamically<br>

</dd>
<dt><b>2.0.0</b> - <i>July 2010</i></dt>
<dd>
Upgrade to SQLite version 3.7.0<br>
Fixed a bug in class wxSQLite3ResultSet<br>
Added support for SQLite's write-ahead log journal mode<br>
Added support for named collections (see class wxSQLite3NamedCollection)<br>
Changed UTF-8 string handling to use methods To/FromUTF8 of the wxString class (requires wxWidgets 2.8.4 or higher)<br>
Compatible with wxWidgets 2.9.1<br>

</dd>
<dt><b>1.9.9</b> - <i>March 2010</i></dt>
<dd>
Upgrade to SQLite version 3.6.23<br>
Fixed a bug when compiling for dynamic loading of SQLite<br>
Added static methods for accessing the run-time library compilation options diagnostics<br>
Added mathod FormatV to class wxSQLite3StatementBuffer<br>

</dd>
<dt><b>1.9.8</b> - <i>February 2010</i></dt>
<dd>
Upgrade to SQLite version 3.6.22<br>
Fixed a bug when compiling without precompiled header support
(by including wx/arrstr.h)<br>

</dd>
<dt><b>1.9.7</b> - <i>November 2009</i></dt>
<dd>
Upgrade to SQLite version 3.6.20<br>
Added methods to query, enable or disable foreign key support<br>

</dd>
<dt><b>1.9.6</b> - <i>September 2009</i></dt>
<dd>
Upgrade to SQLite version 3.6.18<br>
Added method to get the SQLite library source id<br>
Added flags parameter to wxSQLite3Database::Open to allow additional control over the database
connection (see http://www.sqlite.org/c3ref/open.html for further information)<br>
Fixed a potential memory leak in wxSQLite3Statement class<br>
Converted encryption extension from C++ to pure C to make it
compatible with the SQLite amalgamation.<br>

</dd>
<dt><b>1.9.5</b> - <i>February 2009</i></dt>
<dd>
Upgrade to SQLite version 3.6.11<br>
Added user defined function class for REGEXP operator.<br>
Added support for SQLite backup/restore API, introduced with SQLite 3.6.11<br>

</dd>
<dt><b>1.9.4</b> - <i>January 2009</i></dt>
<dd>
Upgrade to SQLite version 3.6.10<br>
Added support for savepoints, introduced with SQLite 3.6.8<br>
Added method IsOk to the classes wxSQLite3Statement, wxSQLite3Table and wxSQLite3ResultSet,
thus instances of these classes can be checked whether the associated SQLite database or
statement are valid without throwing an exception.<br>

</dd>
<dt><b>1.9.3</b> - <i>December 2008</i></dt>
<dd>
Upgrade to SQLite version 3.6.7<br>
Fixed a bug in method wxSQLite3Table::GetDouble
(conversion from string to double failed in non-US locales)<br>
Build system upgraded using Bakefile 0.2.5<br>

</dd>
<dt><b>1.9.2</b> - <i>November 2008</i></dt>
<dd>
Upgrade to SQLite version 3.6.6<br>
Added RAII transaction class (see docs for details)<br>

</dd>
<dt><b>1.9.1</b> - <i>September 2008</i></dt>
<dd>
Upgrade to SQLite version 3.6.2<br>
Introduced own step counting for aggregate user functions
since the sqlite3_aggregate_count function is now deprecated<br>
Enhanced wxSQLite3Database::TableExists method to query an attached database
for existence of a table or to query the main database and all attached databases<br>

</dd>
<dt><b>1.9.0</b> - <i>July 2008</i></dt>
<dd>
Upgrade to SQLite version 3.6.0<br>
The optional key based encryption support has been adapted to
support SQLite version 3.6.0.<br>
Added static methods to initialize and shutdown the SQLite library.<br>
Changed build system to support static library build against shared
wxWidgets build on Linux.<br>
Changed behaviour of wxSQLite3Database::Close method to finalize
all unfinalized prepared statements.

</dd>
<dt><b>1.8.5</b> - <i>June 2008</i></dt>
<dd>
Upgrade to SQLite version 3.5.9<br>
Integration of the optional key based encryption support into SQLite
has been made easier. Changes to original SQLite source files
are no longer necessary.

</dd>
<dt><b>1.8.4</b> - <i>April 2008</i></dt>
<dd>
Upgrade to SQLite version 3.5.8<br>
Added support for accessing database limits<br>
Changed method TableExists to check a table name case insensitive<br>
Fixed several minor issues in the build files.

</dd>
<dt><b>1.8.3</b> - <i>January 2008</i></dt>
<dd>
Added support for shared cache mode<br>
Added support for access to original SQL statement
for prepared statements (requires SQLite 3.5.3 or above)

</dd>
<dt><b>1.8.2</b> - <i>December 2007</i></dt>
<dd>
Upgrade to SQLite version 3.5.4<br>
Fixed a bug in wxSQLite3Database::Begin (wrong transaction type)

</dd>
<dt><b>1.8.1</b> - <i>November 2007</i></dt>
<dd>
Fixed a bug in in wxSQLite3Database::Close (resetting flag m_isEncrypted)<br>
Eliminated several compile time warnings (regarding unused parameters)<br>
Fixed a compile time bug in wxSQLite3Database::GetBlob (missing explicit type cast)

</dd>
<dt><b>1.8.0</b> - <i>November 2007</i></dt>
<dd>
Upgrade to SQLite version 3.5.2<br>
Support for SQLite incremental BLOBs<br>
 Changed source code in the SQLite3 encryption extension to eliminate several warnings<br>
Changed default wxWidgets version to 2.8.x<br>
Adjusted sources for SQLite encryption support are included for all SQLite version from 3.3.1 up to 3.5.2<br>
SQLite link libraries for MinGW on Windows are included<br>
Added <code>WXMAKINGLIB_WXSQLITE3</code> compile time option
to support building wxSQLite3 as a static library while
using the shared libraries of wxWidgets.

</dd>
<dt><b>1.7.3</b> - <i>May 2007</i></dt>
<dd>
Upgrade to SQLite version 3.3.17<br>

Fixed a bug in the SQLite3 encryption extension
(MD5 algorithm was not aware of endianess on
big-endian platforms, resulting in non-portable
database files)

</dd>
<dt><b>1.7.2</b> - <i>February 2007</i></dt>
<dd>
Upgrade to SQLite version 3.3.12<br>
Support for loadable extensions is now optional
Check for optional wxSQLite3 features at runtime
wxSQLite3 API independent of optional features

</dd>
<dt><b>1.7.1</b> - <i>January 2007</i></dt>
<dd>
Fixed a bug in the key based database encryption feature
(The call to <b>sqlite3_rekey</b> in wxSQLite3Database::ReKey
could cause a program crash, when used to encrypt a previously
unencrypted database.)<br>

</dd>
<dt><b>1.7.0</b> - <i>January 2007</i></dt>
<dd>
Upgrade to SQLite version 3.3.10 (<b>Attention</b>: at least SQLite version 3.3.9 is required)<br>
Added support for BLOBs as wxMemoryBuffer objects<br>
Added support for loadable extensions<br>
Optional support for key based database encryption

</dd>
<dt><b>1.6.0</b> - <i>July 2006</i></dt>
<dd>
Added support for user defined collation sequences

</dd>
<dt><b>1.5.3</b> - <i>June 2006</i></dt>
<dd>
Upgrade to SQLite version 3.3.6<br>
Added support for optional SQLite meta data methods

</dd>
<dt><b>1.5.2</b> - <i>March 2006</i></dt>
<dd>
Fixed a bug in wxSQLite3Database::Prepare<br>
Added wxSQLite3Database::IsOpen for convenience

</dd>
<dt><b>1.5.1</b> - <i>February 2006</i></dt>
<dd>
Upgrade to SQLite version 3.3.4 (wxMSW only)

</dd>
<dt><b>1.5</b> - <i>February 2006</i></dt>
<dd>
Upgrade to SQLite version 3.3.3<br>
Added support for commit, rollback and update callbacks

</dd>
<dt><b>1.4.2</b> - <i>November 2005</i></dt>
<dd>
Optimized code for wxString arguments

</dd>
<dt><b>1.4.1</b> - <i>November 2005</i></dt>
<dd>
Fixed a bug in wxSQLite3Database::TableExists,<br>
Changed the handling of Unicode string conversion,<br>
Added support for different transaction types

</dd>
<dt><b>1.4</b> - <i>November 2005</i></dt>
<dd>
Optionally load the SQLite library dynamically at run time.

</dd>
<dt><b>1.3.1</b> - <i>November 2005</i></dt>
<dd>
Corrected wxSQLite3ResultSet::GetInt64.<br>
Added wxSQLite3Table::GetInt64

</dd>
<dt><b>1.3</b> - <i>October 2005</i></dt>
<dd>
Added wxGTK build support<br>

</dd>
<dt><b>1.2</b> - <i>October 2005</i></dt>
<dd>
Corrected error in wxSQLite3Table::FindColumnIndex<br>

</dd>
<dt><b>1.1</b> - <i>October 2005</i></dt>
<dd>
Upgrade to SQLite version 3.2.7 <br>

</dd>

<dt><b>1.0</b> - <i>July 2005</i></dt>
<dd>
First public release
</dd>
</dl>

\author Ulrich Telle (ulrich DOT telle AT gmx DOT de)

\section ackn Acknowledgements

Kudos to <b>Fred Cailleau-Lepetit</b> for developing <b>wxSQLite+</b> as a sample demonstrating
the wxWidgets components <b>wxAUI</b> and <b>wxSQLite3</b> and for allowing it to be included
in the wxSQLite3 distribution.

The following people have contributed to wxSQLite3:

<ul>
<li>Francesco Montorsi (enhancement of the build system)</li>
<li>Neville Dastur (enhancement of the method TableExists)</li>
<li>Tobias Langner (RAII class for managing transactions)</li>
</ul>

 */

#ifndef _WX_SQLITE3_DEF_H_
#define _WX_SQLITE3_DEF_H_

#if defined(WXMAKINGLIB_WXSQLITE3)
  #define WXDLLIMPEXP_SQLITE3
#elif defined(WXMAKINGDLL_WXSQLITE3)
  #define WXDLLIMPEXP_SQLITE3 WXEXPORT
#elif defined(WXUSINGDLL_WXSQLITE3)
  #define WXDLLIMPEXP_SQLITE3 WXIMPORT
#else // not making nor using DLL
  #define WXDLLIMPEXP_SQLITE3
#endif

/*
  GCC warns about using __declspec on forward declarations
  while MSVC complains about forward declarations without
  __declspec for the classes later declared with it. To hide this
  difference a separate macro for forward declarations is defined:
 */
#if defined(HAVE_VISIBILITY) || (defined(__WINDOWS__) && defined(__GNUC__))
  #define WXDLLIMPEXP_FWD_SQLITE3
#else
  #define WXDLLIMPEXP_FWD_SQLITE3 WXDLLIMPEXP_SQLITE3
#endif

#endif // _WX_SQLITE3_DEF_H_
