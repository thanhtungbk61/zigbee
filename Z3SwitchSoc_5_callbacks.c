
/***************************************************************************//**
 * @file
 * @brief
 *******************************************************************************
 * # License
 * <b>Copyright 2018 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 ******************************************************************************/

// This callback file is created for your convenience. You may add application
// code to this file. If you regenerate this file over a previous version, the
// previous version will be overwritten and any code you have added will be
// lost.

//#include "zigbee-device-common.c"
#include "em_msc.h"
#include "myUart.h"
#include "app/framework/include/af.h"
#include "em_emu.h"
// my include

#include "myInit.h"
#include "em_timer.h"
#include "cJSON.h"
#include "structCommand.h"
#include "stdlib.h"
#include "jsonStruct.h"
#include "myCommand.h"
#include "userMemory.h"

#include "userMemory.h"
// test write and read eeprom
//#define USERDATA ((uint32_t*)USERDATA_BASE)

uint32_t Cleared_value;
uint32_t Set_value,Set_value2;
uint8_t myValue=0;
uint8_t nodeAdd=0;
//
EmberKeyData userKey;
//
#include EMBER_AF_API_NETWORK_CREATOR
#include EMBER_AF_API_NETWORK_CREATOR_SECURITY
#include EMBER_AF_API_NETWORK_STEERING
#include EMBER_AF_API_ZLL_PROFILE
#include EMBER_AF_API_FIND_AND_BIND_INITIATOR

#define SWITCH_ENDPOINT (1)

static bool commissioning = false;


EmberEventControl commissioningEventControl;
EmberEventControl ledEventControl;
EmberEventControl findingAndBindingEventControl;
static uint8_t lastButton;
EmberEventControl myDelay;
#define MY_DELAY_IN_MS 1000
uint8_t statusCallback, status2=99;
uint8_t count=0;
uint8_t count2=0;
uint8_t RxBuffer[100];
uint8_t test_duty=0;

#define IntType 0
#define StrType 1
typedef struct {
   uint16_t addr;
   char name[17];
   char value[8];
} schedule;

schedule convert2schedule(char* in) {
    return *((schedule*)in);
}



void emberAfTrustCenterJoinCallback(EmberNodeId newNodeId,
                                    EmberEUI64 newNodeEui64,
                                    EmberNodeId parentOfNewNode,
                                    EmberDeviceUpdate status,
                                    EmberJoinDecision decision)
{
	int i;
	for(i=0;i<8;i++)
	{
		emberAfCorePrintln("newNodeEui64:%x",newNodeEui64[i]);
	}
	userNewNodeId = newNodeId;
	emberAfCorePrintln("emberAfTrustCenterJoinCallback--newNodeId:%d--newNodeEui64:%d--status:%d",newNodeId,newNodeEui64,status);
	//uint8_t status2 = emberAfPluginNetworkCreatorSecurityCloseNetwork();
	//uint8_t status2 =emberLeaveRequest(userNewNodeId,newNodeEui64,EMBER_ZIGBEE_LEAVE_WITHOUT_REJOIN ,0);
	//uint8_t status2 = emberAfPluginNetworkCreatorSecurityCloseNetwork();
	//emberPermitJoining(0);


}
//void attributeRespon(EmberAfClusterId clusterId,
//									  uint8_t *buffer,
//									  uint16_t bufLen,uint8_t typeRespon)
//{
//	ResponFrame respon ={
//			.MAC = "",
//			.PAN = 0x00, //ProvisonFrameStruct.PAN,
//			.Address =emberGetSender(),   // typpe // endpoint // cmd =1 //atr
//			//.Address =2,
//			.Type = 0,
//			.Endpoint =1,
//			.StatusResponse = buffer[2],  // tru Push event se khong co StatusResponse
//			.NameDevice = "testDevice",
//			.Description = "testDescription",
//			.ProfileID =0x0104,
//			.ClusterID =clusterId,
//			.AttributeID = ((uint16_t)buffer[0]<<8) + (uint16_t)buffer[1],
//			.valueType=1,
//			.strValue=NULL,
//			.intValue=0
//	};
//	if(buffer[2]==0)
//	{
//		respon.intValue =buffer[4];
//	}
//	char * responJ = responJson( respon);
//	//uint8_t cmd =1;
//	emberAfCorePrintln("AttributesResponse:%s",responJ);
//	uartSendRespon(typeRespon,responJ);
//	free(responJ);
//}

