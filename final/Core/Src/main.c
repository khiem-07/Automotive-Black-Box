/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "cmsis_os.h"
#include "fatfs.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "stdarg.h"
#include "string.h"
#include "fonts.h"
#include "ST7735.h"
#include "DS3231.h"
#include "stdio.h"
#include "freertos.h"
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
CAN_HandleTypeDef hcan;

I2C_HandleTypeDef hi2c1;

SPI_HandleTypeDef hspi1;
SPI_HandleTypeDef hspi2;

UART_HandleTypeDef huart1;
UART_HandleTypeDef huart2;
UART_HandleTypeDef huart3;

osThreadId Task01Handle;
/* USER CODE BEGIN PV */
osThreadId Task02Handle;
osThreadId Task03Handle;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_I2C1_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_USART3_UART_Init(void);
static void MX_CAN_Init(void);
static void MX_SPI1_Init(void);
static void MX_SPI2_Init(void);
void StartTask01(void const * argument);

/* USER CODE BEGIN PFP */
void StartTask02(void const * argument);
void StartTask03(void const * argument);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
char buff[80];
void mPrint(const char* format, ...){
    va_list args;
    va_start(args, format);
    vsnprintf(buff, sizeof(buff), format, args);
    va_end(args);
    HAL_UART_Transmit(&huart1, (uint8_t*)buff, strlen(buff), 1000);
}
static void processRxData(void);
CAN_RxHeaderTypeDef rxHeader; //CAN Bus Receive Header
CAN_TxHeaderTypeDef txHeader; //CAN Bus Transmit Header
uint8_t canRX[8] = {0,0,0,0,0,0,0,0}; //CAN Bus Receive Buffer
CAN_FilterTypeDef canfil; //CAN Bus Filter
uint32_t canMailbox; //CAN Bus Mail box variable

// Bi?n luu th�ng s? decimal sau chuy?n d?i
volatile float engine_rpm = 0.0f;
volatile int vehicle_speed = 0;
volatile float coolant_temp = 0.0f;
Datetime dt;

// --- KHAI BÁO CHO TÍNH NĂNG TAI NẠN ---
extern UART_HandleTypeDef huart2; // UART kết nối với SIM
float my_lat = 10.8505f; // Ví dụ: Đại học SPKT
float my_lon = 106.7719f;
uint8_t accident_detected = 0; // Cờ báo tai nạn (0: ko, 1: có)

// Hàm gửi lệnh AT cho SIM
void SIM_SendCommand(char* cmd) {
    HAL_UART_Transmit(&huart2, (uint8_t*)cmd, strlen(cmd), 1000);
    HAL_UART_Transmit(&huart2, (uint8_t*)"\r\n", 2, 100);
    HAL_Delay(500); // Đợi SIM phản hồi
}

// Hàm gửi tin nhắn khẩn cấp
void Send_Emergency_Alert() {
    if(accident_detected == 1) return; // Đã gửi rồi thì không gửi lại liên tục
    
    char msg_buffer[100];
    
    // 1. Cấu hình tin nhắn văn bản
    SIM_SendCommand("AT+CMGF=1"); 
    
    // 2. Gửi tin nhắn (Thay số điện thoại của bạn vào đây)
    SIM_SendCommand("AT+CMGS=\"0966976292\""); 
    
    // 3. Nội dung tin nhắn kèm vị trí
    sprintf(msg_buffer, "CANH BAO TAI NAN! Vi tri: Lat: %f, Lon: %f", my_lat, my_lon);
    HAL_UART_Transmit(&huart2, (uint8_t*)msg_buffer, strlen(msg_buffer), 1000);
    
    // 4. Ký tự Ctrl+Z (ASCII 26) để kết thúc tin nhắn
    uint8_t ctrl_z = 26;
    HAL_UART_Transmit(&huart2, &ctrl_z, 1, 100);
    HAL_Delay(3000); // Đợi tin nhắn đi
    
    // 5. Gọi điện thoại
    SIM_SendCommand("ATD0966976292;");
    
    accident_detected = 1; // Khóa lại để không spam
}

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
  MX_I2C1_Init();
  MX_USART1_UART_Init();
  MX_USART2_UART_Init();
  MX_USART3_UART_Init();
  MX_CAN_Init();
  MX_SPI1_Init();
  MX_SPI2_Init();
  MX_FATFS_Init();
  /* USER CODE BEGIN 2 */
	RTC_Init(&dt);
	
	 HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET);
  canfil.FilterBank = 0;
  canfil.FilterMode = CAN_FILTERMODE_IDMASK;
  canfil.FilterFIFOAssignment = CAN_RX_FIFO0;
  canfil.FilterIdHigh = 0;
  canfil.FilterIdLow = 0;
  canfil.FilterMaskIdHigh = 0;
  canfil.FilterMaskIdLow = 0;
  canfil.FilterScale = CAN_FILTERSCALE_32BIT;
  canfil.FilterActivation = ENABLE;
  canfil.SlaveStartFilterBank = 14;
	  // Config filter TRU?C khi start CAN
  HAL_CAN_ConfigFilter(&hcan, &canfil);
  HAL_CAN_Start(&hcan);
