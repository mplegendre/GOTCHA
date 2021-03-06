/*
This file is part of GOTCHA.  For copyright information see the COPYRIGHT
file in the top level directory, or at
https://github.com/LLNL/gotcha/blob/master/COPYRIGHT
This program is free software; you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License (as published by the Free
Software Foundation) version 2.1 dated February 1999.  This program is
distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
without even the IMPLIED WARRANTY OF MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE. See the terms and conditions of the GNU Lesser General Public License
for more details.  You should have received a copy of the GNU Lesser General
Public License along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*/

#include <gotcha/gotcha.h>
#include <link.h>
#include <check.h>
#include "testing_lib.h"
//#include <gotcha_utils.h>
//#include "elf_ops.h"
//#include "tool.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////GOTCHA Core Tests///////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct tool* new_tool;
void setup_infrastructure(){
  new_tool = create_tool("internal_test_tool");
}
void teardown_infrastructure(){
  free(new_tool);
}

int dummy_main(int argc, char* argv[]){return 4;}
START_TEST(symbol_prep_test)
{
  int(*my_main)(int argc, char* argv[]) = 0;
  struct gotcha_binding_t bindings[] = {
    { "main", &dummy_main, &my_main  }
  };
  struct binding_t* internal_bindings = add_binding_to_tool(new_tool, bindings, 1);
  gotcha_prepare_symbols(internal_bindings,1);
  ck_assert_msg((my_main!=0), "gotcha_prepare_symbols was not capable of finding function main");
}
END_TEST

int(*orig_func)();
int wrap_sample_func(){
  return orig_func()+5;
}

START_TEST(auto_tool_creation){
  int(*my_main)(int argc, char* argv[]) = 0;
  struct gotcha_binding_t bindings[] = {
    { "main", &dummy_main, &my_main  }
  };
  gotcha_wrap(bindings,1,"sample_autocreate_tool");
  ck_assert_msg(1,"Should never fail unless segfault on tool creation");
}
END_TEST

START_TEST(symbol_wrap_test){
  struct gotcha_binding_t bindings[] = {
    { "simpleFunc", &wrap_sample_func, &orig_func }
  };
  gotcha_wrap(bindings,1,"internal_test_tool");
  int x = simpleFunc(); 
  ck_assert_msg((x!=TESTING_LIB_RET_VAL),"gotcha_wrap did not redirect a call to the wrapper function");
}
END_TEST

START_TEST(bad_lookup_test){
  struct gotcha_binding_t bindings[] = {
    { "this_is_the_story_of_a_function_we_shouldnt_find", &wrap_sample_func, &orig_func }
  };
  enum gotcha_error_t errcode = gotcha_wrap(bindings,1,"internal_test_tool");
  ck_assert_msg((errcode==GOTCHA_FUNCTION_NOT_FOUND),"Looked up a function that shouldn't be found and did not get correct error code");
}
END_TEST
Suite* gotcha_core_suite(){
  Suite* s = suite_create("Gotcha Core");
  TCase* core_case = tcase_create("Wrapping");
  tcase_add_checked_fixture(core_case, setup_infrastructure, teardown_infrastructure);
  tcase_add_test(core_case, symbol_prep_test);
  tcase_add_test(core_case, symbol_wrap_test);
  tcase_add_test(core_case, bad_lookup_test);
  tcase_add_test(core_case, auto_tool_creation);
  suite_add_tcase(s, core_case);
  return s;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////Libc Wrapper Tests//////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// TODO: this test has no way to fail except segfaults. Need to ensure it does sane things
START_TEST(debug_print_test){
  struct link_map* print_me = _r_debug.r_map;
  debug_print(print_me,"");
}
END_TEST

START_TEST(gotcha_malloc_test){
  int* x = (int*)gotcha_malloc(sizeof(int)*10);
  x[9] = 5;
}
END_TEST

START_TEST(gotcha_free_test){
  int* x = (int*)gotcha_malloc(sizeof(int)*10);
  gotcha_free(x);

}
END_TEST

START_TEST(gotcha_realloc_test){
  int* x = (int*)gotcha_malloc(sizeof(int)*10);
  x[9] = 5;
  x = (int*)gotcha_realloc(x,sizeof(int)*5);
  x[4] = 5;
  x = (int*)gotcha_realloc(x,sizeof(int)*15);
  x[14] = 5;
}
END_TEST

START_TEST(gotcha_memcpy_test){
  int* x = (int*)gotcha_malloc(sizeof(int)*10);
  int* y = (int*)gotcha_malloc(sizeof(int)*10);
  int i =0;
  for(i =0 ;i<10;i++){
    x[i] = i;
    y[i] = 999;
  }
  gotcha_memcpy(y,x,sizeof(int)*10);
  for(i =0 ;i<10;i++){
    ck_assert_msg(y[i] == i, "Target for gotcha_memcpy doesn't have the correct data");
  }
}
END_TEST

START_TEST(gotcha_strcmp_test){
  ck_assert_msg(gotcha_strncmp("dogsaregood","dogsisgreat",4)==0, "gotcha_strncmp is examining too many characters, marking matching prefixes as not matching");
  ck_assert_msg(gotcha_strncmp("dogsaregood","dogsisgreat",5)!=0, "gotcha_strncmp is examining too few characters, marking nonmatching prefixes as matching");
  ck_assert_msg(gotcha_strncmp("dogs","pups",999)!=0, "gotcha_strncmp fails on nonmatching strings of smaller lengths than the declared string length");
  ck_assert_msg(gotcha_strncmp("dogs","dogs",999)==0, "gotcha_strncmp fails on matching strings of smaller lengths than the declared string length");
  ck_assert_msg(gotcha_strcmp("dogs","pups")!=0, "gotcha_strcmp fails on nonmatching strings");
  ck_assert_msg(gotcha_strcmp("dogs","dogs")==0, "gotcha_strcmp fails on matching strings");
}
END_TEST

Suite* gotcha_libc_suite(){
  Suite* s = suite_create("Gotcha Libc");
  TCase* libc_case = tcase_create("Basic tests");
  tcase_add_test(libc_case, debug_print_test);
  tcase_add_test(libc_case, gotcha_malloc_test);
  tcase_add_test(libc_case, gotcha_free_test);
  tcase_add_test(libc_case, gotcha_realloc_test);
  tcase_add_test(libc_case, gotcha_memcpy_test);
  tcase_add_test(libc_case, gotcha_strcmp_test);
  suite_add_tcase(s, libc_case);
  return s;
}


int main(){
  int num_fails;
  Suite* core_suite = gotcha_core_suite();
  SRunner* core_runner = srunner_create(core_suite);
  srunner_run_all(core_runner, CK_NORMAL);
  num_fails = srunner_ntests_failed(core_runner);
  Suite* libc_suite = gotcha_libc_suite();
  SRunner* libc_runner = srunner_create(libc_suite);
  srunner_run_all(libc_runner, CK_NORMAL);
  num_fails += srunner_ntests_failed(libc_runner);
  srunner_free(core_runner);
  srunner_free(libc_runner);
  return num_fails;
}