void myDelayHandler(void )
{
	// test long message
//	emberFragmentSendUnicast();

	//
	emberEventControlSetInactive(myDelay);
	  EmberStatus status1, status2;

	  status1 =emberAfNetworkState();
	  if (emberAfNetworkState()!= EMBER_JOINED_NETWORK)
	  {
		  emberAfCorePrintln("=emberAfPluginNetworkCreatorNetworkForm--- networkStatus:%d1",status1);
		  emberAfPluginNetworkCreatorNetworkForm(true,
	                                                     15,
	                                                     3,
	                                                     20);
		  emberAfPluginNetworkCreatorSecurityOpenNetwork();
		  //emberAfPluginNetworkCreatorSecurityCloseNetwork();
		  emberEventControlSetDelayMS(myDelay, 5 * MY_DELAY_IN_MS);
	  }
	  else
	  {

		  uint16_t nodeId =emberAfGetNodeId();
		  status2 =emberAfNetworkState();
		//  emberPermitJoining(60);
		  emberEventControlSetDelayMS(myDelay, 100 * MY_DELAY_IN_MS);
		  emberAfCorePrintln("myDelayHandler-- networkstatus:%d--nodeId:%d",status2,nodeId);

	  }

}

void printfSchedule(schedule s, int typeValue){
	emberAfCorePrintln("addr:%d\nname:%s\n", s.addr, s.name);
    if( typeValue == StrType) {
    	emberAfCorePrintln("value:%s\n", s.value);
    }
    if( typeValue == IntType ){
        uint64_t v;
        v = * ((uint64_t*)(s.value));
        emberAfCorePrintln("value:%ld\n", v);
    }
}

void emberAfMainInitCallback(void)
{
	int i;
	// test
//	EmberKeyStruct testKey;
//	emberGetKey(EMBER_CURRENT_NETWORK_KEY,&testKey);
//	for(i=0;i<16;i++)
//	{
//		emberAfCorePrintln("key[i]:%x",testKey.key.contents[i]);
//	}

	//EMBER_KEY_PERMISSIONS_READING_ALLOWED;
	//

//	uint8_t installcode1[18] ={0x88,0x77,0x66,0x55,0x44,0x33,0x22,0x11,0x11,0x22,0x33,0x44,0x55,0x66,0x77,0x88,0xD4,0x90}; // install code
//	emberAfCorePrintln("emberAfMainInitCallback");
//	emAfInstallCodeToKey(installcode1,18,&userKey);
//	for(i=0;i<16;i++)
//		{
//		   emberAfCorePrintln("key:%x",userKey.contents[i]);
//		}


//	here
	//  khoi tao vung nho
	  // Clear the Userdata page of any previous data stored
	//  MSC_ErasePage((uint32_t*)USERDATA_BASE );
	//
	// test Led
	  GPIO_PinModeSet(gpioPortD, 8, gpioModePushPull, 0);
	  GPIO_PinModeSet(gpioPortD, 9, gpioModePushPull, 1);
	 //
	  GPIO_PinOutSet(gpioPortD,8);
	  GPIO_PinOutSet(gpioPortD,9);
	  //------------------init PWM---------------
	  EMU_DCDCInit_TypeDef dcdcInit = EMU_DCDCINIT_DEFAULT;
	  EMU_DCDCInit(&dcdcInit);

	  // Initializations
	  //initGpio();
	  initTimer();
	  //---------------------------

	  USART_InitAsync_TypeDef init = USART_INITASYNC_DEFAULT;
	// Enable oscillator to GPIO and USART3 modules
	  init.baudrate = 9600;
	  CMU_ClockEnable(cmuClock_GPIO, true);
	  CMU_ClockEnable(cmuClock_USART3, true);

	  // set pin modes for USART TX and RX pins
	  GPIO_PinModeSet(gpioPortB, 7, gpioModeInput, 0);
	  GPIO_PinModeSet(gpioPortB, 6, gpioModePushPull, 1);

	  // Initialize USART asynchronous mode and route pins
	  USART_InitAsync(USART3, &init);
	  /* Enable I/O and set location */
	  USART3->ROUTEPEN |= USART_ROUTEPEN_RXPEN | USART_ROUTEPEN_TXPEN;
	  USART3->ROUTELOC0 = (USART3->ROUTELOC0 &
	                       ~(_USART_ROUTELOC0_TXLOC_MASK
	                         | _USART_ROUTELOC0_RXLOC_MASK ))
	                       | (PORTIO_USART3_TX_LOC << _USART_ROUTELOC0_TXLOC_SHIFT)
	                       | (PORTIO_USART3_RX_LOC << _USART_ROUTELOC0_RXLOC_SHIFT);

	  //Initializae USART Interrupts
	  USART_IntEnable(USART3, USART_IEN_RXDATAV);

	  //Enabling USART Interrupts
	  NVIC_EnableIRQ(USART3_RX_IRQn);
	emberEventControlSetDelayMS(myDelay, 10 * MY_DELAY_IN_MS);
}

