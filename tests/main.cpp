#define DOCTEST_CONFIG_IMPLEMENT
#include <clocale>
#include <iostream>
#include <doctest/doctest.h>

int main(int argc, const char **argv){                                          
  if(!setlocale(LC_ALL, "")){                                                   
    std::cerr << "Coudln't set locale based on user preferences!" << std::endl; 
    return EXIT_FAILURE;
  }                                                                             
  doctest::Context context;                                                     
                                                                                  // defaults                                                                   
  context.setOption("order-by", "name");            // sort the test cases by their name                                                                        
                                                                                
  context.applyCommandLine(argc, argv);                                         
                                                                                
  // overrides                                                                  
  context.setOption("no-breaks", true);             // don't break in the debugger when assertions fail                                                         
                                                                                
  int res = context.run(); // run

  if(context.shouldExit()){ // important - query flags (and --exit) rely on the user doing this
    return res;             // propagate the result of the tests
  }

  int client_stuff_return_code = 0;
  // your program - if the testing framework is integrated in your production code

  return res + client_stuff_return_code; // the result from doctest is propagated here as well
}
