		printf("rand: %d\n", get_rand_delay_ms());
    towards_duckypad_send(0xab);
    HAL_Delay(500);
void channel_update(void)
{
  uint8_t this_channel_status[CHANNEL_COUNT];

  this_channel_status[CHANNEL_1] = HAL_GPIO_ReadPin(CH1_GPIO_Port, CH1_Pin);
  this_channel_status[CHANNEL_2] = HAL_GPIO_ReadPin(CH2_GPIO_Port, CH2_Pin);
  this_channel_status[CHANNEL_3] = HAL_GPIO_ReadPin(CH3_GPIO_Port, CH3_Pin);
  this_channel_status[CHANNEL_4] = HAL_GPIO_ReadPin(CH4_GPIO_Port, CH4_Pin);
  this_channel_status[CHANNEL_5] = HAL_GPIO_ReadPin(CH5_GPIO_Port, CH5_Pin);
  this_channel_status[CHANNEL_6] = HAL_GPIO_ReadPin(CH6_GPIO_Port, CH6_Pin);

  for (int i = 0; i < CHANNEL_COUNT; ++i)
  {
    // if(button_status[i].prev_state == BUTTON_RELEASED && button_status[i].button_state == BUTTON_PRESSED)
    //   mark_as_pressed(i);
    // else if(button_status[i].prev_state == BUTTON_PRESSED && button_status[i].button_state == BUTTON_RELEASED)
    //   mark_as_released(i);
    // button_status[i].prev_state = button_status[i].button_state;
    printf("%d ", this_channel_status[i]);
  }
  printf("\n");
}