/***********************************************************************
 * Copyright (C) 2009  Andrei Taranchenko
 * Contact: http://www.nulidex.com/contact

 This library is free software; you can redistribute it and/or
 modify it under the terms of the GNU Lesser General Public
 License as published by the Free Software Foundation; either
 version 2.1 of the License, or (at your option) any later version.

 This library is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 Lesser General Public License for more details.

 You should have received a copy of the GNU Lesser General Public
 License along with this library; if not, write to the Free Software
 Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 **********************************************************************/
#ifndef html_template_h
#define html_template_h

#include <string>
#include <vector>
#include <map>
#include <set>
#include <cctype>
#include <list>
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <stdexcept>
#include <assert.h>

/*!\mainpage
  C++ port of Sam Tregar's Perl HTML::Template. This does NOT have
  all the features of the original HTML::Template [yet], but the goal is to
  have something close to this in C++, which is a very underrated language for
  CGI programming (IMHO).

  Be sure to check out GNU <a
  href="http://www.gnu.org/software/cgicc/cgicc.html">cgicc</a>
  for web based applications in C++.

  Combined with this package, you no longer have to recompile code for
  layout changes to take effect.
*/

//! cgi template namespace
namespace tmpl {

//----------------------------------------------------------------------------
// enumeration
//----------------------------------------------------------------------------
enum en_escape_mode {ESC_NONE, ESC_JS, ESC_HTML, ESC_URL};

//----------------------------------------------------------------------------
// static
//----------------------------------------------------------------------------

//! Definition of all valid characters in a variable name. Note that anything
// goes for TMPL_INCLUDE
static const std::string str_valid_chars =
  "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_.-0123456789";

//----------------------------------------------------------------------------
// utilities
//----------------------------------------------------------------------------
//! upper case string helper
void uc(std::string & str);
//! upper case string helper
char char_toupper(char ch);

//! predicate for searching strings ignoring case
bool bin_predicate_search_nocase(char ch1, char ch2);

//! search a string ignoring case
const std::string::size_type find_no_case(
  const std::string & str_src, const std::string & str_find, const size_t start_pos
);

//! determine file dir from path
const std::string file_directory(const std::string & str_path);

//! trim string
std::string trim_string(const std::string & str, const std::string  characters = " \t\n\r");

//! search/replace string by reference
void search_replace(std::string & str_src, const std::string str_to_find, 
                    const std::string & str_replace);

//! escape a URL
const std::string rfc1738_encode(const std::string & src);

//----------------------------------------------------------------------------
// exceptions
//----------------------------------------------------------------------------
//! template syntax exception class
class syntax_ex : public std::runtime_error 
{
 public:
   //! path to template that caused the error
   std::string template_path;

   //! problem line
   size_t line;
   //! error detail
   std::string detail;

   //! ctructor
   syntax_ex(const std::string & str_what) :
   std::runtime_error(str_what) 
   {
    detail = "";
    line   = 0;
   }

   //! dtructor
   ~syntax_ex() throw() {} 
};
//---------------------------------------------------------------------------

//! template runtime exception class
class runtime_ex : public std::runtime_error 
{
 public:
   //! path to template that caused the error
   std::string template_path;

   //! ctructor
   runtime_ex(const std::string & str_what) : std::runtime_error(str_what) 
   {
    template_path = "";
   }

   //!dtructor
   ~runtime_ex() throw() {} 
};

//----------------------------------------------------------------------------
// tag type object
//----------------------------------------------------------------------------
//! tag type struct - defines one of the supported tags
struct tag_type_s
{
 //! tag type class - general tag class
 std::string str_tag_class;
 //! specific tag typy (IF, LOOP, etc.)
 std::string str_tag_type;
 //! flag that marks if this is a block tag (has open and close)
 bool b_block_tag;

 //! ctructor
 tag_type_s();

