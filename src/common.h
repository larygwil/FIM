/* $LastChangedDate: 2017-10-01 00:56:04 +0200 (Sun, 01 Oct 2017) $ */
/*
 common.h : Miscellaneous stuff header file

 (c) 2007-2017 Michele Martone

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
*/
#ifndef FIM_COMMON_H
#define FIM_COMMON_H
#undef yyFlexLexer
#include <FlexLexer.h>
#include "fim_types.h"
extern int fim_pipedesc[2];

extern FlexLexer *lexer;
//#define YYLEX lexer->yylex()
namespace fim
{
	class CommandConsole;
	class string;
}

int fim_isspace(int c);
int fim_isquote(int c);

fim::string fim_dirname(const fim::string& arg);
fim::string fim_shell_arg_escape(const fim::string& arg, bool quoted=true);
fim::string fim_key_escape(const fim::string uk);
void fim_perror(const fim_char_t *s);
//void fim_tolowers(fim_char_t *s);
//void fim_touppers(fim_char_t *s);
size_t fim_strlen(const fim_char_t *str);
void trec(fim_char_t *str,const fim_char_t *f,const fim_char_t*t);
//void trhex(fim_char_t *str);
void chomp(fim_char_t *s);
void sanitize_string_from_nongraph(fim_char_t *s, int c=0);
void sanitize_string_from_nongraph_except_newline(fim_char_t *s, int c=0);

using namespace fim;

int int2msbf(int in);
int int2lsbf(int in);
fim::string slurp_file(fim::string filename);
fim_char_t* slurp_binary_fd(int fd,int *rs);
fim_byte_t* slurp_binary_FD(FILE* fd, size_t  *rs);
bool write_to_file(fim::string filename, fim::string lines, bool append = false);

fim_char_t * dupstr (const fim_char_t* s);
fim_char_t * dupnstr (float n, const fim_char_t c='\0');
fim_char_t * dupnstr (const fim_char_t c1, double n, const fim_char_t c2='\0');
fim_char_t * dupnstr (fim_int n);
fim_char_t * dupsqstr (const fim_char_t* s);
fim_int fim_rand(void);

bool regexp_match(const fim_char_t*s, const fim_char_t*r, int ignorecase=1, int ignorenewlines=0);

int strchr_count(const fim_char_t*s, int c);
int lines_count(const fim_char_t*s, int cols);
int newlines_count(const fim_char_t*s);
const fim_char_t* next_row(const fim_char_t*s, int cols);
int fim_common_test(void);

double getmilliseconds(void);
const fim_char_t * fim_getenv(const fim_char_t * name);
FILE * fim_fread_tmpfile(FILE * fp);
double fim_atof(const fim_char_t *nptr);
ssize_t fim_getline(fim_char_t **lineptr, size_t *n, FILE *stream, int delim);

bool is_dir(const fim::string nf);
bool is_file(const fim::string nf);
bool is_file_nonempty(const fim::string nf);
FILE *fim_fopen(const char *path, const char *mode);
int fim_fclose(FILE*fp);
size_t fim_fread(void *ptr, size_t size, size_t nmemb, FILE *stream);
int fim_rewind(FILE *stream);
int fim_fseek(FILE *stream, long offset, int whence);
int fim_fgetc(FILE *stream);
int fim_snprintf_fim_int(char *r, fim_int n);
int fim_snprintf_XB(char *str, size_t size, size_t q);
fim_byte_t * fim_pm_alloc(unsigned int width, unsigned int height, bool want_calloc = false);
const fim_char_t * fim_basename_of(const fim_char_t * s);
fim::string fim_getcwd(void);
fim_int fim_atoi(const char*s);
size_t fim_maxrss(void);
fim_bool_t fim_is_id(const char*s);
fim_bool_t fim_is_id_char(const char c);

/* exceptions */
typedef int FimException;
#define FIM_E_NO_IMAGE 1
#define FIM_E_NO_VIEWPORT 2
#define FIM_E_WINDOW_ERROR 3
#define FIM_E_TRAGIC -1	/* no hope */
#define FIM_E_NO_MEM 4	/* also a return code */
/* ... */

#define FIM_CHAR_BIT 8 /* FIXME */
#define FIM_IS_SIGNED(T)   (((T)0) > (((T)-1)))
#define FIM_MAX_UNSIGNED(T) ((T)-1)
#define FIM_HALF_MAX_SIGNED(T) ((T)1 << (sizeof(T)*FIM_CHAR_BIT-2))
#define FIM_MAX_SIGNED(T) (FIM_HALF_MAX_SIGNED(T) - 1 + FIM_HALF_MAX_SIGNED(T))
#define FIM_MAX_VALUE_FOR_TYPE(T) (FIM_IS_SIGNED(T)?FIM_MAX_SIGNED(T):FIM_MAX_UNSIGNED(T))
#define FIM_MAX_INT FIM_MAX_VALUE_FOR_TYPE(fim_int) 

#endif /* FIM_COMMON_H */
