/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2023 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "lwip.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

#include "mqtt_client.h"
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
struct mqtt_client_t mqtt_client;
struct mqtt_client_connect_opts_t conn_opts;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

void mqtt_msg_received_user_cb(struct mqtt_publish_resp_t* publish_resp)
{
	uint8_t topic_str[publish_resp->topic_len + 1];
	memcpy(topic_str, publish_resp->topic, publish_resp->topic_len);
	topic_str[publish_resp->topic_len] = '\0';

	uint8_t data_str[publish_resp->data_len + 1];
	memcpy(data_str, publish_resp->data, publish_resp->data_len);
	data_str[publish_resp->data_len] = '\0';

	printf("msg:%s at:%s\n", data_str, topic_str);
}


uint32_t current_time = 0;
uint32_t previous_time = 0;

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_LWIP_Init();
  /* USER CODE BEGIN 2 */

  conn_opts.client_id = "stm krzysiu";
  conn_opts.username = NULL;
  conn_opts.password = NULL;
  conn_opts.will_topic = "info/device";
  conn_opts.will_msg = "stm disc";
  conn_opts.will_qos = 0;
  conn_opts.will_retain = false;
  conn_opts.keepalive_ms = 10000;  // 10 sec

  MQTTClient_init(&mqtt_client, mqtt_msg_received_user_cb, HAL_GetTick, &conn_opts);
  enum mqtt_client_err_t connect_rc = MQTTClient_connect(&mqtt_client, 5000);
  if (connect_rc == MQTT_TIMEOUT_ON_CONNECT)
  {
	  printf("Timeout on connection to MQTT broker.\n");
	  while (1);
  }
  else if (connect_rc == MQTT_CONNECTION_REFUSED_BY_BROKER)
  {
	  printf("Connection refused by MQTT broker. The reason: %d\n", mqtt_client.connack_resp.conn_rc);
	  while (1);
  }
  else if (connect_rc == MQTT_TCP_CONNECT_FAILURE)
  {
	  printf("No TCP connection\n");
	  while (1);
  }
  else if (connect_rc == MQTT_MEMORY_ERR)
  {
	  printf("Memory error\n");
	  while (1);
  }
  else
  {
	  printf("Connected to MQTT broker successfully\n");
  }



  enum mqtt_client_err_t pub_rc[3];
  pub_rc[0] = MQTTClient_publish(&mqtt_client, "sensor/temp", "qos 0 msg", 0, false);
  pub_rc[1] = MQTTClient_publish(&mqtt_client, "sensor/temp", "qos 1 msg", 1, false);
  pub_rc[2] = MQTTClient_publish(&mqtt_client, "sensor/temp", "qos 2 msg", 2, false);
  printf("Published with rc: %d, %d, %d\n", pub_rc[0], pub_rc[1], pub_rc[2]);

  enum mqtt_client_err_t sub_rc[3];
  sub_rc[0] = MQTTClient_subscribe(&mqtt_client, "drive/voltage", 0);
  sub_rc[1] = MQTTClient_subscribe(&mqtt_client, "drive/current", 1);
  sub_rc[2] = MQTTClient_subscribe(&mqtt_client, "drive/power", 2);
  printf("Subscribed with rc: %d, %d, %d\n", sub_rc[0], sub_rc[1], sub_rc[2]);


  MQTTClient_publish(&mqtt_client, "sensor/temp", "check if ok payload", 2, false);


  previous_time = HAL_GetTick();
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
	  MQTTClient_loop(&mqtt_client);

	  current_time = HAL_GetTick();
	  if (current_time - previous_time >= 20000)
	  {
		  pub_rc[0] = MQTTClient_publish(&mqtt_client, "sensor/temp", "25 celsius", 1, false);
		  pub_rc[1] = MQTTClient_publish(&mqtt_client, "sensor/magnet", "5 uT", 0, false);
		  pub_rc[2] = MQTTClient_publish(&mqtt_client, "sensor/acc", "5 g", 2, false);
		  previous_time = current_time;
		  printf("Published with rc: %d, %d, %d\n", pub_rc[0], pub_rc[1], pub_rc[2]);
	  }
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 168;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