bool emberAfReadAttributesResponseCallback(EmberAfClusterId clusterId,
                                           uint8_t *buffer,
                                           uint16_t bufLen)
{
	//reportAttributes(clusterId,buffer,bufLen);
	emberAfCorePrintln("emberAfReadAttributesResponseCallback---clusterId:%d----",clusterId);
	for(int i=0;i<bufLen;i++)
	{
		emberAfCorePrintln("%d",buffer[i]);
	}
	reportAttributes(clusterId,buffer,bufLen,GET_COMMAND);
	return  true;
}

void emberAfOnOffClusterClientDefaultResponseCallback(int8u endpoint,
                                                      int8u commandId,
                                                      EmberAfStatus status)
{
	emberAfCorePrintln("emberAfOnOffClusterClientDefaultResponseCallback endpoint:%d  commandId:%d  status:%d",endpoint,commandId,status);
}

// all callback
bool emberAfReportAttributesCallback(EmberAfClusterId clusterId,
                                     uint8_t *buffer,
                                     uint16_t bufLen)
{
	emberAfCorePrintln("emberAfReportAttributesCallback---clusterId:%d----all callback",clusterId);
	for(int i=0;i<bufLen;i++)
	{
		emberAfCorePrintln("%d",buffer[i]);
	}
	reportAttributes(clusterId,buffer,bufLen,REPORT_COMMAND);
	return 1;
}

void emberAfReportingAttributeChangeCallback(int8u endpoint,
                                             EmberAfClusterId clusterId,
                                             EmberAfAttributeId attributeId,
                                             int8u mask,
                                             int16u manufacturerCode,
                                             EmberAfAttributeType type,
                                             int8u *data)
{
	emberAfCorePrintln("emberAfReportingAttributeChangeCallback---");
}


void USART3_RX_IRQHandler(void)
{
	GPIO_PinOutClear(gpioPortD,9);
	  uint32_t flags=0,data;
	  flags = USART_IntGet(USART3);
	  USART_IntClear(USART3, flags);
//	  USART_IntClear(USART3, USART_IEN_RXDATAV);
	  //USART3->IEN &= ~USART_IEN_RXDATAV;
	  data = USART_Rx(USART3);
	  set_data(data);
//      USART_Tx(USART3,data);

}


