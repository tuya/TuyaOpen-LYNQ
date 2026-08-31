#ifndef MSH_PARSE_H
#define MSH_PARSE_H

#include <rtdef.h>

extern rt_bool_t msh_isint(char *strvalue);
extern rt_bool_t msh_ishex(char *strvalue);
extern int msh_strtohex(char *strvalue);

#endif /* MSH_PARSE_H */
