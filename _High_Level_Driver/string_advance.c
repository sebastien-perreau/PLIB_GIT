/*********************************************************************
*	String advance functions
*	Author : Sébastien PERREAU
*
*	Revision history	:
*               15/09/2018		- Initial release
*********************************************************************/

#include "../PLIB.h"


/*******************************************************************************
  Function:
    void transform_uint8_t_tab_to_string(char *p_buffer, uint16_t buffer_length, uint8_t *data, uint8_t length, STR_BASE_t _base)

  Description:
    This routine allows you to create a string from an uint8_t array. You can 
    specify the 'base_x' transformation for the string. 
 
  Parameters:
    *p_buffer:      It is the char buffer where the string will be store.
    buffer_length:  You have to specify the size of the buffer containing the string. It
                    is necessary to prevent an override of memory (if string is bigger than 
                    the buffer).
    *data:          It is the uint8_t buffer (the data you want to transform into a string).
    length:         The number of data you want to transform (maximum sizeof(data)).
    _base:          It is the 'base_x' transformation (see. STR_BASE_t for more details) you want
                    to apply.
  *****************************************************************************/
void transform_uint8_t_tab_to_string(char *p_buffer, uint16_t buffer_length, uint8_t *data, uint8_t length, STR_BASE_t _base)
{
    uint16_t index_p_buffer = 0;
    uint16_t index_data = 0;
    static char dictionary_char[]= "0123456789ABCDEF";
	static char buffer[20] = {0}; 
	char *ptr = &buffer[19];
    uint8_t val;
    
    p_buffer[index_p_buffer++] = 't';
    p_buffer[index_p_buffer++] = 'a';
    p_buffer[index_p_buffer++] = 'b';
    p_buffer[index_p_buffer++] = '[';
    
    if (_base == BASE_2)
    {
        p_buffer[index_p_buffer++] = 'b';
    }
    else if (_base == BASE_16)
    {
        p_buffer[index_p_buffer++] = '0';
        p_buffer[index_p_buffer++] = 'x';
    }
    
    p_buffer[index_p_buffer++] = ']';
    p_buffer[index_p_buffer++] = ' ';
    p_buffer[index_p_buffer++] = '=';
    p_buffer[index_p_buffer++] = ' ';
    
    for (index_data = 0 ; index_data < length ; index_data++)
    {
        val = data[index_data];
        do 
        { 
            *--ptr = dictionary_char[val%_base]; 
            val /= _base; 
        }
        while (val != 0); 

        do
        {
            if (index_p_buffer >= (buffer_length-4))
            {
                p_buffer[index_p_buffer++] = '.';
                p_buffer[index_p_buffer++] = '.';
                p_buffer[index_p_buffer++] = '.';
                goto out_of_nested_loop;
            }
            p_buffer[index_p_buffer++] = *ptr++;
        }
        while (*ptr != '\0');
        
        if (index_data < (length - 1))
        {
            p_buffer[index_p_buffer++] = ' ';
        }
    }
    out_of_nested_loop:
    p_buffer[index_p_buffer++] = '\0';
}

/*******************************************************************************
  Function:
    char *str_tolower (const char *ct)

  Description:
    This routine allows you to lowercase all character that the string contains.
  *****************************************************************************/
char *str_tolower (const char *ct)
{
   char *s = NULL;
   int i = 0;

   if (ct != NULL)
   {
      s = malloc (sizeof (*s) * (strlen (ct) + 1));
      if (s != NULL)
      {
         for (i = 0; ct[i]; i++)
         {
            s[i] = tolower (ct[i]);
         }
         s[i] = '\0';
      }
   }
   return s;
}

/*******************************************************************************
  Function:
    char *str_toupper (const char *ct)

  Description:
    This routine allows you to uppercase all character that the string contains.
  *****************************************************************************/
char *str_toupper (const char *ct)
{
   char *s = NULL;
   int i = 0;

   if (ct != NULL)
   {
      s = malloc (sizeof (*s) * (strlen (ct) + 1));
      if (s != NULL)
      {
         for (i = 0; ct[i]; i++)
         {
            s[i] = toupper (ct[i]);
         }
         s[i] = '\0';
      }
   }
   return s;
}

/*******************************************************************************
  Function:
    int str_istr (const char *cs, const char *ct)

  Description:
    This routine allows you to find the "table" index of a sub-string (ct) contained 
    in a string (cs). You can use the function str_str whiwh allows you to find the
    adress of the first character of a sub-string (ct) contained in a string (cs).
  *****************************************************************************/
int str_istr (const char *cs, const char *ct)
{
   int index = -1;

   if (cs != NULL && ct != NULL)
   {
      char *ptr_pos = NULL;

      ptr_pos = strstr (cs, ct);
      if (ptr_pos != NULL)
      {
         index = ptr_pos - cs;
      }
   }
   return index;
}

/*******************************************************************************
  Function:
    char *str_sub (const char *s, WORD start, WORD end)

  Description:
    This routine allows you to extract a part of the string (s) from Start to End.
  *****************************************************************************/
char *str_sub (const char *s, WORD start, WORD end)
{
   char *new_s = NULL;
   int i = 0;

   if ((s != NULL) && (start < end))
   {
      new_s = malloc (sizeof (*new_s) * (end - start + 2));
      if (new_s != NULL)
      {
         for (i = start; i <= end; i++)
         {
            new_s[i-start] = s[i];
         }
         new_s[i-start] = '\0';
      }
   }
   return new_s;
}

/*******************************************************************************
  Function:
    char *str_replace (const char *s, unsigned int start, unsigned int lenght, const char *ct)

  Description:
    This routine allows you to replace "Length" characters of String (s) from Start 
    with a new sub-string (ct).
  *****************************************************************************/
char *str_replace (const char *s, unsigned int start, unsigned int lenght, const char *ct)
{
   char *new_s = NULL;

   if ((s != NULL) && (ct != NULL) && (start >= 0) && (lenght > 0))
   {
      size_t size = strlen (s);
      new_s = malloc (sizeof (*new_s) * (size - lenght + strlen (ct) + 1));
      if (new_s != NULL)
      {
         memcpy (new_s, s, start);
         memcpy (&new_s[start], ct, strlen (ct));
         memcpy (&new_s[start + strlen (ct)], &s[start + lenght], size - lenght - start + 1);
      }
   }
   return new_s;
}

/*******************************************************************************
  Function:
    char *str_strip (const char *string)

  Description:
    This routine allows you to remove all unnecessary space in a String.
  *****************************************************************************/
char *str_strip (const char *string)
{
   char *strip = NULL;
   int i = 0, j = 0, ps = 0;
   
   if (string != NULL)
   {
      strip = malloc (sizeof (*strip) * (strlen (string) + 1));
      if (strip != NULL)
      {
         for (i = 0, j = 0; string[i]; i++)
         {
            if (string[i] == ' ')
            {
               if (ps == 0)
               {
                  strip[j] = string[i];
                  ps = 1;
                  j++;
               }
            }
            else
            {
               strip[j] = string[i];
               ps = 0;
               j++;
            }
         }
         strip[j] = '\0';
      }
   }
   return strip;
}