//  HAL_CAN_ActivateNotification(&hcan, CAN_IT_RX_FIFO0_MSG_PENDING);

  txHeader.DLC = 8;
  txHeader.IDE = CAN_ID_STD;
  txHeader.RTR = CAN_RTR_DATA;
  txHeader.ExtId = 0x00;  // Kh�ng d�ng cho STD ID
  txHeader.TransmitGlobalTime = DISABLE;
  /* USER CODE END 2 */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* definition and creation of Task01 */
  osThreadDef(Task01, StartTask01, osPriorityRealtime, 0, 128);
  Task01Handle = osThreadCreate(osThread(Task01), NULL);

  /* USER CODE BEGIN RTOS_THREADS */
	  osThreadDef(Task02, StartTask02, 2, 0, 128);
  Task02Handle = osThreadCreate(osThread(Task02), NULL);
	mPrint("Heap con lai: %d bytes\r\n", xPortGetFreeHeapSize());
	  osThreadDef(Task03, StartTask03, 1, 0, 2024);
  Task03Handle = osThreadCreate(osThread(Task03), NULL);
	mPrint("Heap con lai: %d bytes\r\n", xPortGetFreeHeapSize());

  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* Start scheduler */
  osKernelStart();

  /* We should never get here as control is now taken by the scheduler */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
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

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
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
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief CAN Initialization Function
  * @param None
  * @retval None
  */
