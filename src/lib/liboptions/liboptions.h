/*!
** \file
** \brief Header file for the options library 
** \ingroup LIBOPTIONS
**
** Francesco Evangelista 2008
*/

#ifndef _psi_src_lib_liboptions_liboptions_h
#define _psi_src_lib_liboptions_liboptions_h

#include <map>
#include <string>

void options_init();
void options_close();
void options_read();
void options_print();

void        options_add_bool(const char* cstr_option,bool bool_default);
void        options_add_int(const char* cstr_option,int int_default);
void        options_add_double(const char* cstr_option,double double_default);
void        options_add_str(const char* cstr_option,const char* cstr_default);
void        options_add_str_with_choices(const char* cstr_option,const char* cstr_default,const char* cstr_choices);

bool        options_get_bool(const char* cstr_option);
int         options_get_int(const char* cstr_option);
double      options_get_double(const char* cstr_option);
std::string options_get_str(const char* cstr_option);

typedef struct 
{
  bool option;
} BoolOption;

typedef struct 
{
  int option;
} IntOption;

typedef struct 
{
  double option;
} DoubleOption;

typedef struct 
{
  std::string option;
  std::string choices;
} StringOption;

typedef std::map<std::string,BoolOption>       BoolOptionsMap;
typedef std::map<std::string,IntOption>         IntOptionsMap;
typedef std::map<std::string,DoubleOption>   DoubleOptionsMap;
typedef std::map<std::string,StringOption>   StringOptionsMap;

class Options
{
public:
  Options();
  ~Options();

  void        print();
  void        read_options();

  void        add_bool_option(const char* cstr_option,bool bool_default);
  void        add_int_option(const char* cstr_option,int int_default);
  void        add_double_option(const char* cstr_option,double double_default);
  void        add_str_option(const char* cstr_option,const char* cstr_default);
  void        add_str_option_with_choices(const char* cstr_option,const char* cstr_default,const char* cstr_choices);

  bool        get_bool_option(const char* cstr_option);
  int         get_int_option(const char* cstr_option);
  double      get_double_option(const char* cstr_option);
  std::string get_str_option(const char* cstr_option);

  void        set_bool_option(const char* cstr_option, bool bool_value);
  void        set_int_option(const char* cstr_option,int int_value);
  void        set_double_option(const char* cstr_option,double double_value);
  void        set_str_option(const char* cstr_option,const char* cstr_value);
private:
  void read_bool(BoolOptionsMap::iterator& it);
  void read_int(IntOptionsMap::iterator& it);
  void read_double(DoubleOptionsMap::iterator& it);
  void read_string(StringOptionsMap::iterator& it);

  BoolOptionsMap      bool_options;
  IntOptionsMap       int_options;
  DoubleOptionsMap double_options;
  StringOptionsMap string_options;
};

extern Options *_default_psi_options_;

#endif // _psi_src_lib_liboptions_liboptions_h