void USART3_TX_IRQHandler(void)
{
	  uint32_t flags;
	  flags = USART_IntGet(USART3);
	  USART_IntClear(USART3, flags);
}
void commissioningEventHandler(void)
{
  count++;
  emberEventControlSetInactive(commissioningEventControl);

  if (emberAfNetworkState() == EMBER_JOINED_NETWORK) {
  //  emberAfGetCommandApsFrame()->sourceEndpoint = SWITCH_ENDPOINT;
    if (lastButton == BUTTON0) {
    	toggleLight(userNewNodeId);
    	//putOnOffSchedual("0123456789");
//    	levelControl(userNewNodeId,10*count);
    	emberAfCorePrintln("toggleLight--userNewNodeId:%d",userNewNodeId);
    }
    else
    {
    	emberAfCorePrintln("send long message");
    	emberAfPluginNetworkCreatorSecurityOpenNetwork();
    	//putOnOffSchV2("012345678901234567890123456789012345678901234567890123456789012345678901234567890012345678901234567890123456789012345678901234567890123456789",110);
//    	uint8_t payloadLeng =emberMaximumApsPayloadLength();
//    	emberAfCorePrintln("\n\npayloadLeng:%d",payloadLeng);
//    	uint8_t status=255;
//
//    	char schedule1[] = "012345678901234567890123456789012345678901234567890123456789012345678911";
//        status = putOnOffSchedual(schedule1);
//    	if(count%2==0)
//    	{
//    		char schedule1[] = "012345678901234567890123456789012345678901234567890123456789012345678911";
//        	status = putOnOffSchedual(schedule1);
//        	emberAfCorePrintln("\nputOnOffSchedual:%d---send: 72 byte",status);
//    	}
//    	else if(count%3)
//    	{
//    		char schedule1[] = "0123456789012345678901234567890123456789012345678901234567890123456789012345";
//        	status = putOnOffSchedual(schedule1);
//        	emberAfCorePrintln("\nputOnOffSchedual:%d---send: 80 byte",status);
//    	}
//    	else
//    	{
//    		char schedule1[] = "012345678901234567890123456789012345678901234567890123456789012345678901234567890012345678901234567890123456789012345678911";
//        	status = putOnOffSchedual(schedule1);
//        	emberAfCorePrintln("\nputOnOffSchedual:%d---send: 128 byte",status);
//    	}


    }
//      emberAfFillCommandOnOffClusterToggle();
//    } else if (lastButton == BUTTON1) {
//      uint8_t nextLevel = (uint8_t)(0xFF & emberGetPseudoRandomNumber());
//      emberAfFillCommandLevelControlClusterMoveToLevel(nextLevel, TRANSITION_TIME_DS, 0, 0);
//    }
//    status = emberAfSendCommandUnicastToBindings();
//    emberAfCorePrintln("%p: 0x%X", "Send to bindings", status);
  } else {
	  emberAfPluginNetworkCreatorSecurityOpenNetwork();
	  emberAfCorePrintln("OpenNetwork");
//    bool touchlink = (lastButton == BUTTON1);
//    status = (touchlink
//              ? emberAfZllInitiateTouchLink()
//              : emberAfPluginNetworkSteeringStart());
//    emberAfCorePrintln("%p network %p: 0x%X",
//                       (touchlink ? "Touchlink" : "Join"),
//                       "start",
//                       status);
    emberEventControlSetActive(ledEventControl);
    commissioning = true;
  }
}

void ledEventHandler(void)
{
  emberEventControlSetInactive(ledEventControl);

  if (commissioning) {
    if (emberAfNetworkState() != EMBER_JOINED_NETWORK) {
      //halToggleLed(COMMISSIONING_STATUS_LED);
      emberEventControlSetDelayMS(ledEventControl, LED_BLINK_PERIOD_MS << 1);
    } else {
      //halSetLed(COMMISSIONING_STATUS_LED);
    }
  } else if (emberAfNetworkState() == EMBER_JOINED_NETWORK) {
   // halSetLed(COMMISSIONING_STATUS_LED);
  }
}

