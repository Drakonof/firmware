#include "i2c.h"

#include "xparameters.h"
#include "xiicps.h"
#include "xil_printf.h"

#define IIC_DEVICE_ID		XPAR_XIICPS_0_DEVICE_ID

#define IIC_SLAVE_ADDR		0x68
#define IIC_SCLK_RATE		100000
#define TEST_BUFFER_SIZE	8

int IicPsMasterPolledExample(u16 DeviceId);
XIicPs Iic;

u8 SendBuffer[TEST_BUFFER_SIZE];
u8 RecvBuffer[TEST_BUFFER_SIZE];

status i2c_init(i2c_handle *p_handle, i2c_inition *init) {
	int Status;

	Status = IicPsMasterPolledExample(p_handle->id);
	if (Status != XST_SUCCESS) {
		xil_printf("IIC Master Polled Example Test Failed\r\n");
		return XST_FAILURE;
	}

		xil_printf("Successfully ran IIC Master Polled Example Test\r\n");
	return XST_SUCCESS;
}

int IicPsMasterPolledExample(u16 DeviceId)
{
	int Status;
	XIicPs_Config *Config;
	int Index;

	Config = XIicPs_LookupConfig(DeviceId);
	if (NULL == Config) {
		return XST_FAILURE;
	}

	Status = XIicPs_CfgInitialize(&Iic, Config, Config->BaseAddress);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	Status = XIicPs_SelfTest(&Iic);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	XIicPs_SetSClk(&Iic, IIC_SCLK_RATE);

	for (Index = 0; Index < TEST_BUFFER_SIZE; Index++) {
		SendBuffer[Index] = (Index % TEST_BUFFER_SIZE);
		RecvBuffer[Index] = 0;
	}

	//SendBuffer[1] = (1<<7);

	Status = XIicPs_MasterSendPolled(&Iic, SendBuffer,
			 TEST_BUFFER_SIZE, IIC_SLAVE_ADDR);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	while (XIicPs_BusIsBusy(&Iic)) {

	}

	SendBuffer[0] = 0;

	while (1) {

		Status = XIicPs_MasterSendPolled(&Iic, SendBuffer,
				 1, IIC_SLAVE_ADDR);
		if (Status != XST_SUCCESS) {
			return XST_FAILURE;
		}

		Status = XIicPs_MasterRecvPolled(&Iic, RecvBuffer,
				  TEST_BUFFER_SIZE, IIC_SLAVE_ADDR);
 		if (Status != XST_SUCCESS) {
			return XST_FAILURE;
		}

	}

	return XST_SUCCESS;
}
