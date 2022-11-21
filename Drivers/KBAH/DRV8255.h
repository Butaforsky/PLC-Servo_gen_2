/**
 * @file DRV8255.h
 * @author butaforsky (buta4sky@yandex.ru)
 * @brief
 * @version 0.1
 * @date 2022-07-24
 *
 * @copyright Copyright (c) 2022
 *
 */

#ifndef __DRV8255_H
#define __DRV8255_H

#define TIM_STEP 1
/* includes */
#include "main.h"
#include "tim.h"
//! main.h file must include "delay_micros.h" file for delay_us func
/* typedef */
typedef struct drv8255
{
  GPIO_TypeDef *step_port;
  GPIO_TypeDef *dir_port;

  u16 step_pin;
  u16 dir_pin;

  u16 step_delay_us;

  GPIO_TypeDef *n_enable_port;
  u16 n_enable_pin;

  i32 position;
  i32 target_position;
  u16 max_position;

  u8 inversing;
 // for s - wave revUp
  u8 stage;
  
  u16 init_period;
  u16 first_period;
  u16 second_period;


#if TIM_STEP == 1
  TIM_HandleTypeDef *htim;
  vu32 channel;
#endif

} drv8255_t;

enum DIR
{
  FORWARD,
  BACKWARD,
};

enum MOTOR_STATE
{
  RUNNING, 
  COMPLETE,
};


/* private function declarations */

/**
 * @brief make step in selected direction
 *
 * @param direction according to enum in DRV8255.h 0 - FORWARD, 1 - BACKWARD;
 * @param motor pointer on drv8255 instance with initialized ports, pins and delays;
 */
void drv8255_make_step(drv8255_t *motor);

/**
 * @brief make n steps in selected direction
 *
 * @param direction 0 - FORWARD, 1 - BACKWARD
 * @param motor pointer on drv8255 instance with initialized ports, pins and delays;
 * @param steps n of steps in selected direction
 */
void drv8255_make_steps(drv8255_t *motor, enum DIR direction, u16 steps);

void drv8255_make_steps_z(drv8255_t *motor, enum DIR direction, u16 target_steps);

void drv8255_set_nenbl(drv8255_t *motor, u16 new_state);

void drv8255_set_dir(drv8255_t *motor, enum DIR direction);

void drv8255_set_position(drv8255_t *motor, u16 position);

u8 drv8255_calc_stage(drv8255_t *motor);

u8 drv8255_pulse_cmplt_callback(drv8255_t *motor);

#endif