void findingAndBindingEventHandler(void)
{
  emberEventControlSetInactive(findingAndBindingEventControl);
  EmberStatus status = emberAfPluginFindAndBindInitiatorStart(SWITCH_ENDPOINT);
  emberAfCorePrintln("Find and bind initiator %p: 0x%X", "start", status);
}

static void scheduleFindingAndBindingForInitiator(void)
{
  emberEventControlSetDelayMS(findingAndBindingEventControl,
                              FINDING_AND_BINDING_DELAY_MS);
}

/** @brief Stack Status
 *
 * This function is called by the application framework from the stack status
 * handler.  This callbacks provides applications an opportunity to be notified
 * of changes to the stack status and take appropriate action.  The return code
 * from this callback is ignored by the framework.  The framework will always
 * process the stack status after the callback returns.
 *
 * @param status   Ver.: always
 */
bool emberAfStackStatusCallback(EmberStatus status)
{
  if (status == EMBER_NETWORK_DOWN) {
    halClearLed(COMMISSIONING_STATUS_LED);
  } else if (status == EMBER_NETWORK_UP) {
    halSetLed(COMMISSIONING_STATUS_LED);
  }

  // This value is ignored by the framework.
  return false;
}

/** @brief Hal Button Isr
 *
 * This callback is called by the framework whenever a button is pressed on the
 * device. This callback is called within ISR context.
 *
 * @param button The button which has changed state, either BUTTON0 or BUTTON1
 * as defined in the appropriate BOARD_HEADER.  Ver.: always
 * @param state The new state of the button referenced by the button parameter,
 * either ::BUTTON_PRESSED if the button has been pressed or ::BUTTON_RELEASED
 * if the button has been released.  Ver.: always
 */
void emberAfHalButtonIsrCallback(uint8_t button,
                                 uint8_t state)
{
  if (state == BUTTON_RELEASED) {
    lastButton = button;
    emberEventControlSetActive(commissioningEventControl);
  }
}

/** @brief Complete
 *
 * This callback is fired when the Network Steering plugin is complete.
 *
 * @param status On success this will be set to EMBER_SUCCESS to indicate a
 * network was joined successfully. On failure this will be the status code of
 * the last join or scan attempt. Ver.: always
 * @param totalBeacons The total number of 802.15.4 beacons that were heard,
 * including beacons from different devices with the same PAN ID. Ver.: always
 * @param joinAttempts The number of join attempts that were made to get onto
 * an open Zigbee network. Ver.: always
 * @param finalState The finishing state of the network steering process. From
 * this, one is able to tell on which channel mask and with which key the
 * process was complete. Ver.: always
 */
void emberAfPluginNetworkSteeringCompleteCallback(EmberStatus status,
                                                  uint8_t totalBeacons,
                                                  uint8_t joinAttempts,
                                                  uint8_t finalState)
{
  emberAfCorePrintln("%p network %p: 0x%X", "Join", "complete", status);

  if (status != EMBER_SUCCESS) {
    commissioning = false;
  } else {
    scheduleFindingAndBindingForInitiator();
  }
}

/** @brief Touch Link Complete
 *
 * This function is called by the ZLL Commissioning Common plugin when touch linking
 * completes.
 *
 * @param networkInfo The ZigBee and ZLL-specific information about the network
 * and target. Ver.: always
 * @param deviceInformationRecordCount The number of sub-device information
 * records for the target. Ver.: always
 * @param deviceInformationRecordList The list of sub-device information
 * records for the target. Ver.: always
 */
