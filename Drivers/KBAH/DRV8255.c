/**
 * @file DRV8255.c
 * @author  butaforsky (buta4sky@gmail.com)
 * @brief
 * @version 0.1
 * @date 2022-07-24
 *
 * @copyright Copyright (c) 2022
 *
 */

#define FREERTOS 1

#include "DRV8255.h"

#if FREERTOS == 1
#include "cmsis_os.h"

#endif
/* instances of motor */
drv8255_t servo_1 = { // X
    .step_delay_us = 2,

    .dir_port = GPIOA,
    .dir_pin = 4,

    .step_port = GPIOC,
    .step_pin = 3,

    .n_enable_port = GPIOB,
    .n_enable_pin = 0,

    .position = 0,
    .max_position = 100,

    .htim = &htim2,
    .channel = TIM_CHANNEL_1,

    .init_period = 1000, // TODO - test this
    .first_period = 500,
    .second_period = 750,

    .stage = 0,

};

drv8255_t step_2 = { // Z
    .step_delay_us = 2,

    .dir_port = GPIOA,
    .dir_pin = 5,

    .step_port = GPIOB,
    .step_pin = 4,

    .n_enable_port = GPIOB,
    .n_enable_pin = 1,

    .position = 0,
    .max_position = 100,

    .htim = &htim2,
    .channel = TIM_CHANNEL_2,

    .init_period = 1000, // TODO - test this
    .first_period = 500,
    .second_period = 750,

    .stage = 0,
};

drv8255_t step_1 = {   // Y
    .step_delay_us = 2,

    .dir_port = GPIOA,
    .dir_pin = 6,

    .step_port = GPIOD,
    .step_pin = 6,

    .n_enable_port = GPIOB,
    .n_enable_pin = 2,

    .position = 0,
    .max_position = 100,

    .htim = &htim2,
    .channel = TIM_CHANNEL_3,

    .init_period = 1000, // TODO - test this
    .first_period = 500,
    .second_period = 750,

    .stage = 0,

};

void drv8255_make_step(drv8255_t *motor)
{
  /* set step pin high/low */
  motor->step_port->ODR ^= (1 << motor->step_pin);
}

void drv8255_make_steps(drv8255_t *motor, enum DIR direction, u16 target_steps)
{
  // motor->htim->Instance->ARR = 1000;
  motor->htim->Instance->ARR = 0;
  direction = (motor->target_position > motor->position);
  motor->inversing = direction;
  motor -> first_period = motor->init_period;
  drv8255_set_nenbl(motor, 0);
  osDelay(500);
  drv8255_set_dir(motor, direction);
  osDelay(1);
  if (motor->channel == TIM_CHANNEL_1)
  {
    motor->htim->Instance->ARR = motor->init_period;
    motor->htim->Instance->CCR1 = motor->first_period / 2;
    HAL_TIM_PWM_Start_IT(motor->htim, TIM_CHANNEL_1);
  }
  else if (motor->channel == TIM_CHANNEL_2)
  {
    motor->htim->Instance->ARR = motor->init_period;
    motor->htim->Instance->CCR2 = motor->first_period;
    HAL_TIM_PWM_Start_IT(motor->htim, TIM_CHANNEL_2);
  }
  else if (motor->channel == TIM_CHANNEL_3)
  {
    motor->htim->Instance->ARR = motor->init_period;
    motor->htim->Instance->CCR3 = motor->first_period / 2;
    HAL_TIM_PWM_Start_IT(motor->htim, TIM_CHANNEL_3);
  }

  // drv8255_set_nenbl(motor, 1);
}


void drv8255_make_steps_z(drv8255_t *motor, enum DIR direction, u16 target_steps)
{
  motor->htim->Instance->ARR = 0;
  direction = (motor->target_position > motor->position);
  motor->inversing = direction;
  motor -> first_period = motor->init_period;
  drv8255_set_nenbl(motor, 0);
  drv8255_set_dir(motor, direction);
  if (motor->channel == TIM_CHANNEL_1)
  {
    motor->htim->Instance->ARR = motor->init_period;
    motor->htim->Instance->CCR1 = motor->first_period;
    HAL_TIM_PWM_Start_IT(motor->htim, TIM_CHANNEL_1);
  }
  else if (motor->channel == TIM_CHANNEL_2)
  {
    motor->htim->Instance->ARR = motor->init_period;
    motor->htim->Instance->CCR2 = motor->first_period;
    HAL_TIM_PWM_Start_IT(motor->htim, TIM_CHANNEL_2);
  }
  else if (motor->channel == TIM_CHANNEL_3)
  {
    motor->htim->Instance->ARR = motor->init_period;
    motor->htim->Instance->CCR3 = motor->first_period;
    HAL_TIM_PWM_Start_IT(motor->htim, TIM_CHANNEL_3);
  }

  // drv8255_set_nenbl(motor, 1);
}

void drv8255_set_nenbl(drv8255_t *motor, u16 new_state)
{
  if (0 == new_state)
  {
    motor->n_enable_port->ODR &= ~(1 << motor->n_enable_pin);
  }
  else if (1 == new_state)
  {
    motor->n_enable_port->ODR |= (1 << motor->n_enable_pin);
  }
}

void drv8255_set_dir(drv8255_t *motor, enum DIR direction)
{
  if (FORWARD == direction)
  {
    motor->dir_port->ODR |= (1 << motor->dir_pin);
  }
  else if (BACKWARD == direction)
  {
    motor->dir_port->ODR &= ~(1 << motor->dir_pin);
  }
}

void drv8255_set_position(drv8255_t *motor, u16 position)
{
  if (motor->inversing == 0)
  {
    drv8255_make_steps(motor, FORWARD, position);
  }
  else if (motor->inversing == 1)
  {
    drv8255_make_steps(motor, BACKWARD, position);
  }
}

u8 drv8255_calc_stage(drv8255_t *motor)
{
  if (motor->target_position / 4 > motor->position)
  {
    motor->stage = 3;
  }
  else if (motor->target_position / 2 > motor->position)
  {
    motor->stage = 2;
  }
  else if (motor->target_position > motor->position)
  {
    motor->stage = 1;
  }
}

u8 drv8255_pulse_cmplt_callback(drv8255_t *motor)
{
  if(motor->inversing == 1)
  {
    motor->position++;
  }
  else if(motor->inversing == 0)
  {
    motor->position--;
  }
  if (motor->position != motor->target_position)
  {
    GPIOC->ODR ^= (1 << 13);
    return RUNNING;
  }
  else
  {
    HAL_TIM_PWM_Stop_IT(motor->htim, motor->channel);
    return COMPLETE;
  }
}