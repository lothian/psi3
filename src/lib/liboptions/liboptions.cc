/*!
  \file
  \brief Implementation of the options class and corresponding C-style interface
  \defgroup LIBOPTIONS liboptions: A library to read user options
  \ingroup LIBOPTIONS
*/

#include <vector>
#include <cstdlib>
#include <cstring>
#include <algorithm>

#include <libipv1/ip_lib.h>

#include "liboptions.h"

extern FILE* outfile;

Options* _default_psi_options_;

/*!
 * Initialize the options interface. Must be called before using the C-style functions. 
 */
void options_init()
{
  _default_psi_options_ = new Options();
}

/*!
 * Close the options interface. Must be called before the end of the program when using the C-style interface. 
 */
void options_close()
{
  delete _default_psi_options_;
}

/*!
 * Print the options. 
 */
void options_print(){
  return(_default_psi_options_->print());    
}

/*!
 * Add a boolean option.
 * @param cstr_option a string containing the parameter name
 * @param bool_default the default value for this option
 */
void options_add_bool(char* cstr_option,bool bool_default)
{
  _default_psi_options_->add_bool_option(cstr_option,bool_default);
}

/*!
 * Add an integer option.
 * @param cstr_option a string containing the parameter name
 * @param int_default the default value for this option
 */
void options_add_int(char* cstr_option,int int_default)
{
  _default_psi_options_->add_int_option(cstr_option,int_default);
}

/*!
 * Add a double option.
 * @param cstr_option a string containing the parameter name
 * @param double_default the default value for this option
 */
void options_add_double(char* cstr_option,double double_default)
{
  _default_psi_options_->add_double_option(cstr_option,double_default);
}

/*!
 * Add a string option.
 * @param cstr_option a string containing the parameter name
 * @param cstr_default the default value for this option
 */
void options_add_str(char* cstr_option,char* cstr_default)
{
  _default_psi_options_->add_str_option(cstr_option,cstr_default);
}

/*!
 * Add a string option with a restricted number of choices.
 * @param cstr_option a string containing the parameter name
 * @param cstr_default the default value for this option
 * @param cstr_choices a list of choices separated by a space
 */
void options_add_str_with_choices(char* cstr_option,char* cstr_default,char* cstr_choices)
{
  _default_psi_options_->add_str_option_with_choices(cstr_option,cstr_default,cstr_choices);
}

/*!
 * Get the value of a boolean option
 */
bool options_get_bool(char* cstr_option){
  return(_default_psi_options_->get_bool_option(cstr_option));    
}

/*!
 * Get the value of an integer option
 */
int options_get_int(char* cstr_option){
  return(_default_psi_options_->get_int_option(cstr_option));    
}

// double options_get_double(char* cstr_option){
//   return(_default_psi_options_->get_double_option(cstr_option));    
// }

/*!
 * Get the value of a string option
 */
std::string options_get_str(char* cstr_option){
  return(_default_psi_options_->get_str_option(cstr_option));    
}

// Auxiliary functions
std::vector<std::string> split(const std::string& str);
bool space(char c);
bool not_space(char c);

using namespace std;

Options::Options()
{
}

Options::~Options()
{
}

void Options::add_bool_option(char* cstr_option,bool bool_default)
{
  string str_option(cstr_option);
  // Make sure that the option that we are adding is not already present
  BoolOptionsMap::iterator it = bool_options.find(str_option);
  if(it==bool_options.end()){
    bool_options[str_option];
    bool_options[str_option].option  = bool_default;
  }else{
    fprintf(outfile,"\n  Options: add_bool_option(%s), option %s already declared",cstr_option,cstr_option);
  }
}

void Options::add_int_option(char* cstr_option,int int_default)
{
  string str_option(cstr_option);
  // Make sure that the option that we are adding is not already present
  IntOptionsMap::iterator it = int_options.find(str_option);
  if(it==int_options.end()){
    int_options[str_option];
    int_options[str_option].option  = int_default;
  }else{
    fprintf(outfile,"\n  Options: add_bool_option(%s), option %s already declared",cstr_option,cstr_option);
  }
}

void Options::add_double_option(char* cstr_option,double double_default)
{
  string str_option(cstr_option);
  // Make sure that the option that we are adding is not already present
  DoubleOptionsMap::iterator it = double_options.find(str_option);
  if(it==double_options.end()){
    double_options[str_option];
    double_options[str_option].option  = double_default;
  }else{
    fprintf(outfile,"\n  Options: add_bool_option(%s), option %s already declared",cstr_option,cstr_option);
  }
}