void emberAfPluginZllCommissioningCommonTouchLinkCompleteCallback(const EmberZllNetwork *networkInfo,
                                                                  uint8_t deviceInformationRecordCount,
                                                                  const EmberZllDeviceInfoRecord *deviceInformationRecordList)
{
  emberAfCorePrintln("%p network %p: 0x%X",
                     "Touchlink",
                     "complete",
                     EMBER_SUCCESS);

  scheduleFindingAndBindingForInitiator();
}

/** @brief Touch Link Failed
 *
 * This function is called by the ZLL Commissioning Client plugin if touch linking
 * fails.
 *
 * @param status The reason the touch link failed. Ver.: always
 */
void emberAfPluginZllCommissioningClientTouchLinkFailedCallback(EmberAfZllCommissioningStatus status)
{
  emberAfCorePrintln("%p network %p: 0x%X",
                     "Touchlink",
                     "complete",
                     EMBER_ERR_FATAL);

  commissioning = false;
}

/** @brief Complete
 *
 * This callback is fired by the initiator when the Find and Bind process is
 * complete.
 *
 * @param status Status code describing the completion of the find and bind
 * process Ver.: always
 */
void emberAfPluginFindAndBindInitiatorCompleteCallback(EmberStatus status)
{
  emberAfCorePrintln("Find and bind initiator %p: 0x%X", "complete", status);
  //emberLeaveRequest();
  commissioning = false;
}

// tet holiday test


void emberAfPluginDeviceDatabaseDiscoveryCompleteCallback(const EmberAfDeviceInfo*device)
{

	// test
	userNewNodeId =emberGetSender();
	emberAfCorePrintln("\n\nemberAfPluginDeviceDatabaseDiscoveryCompleteCallback  EmberEUI64:%d, nodeId:%d ,EmberAfDeviceDiscoveryStatus:%d",device->eui64,emberGetSender(),device->status);
	emberAfCorePrintln("clusterId0:%d, endpoint0:%d----clusterId1:%d, endpoint1:%d",device->endpoints[0].clusters[0].clusterId,device->endpoints[0].endpoint,device->endpoints[1].clusters[0].clusterId,device->endpoints[1].endpoint);
	//uint8_t status2 =emberLeaveRequest(userNewNodeId,device->eui64,EMBER_ZIGBEE_LEAVE_WITHOUT_REJOIN ,0);
	//uint8_t status2 = emberAfPluginNetworkCreatorSecurityCloseNetwork();
	//emberAfCorePrintln("emberLeaveRequest:%d",status2);
	// prepare for netDevice
	netDeviceStruct netD;
	netD.address =emberGetSender();
	netD.endpoint =0;  // default endpoint
	netD.type =0;       // unicast

	packet p ={0,0,NULL,NULL,NULL,NULL,0,NULL,0,NULL};
	p.netDevice = strdup(createDeviceJson(netD));
	// hard code
	p.content =strdup("add device");
	p.statusMessage= strdup("successful");
	p.netReadingsCount =0;

	//
	p.cmd =ADD_COMMAND;
	p.statusCode =device->status;
	p.MAC = createEUI64Str(device->eui64);
	p.linkKey = createInstallcodeStr(addDeviceKey);
	char* respondP = createRespondPacket(p);
	uartSendRespon(respondP);
	free(respondP);
	freePacket(p);
}

//typedef struct {
//  EmberEUI64 eui64;
//  EmberAfEndpointInfoStruct endpoints[EMBER_AF_MAX_ENDPOINTS_PER_DEVICE];
//  EmberAfDeviceDiscoveryStatus status;
//  uint8_t discoveryFailures;
//  uint8_t capabilities;
//  uint8_t endpointCount;
//  uint8_t stackRevision;
//} EmberAfDeviceInfo;
//void emberAfPluginDeviceDatabaseDiscoveryCompleteCallback(const EmberAfDeviceInfo*device)
//{
//	emberAfCorePrintln("emberAfPluginDeviceDatabaseDiscoveryCompleteCallback");
//}