static void MX_CAN_Init(void)
{

  /* USER CODE BEGIN CAN_Init 0 */

  /* USER CODE END CAN_Init 0 */

  /* USER CODE BEGIN CAN_Init 1 */

  /* USER CODE END CAN_Init 1 */
  hcan.Instance = CAN1;
  hcan.Init.Prescaler = 9;
  hcan.Init.Mode = CAN_MODE_NORMAL;
  hcan.Init.SyncJumpWidth = CAN_SJW_1TQ;
  hcan.Init.TimeSeg1 = CAN_BS1_5TQ;
  hcan.Init.TimeSeg2 = CAN_BS2_2TQ;
  hcan.Init.TimeTriggeredMode = DISABLE;
  hcan.Init.AutoBusOff = DISABLE;
  hcan.Init.AutoWakeUp = DISABLE;
  hcan.Init.AutoRetransmission = DISABLE;
  hcan.Init.ReceiveFifoLocked = DISABLE;
  hcan.Init.TransmitFifoPriority = DISABLE;
  if (HAL_CAN_Init(&hcan) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN CAN_Init 2 */

  /* USER CODE END CAN_Init 2 */

}

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.ClockSpeed = 100000;
  hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

/**
  * @brief SPI1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI1_Init(void)
{

  /* USER CODE BEGIN SPI1_Init 0 */

  /* USER CODE END SPI1_Init 0 */

  /* USER CODE BEGIN SPI1_Init 1 */

  /* USER CODE END SPI1_Init 1 */
  /* SPI1 parameter configuration*/
  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_2LINES;
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi1.Init.NSS = SPI_NSS_SOFT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_4;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 10;
  if (HAL_SPI_Init(&hspi1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI1_Init 2 */

  /* USER CODE END SPI1_Init 2 */

}

/**
  * @brief SPI2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI2_Init(void)
{

  /* USER CODE BEGIN SPI2_Init 0 */

  /* USER CODE END SPI2_Init 0 */

  /* USER CODE BEGIN SPI2_Init 1 */

  /* USER CODE END SPI2_Init 1 */
  /* SPI2 parameter configuration*/
  hspi2.Instance = SPI2;
  hspi2.Init.Mode = SPI_MODE_MASTER;
  hspi2.Init.Direction = SPI_DIRECTION_2LINES;
  hspi2.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi2.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi2.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi2.Init.NSS = SPI_NSS_SOFT;
  hspi2.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_16;
  hspi2.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi2.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi2.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi2.Init.CRCPolynomial = 10;
  if (HAL_SPI_Init(&hspi2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI2_Init 2 */

  /* USER CODE END SPI2_Init 2 */

}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * @brief USART3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART3_UART_Init(void)
{

  /* USER CODE BEGIN USART3_Init 0 */

  /* USER CODE END USART3_Init 0 */

  /* USER CODE BEGIN USART3_Init 1 */

  /* USER CODE END USART3_Init 1 */
  huart3.Instance = USART3;
  huart3.Init.BaudRate = 115200;
  huart3.Init.WordLength = UART_WORDLENGTH_8B;
  huart3.Init.StopBits = UART_STOPBITS_1;
  huart3.Init.Parity = UART_PARITY_NONE;
  huart3.Init.Mode = UART_MODE_TX_RX;
  huart3.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart3.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart3) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART3_Init 2 */

  /* USER CODE END USART3_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_8|GPIO_PIN_9, GPIO_PIN_RESET);

  /*Configure GPIO pin : PC13 */
  GPIO_InitStruct.Pin = GPIO_PIN_13;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : PB0 PB1 PB8 PB9 */
  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_8|GPIO_PIN_9;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */
	
HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET);
  /*Configure GPIO pin : PC13 */
  GPIO_InitStruct.Pin = GPIO_PIN_13;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  // Th�m config CAN pins (b?t bu?c)
  // CAN_RX PA11
  GPIO_InitStruct.Pin = GPIO_PIN_11;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;  // Pull-up cho Rx
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  // CAN_TX PA12
  GPIO_InitStruct.Pin = GPIO_PIN_12;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */
void HAL_CAN_RxFifo0MsgPendingCallback(CAN_HandleTypeDef *hcan1)
{
    HAL_CAN_GetRxMessage(hcan1, CAN_RX_FIFO0, &rxHeader, canRX);
   
    HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
   
    // G?i h�m chuy?n d?i th�ng s? sang decimal
    processRxData();
   
    // Ghi log qua UART (c� th? delay t�)
   // mPrint("RX ID:%03X DLC:%d Data:", rxHeader.StdId, rxHeader.DLC);
//    for(int i = 0; i < rxHeader.DLC; i++){
  //      mPrint("%02X ", canRX[i]);
 //   }
 //   mPrint("\r\n");
}
void processRxData(void)
{
    switch (rxHeader.StdId) {
        case 0x0C:  // Engine RPM
            // RPM = ((byte3 << 8) + byte4) / 4.0 (scale 0.25)
            engine_rpm = ((canRX[3] << 8) | canRX[4]) / 4.0f;
            mPrint("Engine RPM: %.2f\r\n", engine_rpm);
            // So s�nh v� d?
            if (engine_rpm > 2500.0f) {
                mPrint("Warning: High RPM!\r\n");
            }
            break;

        case 0x0D:  // Vehicle Speed
            // Speed = byte0 (km/h)
            vehicle_speed = canRX[0];
            mPrint("Vehicle Speed: %d km/h\r\n", vehicle_speed);
            // So s�nh v� d?
            if (vehicle_speed > 100) {
                mPrint("Warning: Over speed limit!\r\n");
            }
            break;

        case 0x05:  // Engine Coolant Temp
            // Temp = byte0 - 40 (�C)
            coolant_temp = canRX[0] - 40.0f;
            mPrint("Coolant Temp: %.2f C\r\n", coolant_temp);
            // So s�nh v� d?
            if (coolant_temp > 90.0f) {
                mPrint("Warning: Engine overheating!\r\n");
            }
            break;

        default:
            mPrint("Unknown ID: %03X - No conversion\r\n", rxHeader.StdId);
            break;
    }
}
void StartTask02(void const * argument)
{
  /* USER CODE BEGIN Task02 */
	ST7735_Init(&hspi1);
  ST7735_FillScreen(0x0000); // Chỉ xóa màn hình 1 lần duy nhất khi khởi động
  
  char str_buff[30]; // Bộ đệm chứa chuỗi
  
  /* Infinite loop */
  for(;;)
  {
    // B1: Đọc thời gian thực từ DS3231
    
RTC_Read(&dt);
    uint16_t y_pos = 5;          // Căn lề trên 5 pixel
    uint16_t line_height = 25;   // Khoảng cách dòng

    // --- Dòng 1: Tiêu đề ---
    ST7735_DrawString(0, y_pos, "CAN MONITOR", Font_7x10, 0xFFFF, 0x0000);
    y_pos += line_height;

    // --- Dòng 2: Giờ : Phút : Giây ---
    // %02d nghĩa là luôn hiện 2 số (ví dụ: 09:05:00)
    // Thêm khoảng trắng cuối chuỗi để xóa ký tự thừa nếu chuỗi ngắn lại
    sprintf(str_buff, "Time: %02d:%02d:%02d  ", dt.hour, dt.min, dt.second);
    ST7735_DrawString(0, y_pos, str_buff, Font_7x10, 0x07FF, 0x0000); // Màu Cyan
    y_pos += line_height;

    // --- Dòng 3: Ngày / Tháng / Năm ---
    // Năm chỉ đọc 2 số cuối (ví dụ 25), nên thêm "20" đằng trước
    sprintf(str_buff, "Date: %02d/%02d/20%02d ", dt.date, dt.month, dt.year);
    ST7735_DrawString(0, y_pos, str_buff, Font_7x10, 0x07FF, 0x0000); // Màu Cyan
    y_pos += line_height;

    // --- Dòng 4: Tốc độ ---
    sprintf(str_buff, "Speed: %d km/h   ", vehicle_speed); 
    ST7735_DrawString(0, y_pos, str_buff, Font_7x10, 0xFFE0, 0x0000); // Màu Vàng
    y_pos += line_height;

    // --- Dòng 5: Tên ---
    ST7735_DrawString(0, y_pos, "Dev: Khiem", Font_7x10, 0x07E0, 0x0000); // Màu Xanh lá

    osDelay(200); // Cập nhật màn hình 5 lần/giây (đủ mượt cho mắt)
  }
  /* USER CODE END Task02 */
}

void StartTask03(void const * argument)
{
  /* USER CODE BEGIN Task03 */
FATFS fs;
  FIL fil;
  FRESULT fr;
  UINT bw;
  char log_buff[100];
mPrint("Entered Task03!\r\n");
  // Mount thẻ nhớ
  fr = f_mount(&fs, "", 1);

  // --- DEBUG 1: Nếu Mount lỗi, đèn sẽ SÁNG và hệ thống dừng tại đây ---
  if(fr != FR_OK) {
      HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET); // Đèn sáng (Active Low)
      for(;;) { osDelay(100); } // Chết tại chỗ để báo lỗi Mount
  }
  /* Infinite loop */
  for(;;)
  {
// 2. Mở file để ghi
      fr = f_open(&fil, "log.txt", FA_OPEN_ALWAYS | FA_WRITE);
      
      if(fr == FR_OK) {
          // --- THÀNH CÔNG ---
          f_lseek(&fil, f_size(&fil)); // Xuống cuối file
          
          // Format chuỗi log
          sprintf(log_buff, "%02d:%02d:%02d, SPD:%d, RPM:%.0f\n", 
                  dt.hour, dt.min, dt.second, 
                  vehicle_speed, engine_rpm);
          
          f_write(&fil, log_buff, strlen(log_buff), &bw);
          f_close(&fil);
          
          // Debug: Nháy chậm (1s) -> OK
          HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13); 
          osDelay(1000); 
      }
      else {
          // --- LỖI MỞ FILE ---
          // Debug: Nháy cực nhanh (100ms) -> Lỗi File/Thẻ
          HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
          osDelay(100); 
          mPrint("File Open Error: %d\r\n", fr);
      }
  
  /* USER CODE END Task03 */
}
	}