 //! non-default ctructor that assigns class and type in one whack
 tag_type_s(const std::string & arg_type,
            const std::string & arg_class,
            bool arg_block_tag = false);

 //! returns TRUE if the tag is declared but not defined
 const bool Empty() const {return str_tag_class.empty() ;}

 //! equals operator
 bool operator== (const tag_type_s & rhs) const;
 //! not equals operator
 bool operator!= (const tag_type_s & rhs) const;
};

//----------------------------------------------------------------------------
// tag object
//----------------------------------------------------------------------------
//! Tag object.
/*! This object marks a specific tag in the body of a template. It can be part
 * of a block tag (<IF> , <LOOP>) or a standalone tag, such as <VAR>
*/
struct tag_s
{
 private:
   //! tag type object - tells us what tag this is 
   tag_type_s tag_type;
   //! tag name
   std::string str_name;
   //! beginning of the tag in text
   size_t begin;
   //! end of the tag in text
   size_t end;

   //! is the tag a closing tag in a block?
   bool b_termination;
   //! is the tag a splitter tag in the block?
   bool b_splitter;
   //! escape mode
   en_escape_mode escape_mode;

 public:
   //! ctructor that assigns tag type - the tag is no longer empty
   tag_s(const tag_type_s & arg_type);
   //! default ctructor - Empty() will return TRUE with this
   tag_s();

   //! return tag type
   const tag_type_s Get_Tag_Type() const {return tag_type;}
   //! return tag name as defined in template
   const std::string Get_Name() const {return str_name;}
   //! get begin position
   const size_t Start() const {return begin;}
   //! get end position
   const size_t Stop() const {return end;}
   //! get length of the tag 
   const size_t Length() const;

   //! see if the tag has a name. split tags and closing tags don't have a
   //name. In effect, this is to know if the tag is an opener for a block.
   const bool Is_Named() const {return !str_name.empty();}
   //! is the tag empty?
   const bool Empty() const;
   //! is the tag closing a block
   const bool Is_Termination() const {return b_termination;}
   //! is the tag splitting a block (ELSE)
   const bool Is_Splitter() const {return b_splitter;}
   const en_escape_mode Get_Escape_Mode() const {return escape_mode;}

   //! assign begin position
   void Set_Start(const size_t arg) {begin = arg;} 
   //! assign end position
   void Set_Stop(const size_t arg) {end = arg;} 
   //! assign tag type
   void Set_Tag_Type(const tag_type_s & arg) {tag_type = arg;}
   //! assign a name
   void Set_Name(const std::string & arg) {str_name = arg;}
   //! set termination flag
   void Set_Is_Termination(const bool arg) {b_termination = arg;}
   //!  set splitter flag
   void Set_Is_Splitter(const bool arg) {b_splitter = arg;}
   //! set escape mode
   void Set_Escape_Mode(const en_escape_mode arg) {escape_mode = arg;}
   //! shift tag by a signed offset
   void Shift(const ptrdiff_t i_offset);

   //! initialize this tag, or clear it completely
   void Reset();

   //! 'less than'  will automatically arrange all tags based on position,
   //when in a sorted set
   bool operator< (const tag_s & rhs) const;
   //! greater than op
   bool operator> (const tag_s & rhs) const;
   //! equals op
   bool operator== (const tag_s & rhs) const;
};
//---------------------------------------------------------------------------

//! unary predicate for counting tags of a certain type
class un_predicate_count_tag_type
{
 public:
   //! ctructor
   un_predicate_count_tag_type(const tag_type_s & arg);
   //! predicate op
   bool operator()(const tag_s & arg);

 private:
   //! tag type to compare to
   mutable tag_type_s tag_type;
};

//----------------------------------------------------------------------------
// block object
//----------------------------------------------------------------------------
//! block object ties related tag objects together
//! so an opening and closing IF tags will belong to one IF *block*
struct block_s
{
 public:
   //! default ctrcutor
   block_s();
   