//void emberAfPluginDeviceTableNewDeviceCallback(EmberEUI64 eui64)
//{
//	emberAfCorePrintln("emberAfPluginDeviceTableNewDeviceCallback");
//}
//
bool emberAfPluginNetworkFindJoinCallback(EmberZigbeeNetwork *networkFound,
                                          uint8_t lqi,
                                          int8_t rssi)
{
	emberAfCorePrintln("emberAfPluginDeviceTableNewDeviceCallback");
	return true;
}

void emberAfGroupsClusterClientDefaultResponseCallback(int8u endpoint,
                                                       int8u commandId,
                                                       EmberAfStatus status)
{
	emberAfCorePrintln("emberAfGroupsClusterClientDefaultResponseCallback--stt:%d",status);
}


//void emberAfLevelControlClusterClientAttributeChangedCallback(int8u endpoint,
//                                                              EmberAfAttributeId attributeId)
//{
//	emberAfCorePrintln("emberAfLevelControlClusterClientAttributeChangedCallback");
//}
void emberAfLevelControlClusterClientDefaultResponseCallback(int8u endpoint,
                                                             int8u commandId,
                                                             EmberAfStatus status)
{
	emberAfCorePrintln("emberAfLevelControlClusterClientDefaultResponseCallback--status:%d",status);
}

void emberAfPluginOtaServerUpdateCompleteCallback(uint16_t manufacturerId,
                                                  uint16_t imageTypeId,
                                                  uint32_t firmwareVersion,
                                                  EmberNodeId source,
                                                  uint8_t status)
{
	emberAfCorePrintln("emberAfPluginOtaServerUpdateCompleteCallback");
}
// PWM
//emberEventControlSetInactive(myDelay);
//GPIO_PinOutToggle(gpioPortD,8);
//GPIO_PinOutSet(gpioPortD,9);
////	USART_Tx(USART3,'6');
////	char *jsonTest =create_monitor()
//test_duty +=10;
//if(test_duty== 100)
//{
//	test_duty=0;
//}
//TIMER_CompareBufSet(TIMER1, TIMER_CHANNEL, test_duty);
////emberAfCorePrintln("----myDelayHandler---myDelayHandler----:%s",jsonTest);
//emberAfCorePrintln("----myDelayHandler-");
//emberEventControlSetDelayMS(myDelay, 10 * MY_DELAY_IN_MS);

//
//
//char test[] = {0x01, 0x00,
//            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x12, 0x34};
//emberAfCorePrintln("Hello World\n");
//schedule s = convert2schedule(test);
//printfSchedule(s, IntType);
/*
if(count ==1)
{
	flash2 = *((data_flash*)(USERDATA + 4));
	emberAfCorePrintln("sizeofDataFlash:x%d--y:%d",flash2.x,flash2.y);
}
*
if(count ==2)
{			flashTest.x =15;
			flashTest.y =15;
		  MSC_Init();
		  MSC_WriteWord(((uint32_t*)USERDATA_BASE + 4),&flashTest,sizeof(data_flash));
		  MSC_Deinit();
		  flash2 = *((data_flash*)(USERDATA + 4));

	emberAfCorePrintln("sizeofDataFlash:x%d--y:%d",flash2.x,flash2.y);
}
if(count ==3)
{
			flashTest.x =240;           // fo
			flashTest.y =61;
		  MSC_Init();
		  MSC_WriteWord(((uint32_t*)USERDATA_BASE + 4),&flashTest,sizeof(data_flash));
		  MSC_Deinit();
		  flash2 = *((data_flash*)(USERDATA + 4));
	emberAfCorePrintln("sizeofDataFlash:x%d--y:%d",flash2.x,flash2.y);
}
*/
// test get command
//CommandFrame frame1 =
//{
//		.ProfileID =260,    //emberAfProfileIdFromIndex(0);
//		.ClusterID =6,
//		.Endpoint=1,
//		.Address= 2
//};
//
//
//CommandFrame frame2 =
//{
//		.strValue= "123456789123456780123456789012345678901",
//		.ProfileID =260,    //emberAfProfileIdFromIndex(0);
//		.ClusterID =0xFC10,
//		.Endpoint=1,
//		.Address= 2
//};
//
//uint8_t status=1;
//if(count%2==0)
//{
//	status= readAttributeLight(frame1);
//	emberAfCorePrintln("readAttributeLight");
//}
//else
//{
//	status= setSubcriber(frame2);
//	emberAfCorePrintln("setSubcriber");
//}


