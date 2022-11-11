#ifndef __PARSER_H
#define __PARSER_H
/* includes */
#include <stdio.h>
#include <string.h>
#include "short_types.h"
#include "uart_lib.h"
#include "tasks.h"
#include "main.h"

#define WORD_MAX_LEN 12


typedef struct word
{
  u8 beg;
  u8 end;
  u8 word_len;
  u8 end_found;
} word_t;

typedef struct parsed_data
{
  u32 on_threshold;
  u32 noise_threshold;
  u32 smooth_val;
} data_t;

typedef struct parser
{
  uart_t *uart_inst;
  word_t *first_word_desc;
  data_t *parsed_parameters; // new parameters from user
  u8 end_string_found;
  u8 mute;
} parser_t;

void input_clear(word_t *input);

u8 uart_parse(parser_t *parser);

u8 clear(void *word);
u8 convert_caps_to_reg(u8 caps_symb);

void clear_get_new_command(parser_t *parser);
u8 compare(char *word, const void *text);

#endif