   //! ctructor - assign the opening tag right away
   block_s(const tag_s & arg_open);

   //! get block name (name of opening tag)
   const std::string Get_Name() const {return tag_open.Get_Name();}
   //! get block escape mode (escape mode of opening tag)
   const en_escape_mode Get_Escape_Mode() const {return tag_open.Get_Escape_Mode();}
   //! block type
   const tag_type_s Get_Type() const {return block_type;}
   //! get open tag object
   const tag_s Get_Open_Tag() const {return tag_open;}
   //! get split tag object
   const tag_s Get_Split_Tag() const {return tag_split;}
   //! get close tag object
   const tag_s Get_Close_Tag() const {return tag_close;}
   //! see of the block has been deleted from block map
   bool Deleted() const {return b_deleted;}
   
   //! set open tag object
   void Set_Open_Tag(const tag_s & arg);
   //! set close tag object
   void Set_Close_Tag(const tag_s & arg) {tag_close = arg;}
   //! set split tag object
   void Set_Split_Tag(const tag_s & arg) {tag_split = arg;}
   //! delete block from block map

   void Delete() 
   {
#ifdef DEBUG
    std::cout << "Deleting block " << this->Get_Name() << std::endl;
#endif

    b_deleted = true;
    // make sure tags are not used to find this block again
    // (IMPORTANT, fixed source of an obscure bug)
    tag_open.Reset();
    tag_close.Reset();
    tag_split.Reset();
   }

   //! shift positions of all block tags
   void Shift(const ptrdiff_t i_offset, const size_t ui_start,
              const size_t ui_end);

   //! find out if this block has a split tag, such as ELSE
   bool Has_Split_Tag() const {return tag_split.Start() != std::string::npos;}

   //! see if this block contains another block
   bool Contains(const block_s & rhs) const;

   //! equals to tag op (block equals to tag if the opening tag is the same
   bool operator== (const tag_s & rhs) const;
   //! not equals to tag op
   bool operator!= (const tag_s & rhs) const;
   //! equals to block op
   bool operator== (const block_s & rhs) const;
   //!not equals to block op
   bool operator!= (const block_s & rhs) const;
   //! greater than another block op
   bool operator> (const block_s & rhs) const;

 private:
   //! tag type object
   tag_type_s block_type;
   //! open tag object
   tag_s tag_open;
   //! split tag object
   tag_s tag_split;
   //! close tag object
   tag_s tag_close;

   //! deleted block flag
   bool b_deleted;
};
//---------------------------------------------------------------------------

//! predicate for counting tag occurances in a block
class un_predicate_count_tag
{
 public:
   //! ctructor
   un_predicate_count_tag(const tag_s & arg);
   //! predicate op
   bool operator()(const block_s & arg);

 private:
   //! tag to compare the block to
   mutable tag_s tag;
};

//----------------------------------------------------------------------------
// html table
//---------------------------------------------------------------------------

//! table class forward declaration
class loop_s;

//! table cell object
struct row_cell_s
{
 //! string value for the cell
 std::string str_val;
 //! pointer to the nested table (if any)
 loop_s * p_table;

 //! ctructor
 row_cell_s();
 //! dstructor
 ~row_cell_s();
 //! copy ctructor
 row_cell_s(const row_cell_s & cpy);

 //! template for assigning all sorts of values to a cell, converting
 //everything to std::string
 template <class T>
 row_cell_s & operator= (const T & arg)
 {
  std::ostringstream buffer;
  buffer << arg;
  str_val = buffer.str(); 
  return *this; 
 }

 //! op to assign a nested table to a cell
 row_cell_s & operator= (const loop_s & arg);
};

//! alias typedef for a loop structure
typedef loop_s loop_t;

//---------------------------------------------------------------------------

//! table row object
struct row_s
{
 //! type of cell container
 typedef std::map <std::string, row_cell_s> cells_t;
 //! cell container
 cells_t cells_c;
   
