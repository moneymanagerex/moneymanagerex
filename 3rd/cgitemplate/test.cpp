#ifndef test_c
#define test_c
#include "test.h"
#endif

#ifdef WIN
#include <conio.h>
#endif

using namespace std;
using namespace tmpl;

html_template templ;
tests_t tests_c;

// convert a file.tmpl into a file.txt
string get_reference_file(const string str_file_name) {
    size_t dot_pos = str_file_name.rfind('.', str_file_name.length());
    string str_ret = "";

    if ( dot_pos != string::npos && dot_pos + 1 < str_file_name.length() - 1 ) {
        str_ret = str_file_name;
        str_ret.replace(dot_pos + 1, str_file_name.length() - dot_pos, "txt");
    }

    return str_ret;
}
//----------------------------------------------------------------------------

void test(const tests_t & tests_c, const string str_test_name) {
    cout << str_test_name << " ";

    for (unsigned int n=0; n < tests_c.size(); n++) {
        const test_s & t = tests_c[n];

        string str_template_output = "";

        try {
            templ.Set_Template_File( t.str_file );
            str_template_output = templ.Process();
        } catch (tmpl::syntax_ex & ex) {
            if ( t.str_error_to_catch == ex.what() ) {
                cout << ".";
                continue;
            } else { // NOT the type of error we expect, rethrow
                throw ex;
            }
        }


        // if there is an error to catch - we didn't catch it
        if ( t.str_error_to_catch.length() ) {
            throw( string("Uncaught error: '") + t.str_error_to_catch + "'" );
        }

        const string str_ref_file_path = get_reference_file( t.str_file );

        std::ifstream in_stream;
        in_stream.open(str_ref_file_path.c_str());

        if( !in_stream.is_open() ) {
            throw(string("Could not open reference test file '")
                  + str_ref_file_path
                  + "'. All tests aborted");
        }

        std::stringstream oss;
        oss << in_stream.rdbuf();
        string str_reference_output = oss.str();

        in_stream.close();

        const bool b_match = (str_template_output == str_reference_output);

        if (b_match == true) {
            cout << ".";
        } else {
            cout << " FAIL: '" << t.str_file << "'" << endl;
            cout << "Output left in 'fail.txt'" << endl;
            ofstream out_stream;
            out_stream.open("fail.txt", std::ios::out);
            out_stream.write(str_template_output.c_str(),
                             str_template_output.length());
            out_stream.close();

#ifdef WIN
            cout << "\nPress any key to QUIT";
            _getch();
#endif

            exit(1);
        }
    }

    cout <<" Pass" << endl;
}
//----------------------------------------------------------------------------

void run() {
    cout << "[ Testing ]" << endl;
    tests_t simple;
    simple.push_back( test_s("t/simple.tmpl") );
    simple.push_back( test_s("t/no_name.tmpl") );
    simple.push_back( test_s("t/simple2.tmpl") );
    simple.push_back( test_s("t/malformed_tag1.tmpl") );
    simple.push_back( test_s("t/malformed_tag3.tmpl") );
    simple.push_back( test_s("t/malformed_tag4.tmpl") );
    simple.push_back( test_s("t/malformed_tag5.tmpl") );
    simple.push_back( test_s("t/name_var.tmpl") );
    test(simple, "Substitution");

    tests_t cond;
    cond.push_back( test_s("t/if.tmpl") );
    cond.push_back( test_s("t/if2.tmpl") );
    cond.push_back( test_s("t/if3.tmpl") );
    cond.push_back( test_s("t/if4.tmpl") );
    cond.push_back( test_s("t/if_else.tmpl") );
    cond.push_back( test_s("t/if_else2.tmpl") );
    cond.push_back( test_s("t/if_else3.tmpl") );
    cond.push_back( test_s("t/if_and_vars.tmpl") );
    cond.push_back( test_s("t/unless.tmpl") );
    cond.push_back( test_s("t/unless_else.tmpl") );
    test(cond, "Conditional");

    tests_t loops;
    loops.push_back( test_s("t/loop.tmpl") );
    loops.push_back( test_s("t/loop_globals.tmpl") );
    loops.push_back( test_s("t/empty_loop.tmpl") );
    loops.push_back (test_s("t/loop_with_if.tmpl") );
    loops.push_back (test_s("t/nested_loop.tmpl") );
    loops.push_back (test_s("t/context_vars.tmpl") );
    test(loops, "Loop");

    tests_t include;
    include.push_back( test_s("t/includer.tmpl") );
    include.push_back( test_s("t/includer2.tmpl") );
    test(include, "Includes");

    tests_t encodings;
    encodings.push_back( test_s("t/escape.tmpl") );
    test(encodings, "Encodings");

    tests_t exceptions;
    exceptions.push_back( test_s("t/overlapping_ifs.tmpl", "Block not properly terminated") );
    exceptions.push_back( test_s("t/open_loop.tmpl", "Block not terminated") );
    exceptions.push_back( test_s("t/malformed_tag2.tmpl", "Malformed tag") );
    test(exceptions, "Exceptions");

    tests_t all;
    all.push_back( test_s("t/all.tmpl") );
    test(all, "Combined");
}
//----------------------------------------------------------------------------

