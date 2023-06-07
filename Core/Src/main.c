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
const struct mqtt_client_connect_opts_t conn_opts = {
	.client_id="stm krzysiu",
	.username=NULL,
	.password=NULL,
	.will_msg.topic="info/device",
	.will_msg.payload = "I was disconnected",
	.will_msg.qos = 0,
	.will_msg.retain = false,
	.keepalive_ms = 10000
};
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

void on_msg_received_cb(union mqtt_context_t* context)
{
	printf("Received message from topic: %s : %s, payload len: %d\n", context->pub.topic, context->pub.payload, context->pub.payload_len);
}

void on_sub_completed_cb(struct mqtt_suback_resp_t* suback_resp, union mqtt_context_t* context)
{
	if (suback_resp->suback_rc == 0x80)
	{
		printf("Failed to subscribe to topic: %s\n", context->sub.topic);
	}
	else
	{
		printf("Subscribed to topic: %s with max QOS: %d\n", context->sub.topic, suback_resp->suback_rc);
	}
}

void on_pub_completed_cb(union mqtt_context_t* context)
{
	printf("Publish msg: %s to topic: %s completed\n", context->pub.payload, context->pub.topic);
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

  MQTTClient_init(&mqtt_client, HAL_GetTick, &conn_opts, 5000);
  MQTTClient_set_cb_on_msg_received(&mqtt_client, on_msg_received_cb);
  MQTTClient_set_cb_on_sub_completed(&mqtt_client, on_sub_completed_cb);
  MQTTClient_set_cb_on_pub_completed(&mqtt_client, on_pub_completed_cb);

  enum mqtt_client_err_t connect_rc = MQTTClient_connect(&mqtt_client);
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

  struct mqtt_pub_msg_t pub_msg_qos_0 = { .topic="sensor/temp", .payload="qos 0 msg", .qos=0, .retain=false };
  struct mqtt_pub_msg_t pub_msg_qos_1 = { .topic="sensor/temp", .payload="qos 1 msg", .qos=1, .retain=false };
  struct mqtt_pub_msg_t pub_msg_qos_2 = { .topic="sensor/temp", .payload="qos 2 msg", .qos=2, .retain=false };

  MQTTClient_publish(&mqtt_client, &pub_msg_qos_0);
  MQTTClient_publish(&mqtt_client, &pub_msg_qos_1);
  MQTTClient_publish(&mqtt_client, &pub_msg_qos_2);

  struct mqtt_sub_msg_t sub_voltage = { .topic="drive/voltage", .qos=0 };
  struct mqtt_sub_msg_t sub_current = { .topic="drive/current", .qos=1 };
  struct mqtt_sub_msg_t sub_power = { .topic="drive/power", .qos=2 };

  MQTTClient_subscribe(&mqtt_client, &sub_voltage);
  MQTTClient_subscribe(&mqtt_client, &sub_current);
  MQTTClient_subscribe(&mqtt_client, &sub_power);

  struct mqtt_pub_msg_t new_msg = { .topic="sensor/temp", .payload="check if ok payload", .qos=2, .retain=false };
  MQTTClient_publish(&mqtt_client, &new_msg);

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
	  if (current_time - previous_time >= 20000 && mqtt_client.mqtt_connected)
	  {
		  struct mqtt_pub_msg_t temp_msg = { .topic="sensor/temp", .payload="25 celsius", .qos=1, .retain=false };
		  struct mqtt_pub_msg_t magnet_msg = { .topic="sensor/magnet", .payload="5 uT", .qos=0, .retain=false };
		  struct mqtt_pub_msg_t acc_msg = { .topic="sensor/acc", .payload="5 g", .qos=2, .retain=false };

		  MQTTClient_publish(&mqtt_client, &temp_msg);
		  MQTTClient_publish(&mqtt_client, &magnet_msg);
		  MQTTClient_publish(&mqtt_client, &acc_msg);
		  previous_time = current_time;

		  struct mqtt_pub_msg_t status_msg = { .topic="info/device", .payload="ok", .qos=2, .retain=false };
		  MQTTClient_publish(&mqtt_client, &status_msg);
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