 //! assign table cell op
 row_cell_s & operator() (const std::string & str_name);
   
 //! clear row contents
 void Clear() 
 {
  cells_c.clear();
 }
};

//! alias typedef for a row structure
typedef row_s row_t;

//---------------------------------------------------------------------------

//! table object
class loop_s
{
 public:
   //! type of rows conainer
   typedef std::list<row_s> rows_t;
 
 private:
   //! rows container
   rows_t rows_c;
 
 public:
   //! adds a row to the table
   loop_s & operator+= (const row_s & row)
   { 
    rows_c.push_back(row);
    return *this;
   }

   //! adds a row to the table
   loop_s operator+ (const row_s & row)
   { 
    return loop_s(*this) += row;
   }

   //! get rows container copy from the table object
   const rows_t & Get_Rows() const {return rows_c;}

   //! see if the loop is empty (has no rows)
   bool Empty() const;
};

//----------------------------------------------------------------------------
// variable object
//---------------------------------------------------------------------------

//! variable object
class cls_variable
{
 public:
   //! default ctructor
   cls_variable() {}
   //! ctructor wich assigns variable name
   cls_variable(const std::string & arg_var_name);
   //! dtructor
   virtual ~cls_variable() {}

   //! Assign any type of data, converting it to std::string
   /* Note that there
    * is no string data member that stores the value. The data is ultimately
    * stored in the first cell of the member table.
   */
   template <class T>
   cls_variable & operator= (const T & arg_var_val)
   {
    std::ostringstream buffer;
    buffer << arg_var_val;
    // create table row object
    row_s row;
    // store the string value as the cell with empty key
    row.cells_c[""] = buffer.str();
    // add the row to the member table
    table += row;
    // return the variable
    return *this;
   }

   //! assign a table to the variable instead
   cls_variable & operator= (const loop_s & arg_table);

   //! pull name of the variable
   const std::string Get_Name() const {return str_name;}

   //! get the value stored in this variable, assuming it's a single string
   // value
   const std::string Get_Val_String() const;
   //! get the value stored in this variable, assuming it's a table.
   const loop_s & Get_Val_Table() const;

 private:
   //! table that stores variable value. 
   /* It can either be a real table or a
    * string value, shich is stored in cell of the first row
    * under empty key
   */
   loop_s table;
   //! variable name - this the name that is expected in template body
   mutable std::string str_name;
};

//----------------------------------------------------------------------------
// template object
//---------------------------------------------------------------------------
//! template object
class html_template 
{
 //! type of tag type container
 typedef std::map <std::string, tag_type_s> tag_types_t;
 //! type of full tag string container 
 typedef std::map <std::string, unsigned short> tag_strings_t;
 //! type of variable container
 typedef std::map <std::string, cls_variable> variables_t;
 //! type of tag map container
 typedef std::set <tag_s> tag_map_t;
 //! type of block map container
 typedef std::vector <block_s> block_map_t;
 //! Type of line map container. 
 /* The key is the line number. The value is the
  * pair marking beginning and end position of the line, relative to the whole
  * template, starting from position 0.
 */
 typedef std::map < size_t, std::pair<size_t,size_t> > line_map_t;

 private:
   //! template file name
   std::string str_tmpl_file_name;
   //! template body as loaded from file (cannot be changed)
   mutable std::string str_tmpl_txt;
   //! copy of template body, for modifying
   std::string str_tmpl_txt_cpy;

   //! container of high-level tag types
   tag_types_t tag_types_c;

   //! container of tag types fully expanded as strings
   tag_strings_t tag_strings_c;

   //! reserved words
   tag_strings_t reserved_words_c;

   //! constant string that stores the prefix of a template tag
   mutable std::string tag_type_prefix;

   //! variables container
   variables_t variables_c;