void create(const string str_tmpl_file = "") {
    // if we are trying to create a single reference file, overwrite the global
    // test list
    if ( str_tmpl_file.length() ) {
        tests_c.clear();
        tests_c.push_back( test_s(str_tmpl_file) );
    }

    cout << "\nCreating " << tests_c.size() << " reference tests...\n" << endl;

    for (unsigned int n=0; n < tests_c.size(); n++) {
        test_s & r_test = tests_c[n];
        templ.Set_Template_File( r_test.str_file );

        const string str_out_file = get_reference_file( r_test.str_file );

        ofstream out_stream;
        out_stream.open(str_out_file.c_str(), std::ios::out);

        const string str_reference_output = templ.Process();

        out_stream.write(str_reference_output.c_str(),
                         str_reference_output.length());
        out_stream.close();
        cout << r_test.str_file << endl;
    }
}
//----------------------------------------------------------------------------

int main(int argc, char* argv[]) {
    try {
        tmpl::row_t row;
        tmpl::loop_t table;
        tmpl::row_t row2;
        tmpl::loop_t table2;
        tmpl::row_t row3;
        tmpl::loop_t table3;

        tmpl::loop_t empty_table;

        row2("table_2_column_1") = "TABLE 2 ROW 1 COLUMN 1";
        row2("table_2_column_2") = "TABLE 2 ROW 1 COLUMN 2";
        row2("table_2_column_3") = "I GOT THE WHOLE WOLRD IN MY HAND... I GOT THE WHOLE WIDE WORLD, IN MY HAND";
        table2 += row2;
        row2("table_2_column_1") = "TABLE 2 ROW 2 COLUMN 1";
        row2("table_2_column_2") = "TABLE 2 ROW 2 COLUMN 2";
        row2("table_2_column_3") = "I GOT THE WHOLE WOLRD IN MY HAND... I GOT THE WHOLE WIDE WORLD, IN MY HAND";
        table2 += row2;

        row3("table_3_column_1") = "TABLE 3 ROW 1 COLUMN 1";
        table3 += row3;

        row("table_1_column_1") = "TABLE 1 ROW 1 COLUMN 1";
        row("table_1_column_2") = "TABLE 1 ROW 1 COLUMN 2";
        row("table_1_column_3") = "I GOT THE WHOLE WOLRD IN MY HAND... I GOT THE WHOLE WIDE WORLD, IN MY HAND";
        row("LOOP2") = table2;
        row("LOOP3") = table3;
        table += row;
        row("table_1_column_1") = "TABLE 1 ROW 2 COLUMN 1";
        row("table_1_column_2") = "TABLE 1 ROW 2 COLUMN 2";
        row("table_1_column_3") = "I GOT THE WHOLE WOLRD IN MY HAND... I GOT THE WHOLE WIDE WORLD, IN MY HAND";
        row("LOOP2") = table2;
        row("LOOP3") = table3;
        table += row;
        row("table_1_column_1") = "TABLE 1 ROW 3 COLUMN 1";
        row("table_1_column_2") = "TABLE 1 ROW 3 COLUMN 2";
        row("table_1_column_3") = "I GOT THE WHOLE WOLRD IN MY HAND... I GOT THE WHOLE WIDE WORLD, IN MY HAND";
        row("LOOP2") = table2;
        row("LOOP3") = table3;
        table += row;

        templ("VAR")     = "VARIABLE 1";
        templ("VAR2")    = "VERY VERY VERY LONG VARIABLE 2";
        templ("CHAR")    = "V";
        templ("INT")     = 123456789;
        templ("NEG_INT") = -123456789;
        templ("VAR_DBL") = 1234.5678;
        templ("EMPTY1")  = 0;
        templ("EMPTY2")  = "";
        templ("EMPTY3")  = "  ";
        templ("EMPTY4")  = "0000 ";
        templ("NEMPTY1") = " 000.00";
        templ("LOOP1")   = table;
        templ("EMPTY_LOOP") = empty_table;
        templ("TEXT")       = "Cheney was of military age and a supporter \
of the Vietnam War but he did not serve in the war, applying for and \
receiving five draft deferments. On May 19, 1965, Cheney was classified \
as 1-A , \"available for service\" by the Selective Service. On October 26, \
1965 the Selective Service lifted the constraints on drafting childless \
married men. Cheney and his wife then had a child after which he applied \
for and received, a reclassification of 3-A, gaining him a final draft \
deferment.\n\nIn an interview with George C. Wilson that appeared in the \
April 5, 1989 issue of The Washington Post, when asked about his deferments \
the future Defense Secretary said, \"I had other priorities in the '60s than \
military service.\"" ;
        templ("RESULT_SET") = "";
        templ("SEARCH_PARAM") = "Jessica Simpson";
        templ("SEARCH_PARAM2") = "get a life";
        templ("NAME") = "NAME";
        templ("ENCODED") = "Tom & Jerry is \"funny\"";
        templ("ENCODED2") = "\\ ' \" \n";
        templ("ENCODED3") = "Dolce & Gabbana";

        // for testing global var overrides
        templ("table_1_column_1") = "I_MUST_BE_OVERWRITTEN_IN_LOOP";

        if (argc == 2 && strcmp(argv[1], "create") == 0) {
            create();
        }
        // if creating a single test
        else if (argc == 3 && strcmp(argv[1], "create") == 0) {
            string str_tmpl_file = argv[2];
            create( str_tmpl_file );
        }
        // if running a single test
        else if (argc == 3 && strcmp(argv[1], "run") == 0) {
            string str_tmpl_file = argv[2];
            tests_t single_test;
            single_test.push_back( test_s(str_tmpl_file) );
            test(single_test, str_tmpl_file);
        } else {
            run();
        }
    } catch (tmpl::runtime_ex & ex) {
        cout << "\nCGI TEMPLATE ERROR: " << ex.what() << "\n";
        cout << "CGI TEMPLATE FILE: " << ex.template_path << "\n";
    } catch (tmpl::syntax_ex & ex) {
        cout << "\nSYNTAX ERROR: " << ex.what() << "\n";
        cout << "LINE: " << ex.line << "\n";
        cout << "DETAIL: " << ex.detail << "\n";
        cout << "PATH: " << ex.template_path << "\n";
    } catch (std::exception & ex) {
        cout << "\nEXCEPTION: " << ex.what() << "\n";
    } catch (string & str) {
        cout << "\nERROR: " << str << "\n";
    }

#ifdef WIN
    cout << "\nPress any key to QUIT";
    _getch();
#endif

    return 0;
}