//
//	  data_flash a={1,7}, b ={2,8}, c= {3,9};
//	  writeData(a);
//	  writeData(b);
//	  writeData(c);
//	  for(uint8_t i=0;i<3;i++)
//	  {
//		  emberAfCorePrintln("-before delete data.x:%d--",ramSchData[i].x);
//	  }
//	  deleteData(b.x);
//	  for(uint8_t i=0;i<3;i++)
//	  {
//		  emberAfCorePrintln("-after delete data.x:%d--",ramSchData[i].x);
//	  }
//
//	  data_flash read = readSchRam(3);
//	  emberAfCorePrintln("--read--x:%d--y:%d",read.x,read.y);


// test key
//		  EmberKeyStruct testKey;
//			emberGetKey(EMBER_CURRENT_NETWORK_KEY,&testKey);
//			for(i=0;i<16;i++)
//			{
//				emberAfCorePrintln("key[i]:%x",testKey.key.contents[i]);
//			}


//
//    	int i;
//    	for(i=0;i<16;i++)
//    		{
//    		   emberAfCorePrintln("key:%x",userKey.contents[i]);
//    		}
    	//  key 88776655443322111122334455667788
    	//status = emAfInstallCodeToKey(code, 18, &key);
//    	EmberEUI64 eui64 ={0xE1,0xF2,0xD8,0xFE,0xFF,0x57,0x0B,0x00};  //E1F2D8FEFF570B00  //000B57FFFED8F2E1
//    	EmberEUI64 eui642 ={0x00,0x0B,0x57,0xFF,0xFE,0xD8,0xF2,0xE1};
//    	EmberEUI64 data ={0xE1,0xF2,0xD8,0xFE,0xFF,0x57,0x0B,0x00};
//    	EmberEUI64 key2 ={0x00,0x0B,0x57,0xFF,0xFE,0xD8,0xF2,0xE1};
//    	EmberKeyData data2 ={0x88,0x77,0x66,0x55,0x44,0x33,0x22,0x11,0x11,0x22,0x33,0x44,0x55,0x66,0x77,0x88};
//    	uint8_t eui64[8] ={0xE1,0xF2,0xD8,0xFE,0xFF,0x57,0x0B,0x00};
//    	uint8_t   key[16] ={0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};

//    	int status = emberAfPluginNetworkCreatorSecurityOpenNetworkWithKeyPair(eui64,userKey); //0x000B57FFFED8F2E1
//    	emberAfCorePrintln("OpenNetworkWithKeyPair:%d",status);
//    	count2++;
//    	count++;
//    	EmberStatus status;
//    	switch(count2)
//    	{
//    	case 1:
//    		status =sendAddGroupCommand1(userNewNodeId,count);
//    		emberAfCorePrintln("sendAddGroupCommand1-stt:%d--count:%d",status,count2);
//    		break;
//    	case 2:
//    		emberAfCorePrintln("levelControl--count:%d---count2:%d",count,count2);
//    		levelControl(userNewNodeId,count);
//    		break;
//    	case 3:
//    	  	  emberAfPluginNetworkCreatorSecurityOpenNetwork();
//    	  	//emberPermitJoining(0);
//    	  	  emberAfCorePrintln("OpenNetwork:%d",count2);
//    	  	  count2=0;
//    	  	  break;
//    	}