   //! line map container
   line_map_t line_map;
   //! tag map container
   tag_map_t tag_map;
   //! block map container
   block_map_t block_map;

   //! initialize class constants (NOTE, it's not a RESET)
   void init();

   //! expand external template includes
   void expand_includes();
   //! builds line map
   void build_line_map();
   //! builds tag map
   void build_tag_map();
   //! builds block map, once tag map is available
   void build_block_map();

   //! debug dump of block map
   void print_block_map(block_map_t & r_block_map);

   //! debug dump of tag map (valid only before subsitution!)
   void print_tag_map();

   //! get line number based on absolute position. this is essential for debug
   //and syntax error messages
   const size_t get_line_from_pos(const size_t & ui_pos) const;
   
   //! find first encountered tag in template body
   tag_s find_tag() const;

   //! truncate spaces, relatives of spaces and anything resembling spaces, for easier parsing
   const std::string truncate(const std::string & arg) const;
   //! Parse, validate and return a standard tag name.
   // The internally used format is <TMPL_TYPE VALUE>
   const tag_s parse_tag( const std::string & arg ) const;

   //! Shift tag positions in the template. 
   /* This does not have to be done on the whole template. 
    * Changes may have to be done on local variables,
    * (loop scopes, for example), and that's why the function
    * is flexible in terms of source text and block maps it can work with.
    *  
    *  Input: 
    *    string to shift the tags in;
    *    block map of the string;
    *    signed offset to shift by;
    *    position to start shifting tags at;
    *    position to stop shifting tags at;
    */
   void shift_tags(
     const std::string & str_in,
     block_map_t & r_block_map,
     const ptrdiff_t i_offset,
     const size_t ui_start,
     size_t ui_end = std::string::npos
   );

   //! delete blocks between giver start and close tags
   void delete_blocks(
   	 const tag_s & open_tag, 
     const tag_s & close_tag,
     block_map_t & r_block_map
   ); 

   //! given block length (such as an IF block or a VAR block),
   //! and string to replace the block with
   //! figure out the offset to shift other tags by.
   const ptrdiff_t get_offset(
     const size_t ui_block_len, 
     const std::string & r_str
   ) const;

   //! evaluate a variable
   bool evaluate(cls_variable & var) const;

   //! process a repeating block
   void process_loop(
     const block_s & block,
     std::string & str_text,
     block_map_t & r_block_map,
     variables_t & r_variables_c
   );

   //! process simple varaibles
   void process_simple_vars(
     block_map_t & r_block_map, 
     std::string & str_text,
     variables_t & r_variables_c
   );

   //! process IF/UNLESS blocks
   void process_conditionals(
     block_map_t & r_block_map, 
     std::string & str_text,
     variables_t & r_variables_c
   );

   //! process loop blocks
   void process_loops(
     block_map_t & r_block_map, 
     std::string & str_text,
     variables_t & r_variables_c
   );
   
   //! process escape directives
   void escape_var( std::string & arg, const en_escape_mode );

   //! throw an exception (after adding more detail to it)
   void throw_exception( syntax_ex & ex ) const;
   //! throw exception overloaded for runtime exception class
   void throw_exception( runtime_ex & ex ) const;

 public:
   //! ctructor
   html_template();
   //! ctructor that loads the template
   html_template(const std::string & arg_file_name);
   //! dtructor
   virtual ~html_template();

   //! assign template file if none was given at first. This loads the file as
   //well
   void Set_Template_File(const std::string arg_file_name);

   //! assign a variable to the template
   cls_variable & operator() (const std::string arg_var_name); 

   //! process the template, return generated string
   const std::string & Process();

   //! print template output to stream
   friend std::ostream & operator << (std::ostream & out, html_template & obj)
   { 
    out << obj.Process();
    return out;
   }

   //! get tag count for each tag type
   const size_t Get_Tag_Type_Count(const tag_type_s & arg) const;
};

#endif

} // end namespace
