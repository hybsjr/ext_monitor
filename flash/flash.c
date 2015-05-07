

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x.h"


#define FLASH_PAGE_SIZE    ((uint16_t)0x800)


#define BANK1_WRITE_START_ADDR  ((uint32_t)0x0803F800)
#define BANK1_WRITE_END_ADDR    ((uint32_t)0x08040000)


/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/  
uint32_t EraseCounter = 0x00, Address = 0x00;
uint32_t Data = 0x3210ABCD;
uint32_t NbrOfPage = 0x00;
volatile FLASH_Status FLASHStatus = FLASH_COMPLETE;
/**
  * @brief   Main program
  * @param  None
  * @retval None
  */
int flash_write(unsigned int * buf_32bit, int len)
{
  /*!< At this stage the microcontroller clock setting is already configured, 
       this is done through SystemInit() function which is called from startup
       file (startup_stm32f10x_xx.s) before to branch to application main.
       To reconfigure the default setting of SystemInit() function, refer to
       system_stm32f10x.c file
     */     
	int i = 0;
	
/* Porgram FLASH Bank1 ********************************************************/       
  /* Unlock the Flash Bank1 Program Erase controller */
  FLASH_UnlockBank1();

  /* Define the number of page to be erased */
  NbrOfPage = (BANK1_WRITE_END_ADDR - BANK1_WRITE_START_ADDR) / FLASH_PAGE_SIZE;

  /* Clear All pending flags */
  FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR);	

  /* Erase the FLASH pages */
  for(EraseCounter = 0; (EraseCounter < NbrOfPage) && (FLASHStatus == FLASH_COMPLETE); EraseCounter++)
  {
    FLASHStatus = FLASH_ErasePage(BANK1_WRITE_START_ADDR + (FLASH_PAGE_SIZE * EraseCounter));
  }
  
  /* Program Flash Bank1 */
  Address = BANK1_WRITE_START_ADDR;

  while((i < len) && (FLASHStatus == FLASH_COMPLETE))
  {
    FLASHStatus = FLASH_ProgramWord(Address, buf_32bit[i++]);
    Address = Address + 4;
  }

  FLASH_LockBank1();
	return 0;
}

int flash_read(unsigned int * buf_32bit, int len)
{
	
	int i = 0;
	/* Check the correctness of written data */
  Address = BANK1_WRITE_START_ADDR;

  while(i < len)
  {
		buf_32bit[i++] = *(__IO uint32_t*) Address;
    Address += 4;
  }
	return 0;
}

/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/
