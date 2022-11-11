#include "parser.h"
/* private variables */
/* private functions */
// TODO - clear first word params after successful parse
void parser_uart_callback(UART_HandleTypeDef *huart, USART_TypeDef *UART, parser_t *parser)
{
  if (huart->Instance == UART)
  {
    if (UART->DR >= 85 && UART->DR <= 90)
    {
      parser->uart_inst->byte[0] = convert_caps_to_reg(parser->uart_inst->byte[0]);
    }
    parser->uart_inst->rx_buffer[parser->uart_inst->tail] = parser->uart_inst->byte[0];
    if (UART->DR != 0x0A) // if recieved byte is not \n
    {
      if (parser->first_word_desc->end_found == 0 &&
          parser->uart_inst->rx_buffer[parser->uart_inst->tail] == 0x20)
      {
        parser->first_word_desc->end_found = 1;
        parser->first_word_desc->end = parser->uart_inst->tail;
        parser->first_word_desc->word_len = parser->uart_inst->tail;
      }
      parser->uart_inst->tail++;
      // HAL_UART_Receive_DMA(&user_uart, (u8 *)uart.byte, 1);
    }
  }
}

void input_clear(word_t *input)
{
  input->beg = 0;
  input->end = 0;
  input->end_found = 0;
}

void clear_get_new_command(parser_t *parser)
{
  parser->end_string_found = 0;
  memset(parser->uart_inst->tx_buffer, 0, strlen((char *)parser->uart_inst->tx_buffer));
  memset(parser->uart_inst->rx_buffer, 0, strlen((char *)parser->uart_inst->rx_buffer));
}

u8 clear(void *word)
{
  memset(word, 0, strlen(word));
  return 0;
}

u8 convert_caps_to_reg(u8 caps_symb)
{
  return caps_symb + (97 - 65); /* 97 - A, 65 - a */
}

u8 compare(char *word, const void *text)
{
  return memcmp((void *)word, text, strlen(text));
}

u8 uart_parse_error(parser_t *parser)
{
  sprintf((char *)parser->uart_inst->tx_buffer, "ERROR\n");
  return HAL_UART_Transmit(parser->uart_inst->uart,
                           (u8 *)parser->uart_inst->tx_buffer,
                           strlen((char *)parser->uart_inst->tx_buffer),
                           parser->uart_inst->timeout_ms);
}

u8 uart_parse_clear(parser_t *jet, word_t *input)
{
  jet->end_string_found = 0;
  input->beg = 0;
  input->end = 0;
  input->end_found = 0;
  input->word_len = 0;
}