void Options::add_str_option(char* cstr_option,char* cstr_default)
{
  add_str_option_with_choices(cstr_option,cstr_default,"");
}

void Options::add_str_option_with_choices(char* cstr_option,char* cstr_default,char* cstr_choices)
{
  string str_option(cstr_option);
  string str_default(cstr_default);
  string str_choices(cstr_choices);
  // Make sure that the option that we are adding is not already present
  StringOptionsMap::iterator it = string_options.find(str_option);
  if(it==string_options.end()){
    string_options[str_option];
    string_options[str_option].option  = str_default;
    string_options[str_option].choices = str_choices;
  }else{
    fprintf(outfile,"\n  Options: add_str_option(%s), option %s already declared",cstr_option,cstr_option);
  }
}

bool Options::get_bool_option(char* cstr_option)
{
  string str_option(cstr_option);
  // Make sure that the option that we are adding is available
  BoolOptionsMap::iterator it = bool_options.find(str_option);
  if(it!=bool_options.end()){
    return(bool_options[str_option].option);
  }else{
    fprintf(outfile,"\n  Options: get_str_option(%s), option %s is not available",cstr_option,cstr_option);
    fflush(outfile);
    abort();
    return(false);
  }
}

int Options::get_int_option(char* cstr_option)
{
  string str_option(cstr_option);
  // Make sure that the option that we are adding is available
  IntOptionsMap::iterator it = int_options.find(str_option);
  if(it!=int_options.end()){
    return(int_options[str_option].option);
  }else{
    fprintf(outfile,"\n  Options: get_str_option(%s), option %s is not available",cstr_option,cstr_option);
    fflush(outfile);
    abort();
    return(0);
  }
}

std::string Options::get_str_option(char* cstr_option)
{
  string str_option(cstr_option);
  // Make sure that the option that we are adding is available
  StringOptionsMap::iterator it = string_options.find(str_option);
  if(it!=string_options.end()){
    return(string_options[str_option].option);
  }else{
    fprintf(outfile,"\n  Options: get_str_option(%s), option %s is not available",cstr_option,cstr_option);
    fflush(outfile);
    abort();
    return("NA");
  }
}

void Options::set_bool_option(char* cstr_option, bool bool_value)
{
  string str_option(cstr_option);
  bool_options[str_option].option  = bool_value;
}

void Options::set_int_option(char* cstr_option, int int_value)
{
  string str_option(cstr_option);
  int_options[str_option].option  = int_value;
}

void Options::set_double_option(char* cstr_option, double double_value)
{
  string str_option(cstr_option);
  double_options[str_option].option  = double_value;
}

void Options::set_str_option(char* cstr_option, char* cstr_value)
{
  string str_option(cstr_option);
  string_options[str_option].option  = string(cstr_value);
}

void Options::read_options()
{
  // Bool
  for(BoolOptionsMap::iterator it = bool_options.begin();it != bool_options.end();++it){
    read_bool(it);
  }
  // Int
  for(IntOptionsMap::iterator it = int_options.begin();it != int_options.end();++it){
    read_int(it);
  }
  // Int
  for(DoubleOptionsMap::iterator it = double_options.begin();it != double_options.end();++it){
    read_double(it);
  }
  // String Options
  for(StringOptionsMap::iterator it = string_options.begin();it != string_options.end();++it){
    read_string(it);
  }

}

void Options::read_bool(BoolOptionsMap::iterator& it)
{
  int int_value;
  char* cstr_label = new char [it->first.length () + 1];
  memset (cstr_label, 0x00, it->first.length () + 1);
  strcpy (cstr_label, it->first.c_str ());

  int status = ip_boolean(cstr_label,&int_value,0);
  if (status == IPE_OK){
    it->second.option = bool(int_value);
  }
  delete[] cstr_label;
}

void Options::read_int(IntOptionsMap::iterator& it)
{
  int int_value;
  char* cstr_label = new char [it->first.length () + 1];
  memset (cstr_label, 0x00, it->first.length () + 1);
  strcpy (cstr_label, it->first.c_str ());

  int status = ip_data(cstr_label,"%d",&int_value,0);
  if (status == IPE_OK){
    it->second.option = int_value;
  }
  delete[] cstr_label;
}

void Options::read_double(DoubleOptionsMap::iterator& it)
{
  double double_value;
  char* cstr_label = new char [it->first.length () + 1];
  memset (cstr_label, 0x00, it->first.length () + 1);
  strcpy (cstr_label, it->first.c_str ());

  int status = ip_data(cstr_label,"%f",&double_value,0);
  if (status == IPE_OK){
    it->second.option = double_value;
  }
  delete[] cstr_label;
}