/* USER CODE END 4 */

/* USER CODE BEGIN Header_StartTask01 */
/**
  * @brief  Function implementing the Task01 thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartTask01 */
void StartTask01(void const * argument)
{
  /* USER CODE BEGIN 5 */
	// Biến phục vụ thuật toán phát hiện tai nạn
  int last_speed = 0;
  uint32_t last_can_msg_tick = 0;
  uint32_t crash_potential_tick = 0;
  uint8_t potential_crash = 0; // 0: Bình thường, 1: Nghi vấn
  /* Infinite loop */
  for(;;)
  {
if (HAL_CAN_GetRxFifoFillLevel(&hcan, CAN_RX_FIFO0) > 0) {
        if (HAL_CAN_GetRxMessage(&hcan, CAN_RX_FIFO0, &rxHeader, canRX) == HAL_OK) {
            
            // Cập nhật thời gian nhận tin cuối cùng
            last_can_msg_tick = HAL_GetTick(); 
            
            // Xử lý dữ liệu (Cập nhật vehicle_speed)
            processRxData(); 
            
            // --- THUẬT TOÁN GIA TỐC ---
            // Chỉ kiểm tra khi có sự thay đổi tốc độ
            if(vehicle_speed != last_speed) {
                // Tính độ giảm tốc (Deceleration)
                int delta_v = last_speed - vehicle_speed;
                
                // Nếu đang đi nhanh (>30km/h) mà giảm đột ngột (>20km/h trong 1 lần đọc)
                // Ngưỡng này cần tinh chỉnh tùy thuộc vào tần số gửi CAN của xe
                if(last_speed > 30 && delta_v > 20) {
                    potential_crash = 1; 
                    crash_potential_tick = HAL_GetTick(); // Ghi lại thời điểm nghi vấn
                    mPrint("WARNING: High Deceleration Detected!\r\n");
                }
                
                last_speed = vehicle_speed; // Lưu tốc độ cũ
            }
        }
    }
    
    // 2. XỬ LÝ NGHI VẤN TAI NẠN
    if(potential_crash == 1 && accident_detected == 0) {
        
        // Đợi 3 giây sau cú va chạm để kiểm tra trạng thái cuối cùng
        if(HAL_GetTick() - crash_potential_tick > 3000) {
            
            // Điều kiện xác nhận tai nạn:
            // 1. Xe dừng hẳn (Speed = 0)
            // HOẶC 2. Mất tín hiệu CAN quá 3 giây (Do hỏng ECU hoặc đứt dây)
            uint8_t can_lost = (HAL_GetTick() - last_can_msg_tick > 3000);
            
            if(vehicle_speed == 0 || can_lost) {
                // ---> XÁC NHẬN TAI NẠN <---
                mPrint("ACCIDENT CONFIRMED! Sending Alert...\r\n");
                
                // Gọi hàm gửi SIM (Lưu ý: Hàm này sẽ delay hệ thống vài giây)
                Send_Emergency_Alert();
            }
            else {
                // Nếu sau 3s mà xe vẫn chạy tiếp -> Phanh gấp bình thường, không phải tai nạn
                potential_crash = 0; 
                mPrint("False Alarm: Just hard braking.\r\n");
            }
        }
    }

    // 3. Xử lý RTC (đọc thời gian cho hệ thống)
    // Lưu ý: Task 2 đã đọc rồi, Task 1 không cần đọc liên tục để tránh xung đột I2C
    // RTC_Read(&dt); 
    
    osDelay(100); // Delay ngắn để check CAN liên tục
}
  /* USER CODE END 5 */
}

/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM4 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM4)
  {
    HAL_IncTick();
  }
  /* USER CODE BEGIN Callback 1 */

  /* USER CODE END Callback 1 */
}

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
#ifdef USE_FULL_ASSERT
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