u8 uart_parse(parser_t *parser)
{
  u8 word_len = parser->first_word_desc->word_len;

  char word[3][WORD_MAX_LEN] = {
      0,
  }; /* create a local array for parsed words */

  if (word_len == 3) /* if first word len is 3 */
  {
    sscanf((char *)parser->uart_inst->rx_buffer, "%s %s ", word[0], word[1]); /*get first two words */

    if ((compare(word[0], "set") == 0) && (compare(word[1], "on") == 0))
    {
      sscanf((char *)parser->uart_inst->rx_buffer, "set on %d ",
             &parser->parsed_parameters->on_threshold);
      uart_send(parser->uart_inst, parser->uart_inst->rx_buffer); // echo
      uart_send_format(parser->uart_inst, sprintf(parser->uart_inst->tx_buffer,
                                                  "��������� ��� �������: %d\r\n",
                                                  parser->parsed_parameters->on_threshold));
      uart_send_ok(parser->uart_inst);

      return TASK_SET_ON_THRESHOLD;
    }

    else if ((compare(word[0], "get") == 0) && (compare(word[1], "on") == 0))
    {
      uart_send(parser->uart_inst, parser->uart_inst->rx_buffer);
      uart_send_format(parser->uart_inst, sprintf(parser->uart_inst->tx_buffer,
                                                  "��������� ��� �������: %d\r\n",
                                                  parser->parsed_parameters->on_threshold));
      uart_send_ok(parser->uart_inst);
      return TASK_GET_ON_THRESHOLD;
    }

    else if ((compare(word[0], "set") == 0) && (compare(word[1], "noise") == 0))
    {
      sscanf((char *)parser->uart_inst->rx_buffer, "set noise %d ",
             &parser->parsed_parameters->noise_threshold);
      uart_send(parser->uart_inst, parser->uart_inst->rx_buffer);
      uart_send_format(parser->uart_inst, sprintf((char *)parser->uart_inst->tx_buffer,
                                                  "����� ��������� �����: %d\r\n",
                                                  parser->parsed_parameters->noise_threshold));
      uart_send_ok(parser->uart_inst);
      return TASK_SET_NOISE_THRESHOLD;
    }

    else if ((compare(word[0], "get") == 0) && (compare(word[1], "noise") == 0))
    {

      uart_send_format(parser->uart_inst, sprintf((char *)parser->uart_inst->tx_buffer,
                                                  "����� ��������� �����: %d\r\n",
                                                  parser->parsed_parameters->noise_threshold));
      uart_send_ok(parser->uart_inst);
      return TASK_GET_NOISE_THRESHOLD;
    }

    else if ((compare(word[0], "get") == 0) && (compare(word[1], "smooth") == 0))
    {
      uart_send(parser->uart_inst, parser->uart_inst->rx_buffer);
      uart_send_format(parser->uart_inst, sprintf((char *)parser->uart_inst->tx_buffer,
                                                  "������� ����� ���������: %d\r\n",
                                                  parser->parsed_parameters->smooth_val));
      uart_send_ok(parser->uart_inst);
      return TASK_GET_SMOOTH_VAL;
    }
    else if (!compare(word[0], "get") && !compare(word[1], "mute"))
    {
      uart_send(parser->uart_inst, parser->uart_inst->rx_buffer);
      if (parser->mute == 0)
      {
        uart_send_format(parser->uart_inst, sprintf((char *)parser->uart_inst->tx_buffer,
                                                    "����� �������������\r\n"));
      }
      else
      {
        uart_send_format(parser->uart_inst, sprintf((char *)parser->uart_inst->tx_buffer,
                                                    "����� ������������\r\n"));
      }
    }

    else if ((compare(word[0], "set") == 0) && (compare(word[1], "smooth") == 0))
    {
      uart_send(parser->uart_inst, parser->uart_inst->rx_buffer);
      sscanf((char *)parser->uart_inst->rx_buffer, "set smooth %d ",
             &parser->parsed_parameters->smooth_val);

      uart_send_format(parser->uart_inst, sprintf((char *)parser->uart_inst->tx_buffer,
                                                  "������� ����� ���������: %d\r\n",
                                                  parser->parsed_parameters->smooth_val));
      uart_send_ok(parser->uart_inst);
      return TASK_SET_SMOOTH_VAL;
    }

    else
    {
      uart_send_error(parser->uart_inst);
      return 0xFF;
    }
  }

  else if (word_len == 4) /* first word == "help" */
  {
    sscanf((char *)parser->uart_inst->rx_buffer, "%s", word[0]);
    if (!compare(word[0], "help"))
    {
      uart_send(parser->uart_inst, "HELP\r\n");
      uart_send(parser->uart_inst, "\r\n������ ������:\r\n"
                                   "HELP - ����� ������� ����������� �����������\r\n"
                                   "SET ON - ���������� ����� ���������� ��������� ��� ������� ������ (�� - ���������: 50)\r\n"
                                   "GET ON - �������� ������� �������� ��������� ��� ������� ������\r\n"
                                   "SET NOISE - ���������� ����� �������� ������ ����� ��� �������������� (�� - ���������: 25)\r\n"
                                   "�������������� ������� (�������� ������ ���� ������ ���������� ��������� ��� �������)\r\n"
                                   "GET NOISE - �������� ������� �������� ������ ����� (�� - ���������: 1100)\r\n"
                                   "SET SMOOTH - ���������� ����� �������� ������ ���������, ������������ �� ���� \r\n"
                                   "GET SMOOTH - �������� ������� �������� ������ ���������, ������������ �� ����\r\n"
                                   "\r\nOPEN - ������� ������\r\n"
                                   "CLOSE - ������� ������\r\n"
                                   "STOP - ���������� ����\r\n"
                                   "MUTE - ����������� ������ ������\r\n"
                                   "UNMUTE - �������������� ������ ������\r\n"
                                   "GET MUTE - �������� ������ ������\r\n");
      return TASK_HELP;
    }
    else if (!compare(word[0], "open"))
    {
      uart_send(parser->uart_inst, "OPEN\r\n");
      return TASK_OPEN;
    }
    else if (!compare(word[0], "stop"))
    {
      uart_send(parser->uart_inst, "STOP\r\n");
      return TASK_STOP;
    }
    else if (!compare(word[0], "data"))
    {
      uart_send(parser->uart_inst, "logdata:\r\n");
      u32 *base = 0x0800C800;
      for (u32 i = 0; i <= LOG_LEN; i++)
      {
        uart_send_format(parser->uart_inst,
                         sprintf((char *)parser->uart_inst->tx_buffer, "%x\r\n", base[0]));
        base++;
      }
      uart_send(parser->uart_inst, "\r\n\r\n");
    }

    else if (!compare(word[0], "mute"))
    {
      uart_send(parser->uart_inst, "����� ������������\r\n");
      parser->mute = 1;
      return TASK_MUTE;
    }
    else
    {
      return 0xFF;
    }
  }

else if (word_len == 5)
{
  sscanf((char *)parser->uart_inst->rx_buffer, "%s", word[0]);
  if (compare(word[0], "close") == 0)
  {
    uart_send(parser->uart_inst, "CLOSE\r\n");
    return TASK_CLOSE;
  }
}
else if (word_len == 6)
{
  sscanf((char *)parser->uart_inst->rx_buffer, "%s", word[0]);
  if (!compare(word[0], "unmute"))
  {
    uart_send(parser->uart_inst, "����� �������������\r\n");
    parser->mute = 0;
    return TASK_UNMUTE;
  }
}
else if (word_len == 7)
{
  sscanf((char *)parser->uart_inst->rx_buffer, "%s", word[0]);
  if (!compare(word[0], "restart"))
  {
    HAL_NVIC_SystemReset();
    uart_send_ok(parser->uart_inst);
  }
}
else
{
  uart_send_error(parser->uart_inst);
  return 0xFF;
}
uart_send_error(parser->uart_inst);
return 0xFF;
}