void Options::read_string(StringOptionsMap::iterator& it)
{
  char* cstr_value = NULL;
  char* cstr_label = new char [it->first.length () + 1];
  memset (cstr_label, 0x00, it->first.length () + 1);
  strcpy (cstr_label, it->first.c_str ());

  int status = ip_string(cstr_label,&cstr_value,0);
  if (status == IPE_OK){
    it->second.option = string(cstr_value);
    // Check if there are restricted choices
    if(it->second.choices.size()>0){
      bool wrong_input = true;
      vector<string> choices = split(it->second.choices);
      for(int i=0;i<choices.size();++i){
        if(it->second.option==choices[i])
          wrong_input = false;
      }
      if(wrong_input){
        fprintf(outfile,"\n\n  Option %s has the wrong value (%s).\n  Possible choices are\n    %s\n",it->first.c_str(),it->second.option.c_str(),it->second.choices.c_str());
        fflush(outfile);
        abort();
      }
    }
    if(cstr_value!=NULL)
      free(cstr_value);
  }
  delete[] cstr_label;
}

void Options::print()
{
  fprintf(outfile,"\n\n  Options:");
  fprintf(outfile,"\n  ------------------------------------------------------------------------------");
  for(BoolOptionsMap::iterator it = bool_options.begin();it != bool_options.end();++it){
    fprintf(outfile,"\n  %-35s = %s",it->first.c_str(),it->second.option ? "TRUE" : "FALSE" );
  }
  for(IntOptionsMap::iterator it = int_options.begin();it != int_options.end();++it){
    fprintf(outfile,"\n  %-35s = %-d",it->first.c_str(),it->second.option);
  }
  for(DoubleOptionsMap::iterator it = double_options.begin();it != double_options.end();++it){
    fprintf(outfile,"\n  %-35s = %-d",it->first.c_str(),it->second.option);
  }
  for(StringOptionsMap::iterator it = string_options.begin();it != string_options.end();++it){
    fprintf(outfile,"\n  %-35s = %-35s",it->first.c_str(),it->second.option.c_str());
  }
//   int bool_printed = 0;
//   for(BoolOptionsMap::iterator it = bool_options.begin();it != bool_options.end();++it){
//     if(bool_printed % 2 ==0)
//       fprintf(outfile,"\n  %-12s = %-21s",it->first.c_str(),it->second.option ? "TRUE" : "FALSE" );
//     else
//       fprintf(outfile," %-12s = %-21s",it->first.c_str(),it->second.option ? "TRUE" : "FALSE" );
//     bool_printed++;
//   }
// 
//   int int_printed = 0;
//   for(IntOptionsMap::iterator it = int_options.begin();it != int_options.end();++it){
//     if(int_printed % 2 ==0)
//       fprintf(outfile,"\n  %-12s = %-21d",it->first.c_str(),it->second.option);
//     else
//       fprintf(outfile," %-12s = %-21d",it->first.c_str(),it->second.option);
//   }
// 
//   int double_printed = 0;
//   for(DoubleOptionsMap::iterator it = double_options.begin();it != double_options.end();++it){
//     if(double_printed % 2 ==0)
//       fprintf(outfile,"\n  %-12s = %-21f",it->first.c_str(),it->second.option);
//     else
//       fprintf(outfile," %-12s = %-21f",it->first.c_str(),it->second.option);
//   }
// 
//   int str_printed = 0;
//   for(StringOptionsMap::iterator it = string_options.begin();it != string_options.end();++it){
//     if(str_printed % 2 ==0)
//       fprintf(outfile,"\n  %-12s = %-21s",it->first.c_str(),it->second.option.c_str());
//     else
//       fprintf(outfile," %-12s = %-21s",it->first.c_str(),it->second.option.c_str());
//     str_printed++;
//   }
}

std::vector<std::string> split(const std::string& str)
{
  // Split a string
  typedef string::const_iterator iter;
  std::vector<std::string> splitted_string;
  iter i = str.begin();
  while(i != str.end()){
    // Ignore leading blanks
    i = find_if(i,str.end(), not_space);
    // Find the end of next word
    iter j = find_if(i,str.end(),space);
    // Copy the characters in [i,j)
    if(i!=str.end())
      splitted_string.push_back(string(i,j));
    i = j;
  }
  return(splitted_string);
}

bool space(char c)
{
  return isspace(c);
}

bool not_space(char c)
{
  return !isspace(c);
}