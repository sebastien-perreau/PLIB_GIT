#ifndef __DEF_STRING_ADVANCE
#define __DEF_STRING_ADVANCE

// New MACROS for standard String functions.
#define str_cpy     strcpy
#define str_ncpy    strncpy
#define str_cat     strcat
#define str_ncat    strncat
#define str_cmp     strcmp
#define str_ncmp    strncmp
#define str_len     strlen
#define str_chr     strchr
#define str_rchr    strrchr
#define str_str     strstr
#define str_spn     strspn
#define str_pbrk    strpbrk
#define str_tok     strtok

typedef enum
{
    BASE_2  = 2,
    BASE_8  = 8,
    BASE_10 = 10,
    BASE_16 = 16
} STR_BASE_t;

void transform_uint8_t_tab_to_string(char *p_buffer, uint16_t buffer_length, uint8_t *data, uint8_t length, STR_BASE_t _base);
char *str_tolower (const char *ct);
char *str_toupper (const char *ct);
int str_istr (const char *cs, const char *ct);
char *str_sub (const char *s, WORD start, WORD end);
char *str_replace (const char *s, unsigned int start, unsigned int length, const char *ct);
char *str_strip (const char *string);

#endif
