#include "../html_template.h"

using namespace std;
using namespace tmpl;

//----------------------------------------------------------------------------

int main(int argc, char* argv[]) 
{
 // create a template object
 html_template templ("example.tmpl");

 // assign a value to variable VAR1
 templ("VAR1") = "I am your very first template variable.";

 // create a loop - does not have to be an std::map, we are just using this for
 // clarity

 map<string, int> scores; // student names and scores on a test
 scores["Aagney"] = 45;
 scores["Eileen"] = 83;
 scores["Basil" ] = 91;
 scores["Jagati"] = 93;
 scores["Raine" ] = 69;

 map<string, int>::const_iterator itr = scores.begin();
 loop_t loop; // the loop variable
  
   for( ; itr != scores.end(); ++itr )
   {
    const string & name  = itr->first;
    const int    & score = itr->second;

    // create a row
    row_t row;
    
    // populate columns
    row("name")  = name;
    row("score") = score; // can be ANY basic data type, this one is an integer
    
    // add the row to the loop, and so on....
    loop += row;
   }
   
 // assign the loop to the template (case-insensitive)
 templ("SCORES") = loop;

 cout << templ;
 return 0;
}

