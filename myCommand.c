/*
 * myCommand.c
 *
 *  Created on: Dec 3, 2019
 *      Author: who_you_are
 */
#include "myCommand.h"


//#include "af.h"


//  v2


// handling GateWay Command
//{"cmd":1,"dev":"00000001","evt":[{"r":"123402","t":1,"v":"AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"}]
//                                 ,"sc":0,"sm":""}
void fix32bit(char* data,uint8_t length)
{
    char endian[length];
#ifdef BIGENDIAN
    for (int i = 0; i < length; i++)
    {
        endian[i] = data[i];
    }
#else
    for (int i = 0; i < length; i++)
    {
        endian[i] = data[length - 1 - i];
    }
#endif
    strcpy(data,endian);
}
uint8_t putCommand()
{
	static uint8_t level;
	level+=10;
	if(level==100)
	{
		level=0;
	}
	emberAfCorePrintln("putCommand");
	uint16_t putValue;
	uint8_t status = 255;
	char* value;
	int valueLeng;
	netDeviceStruct netD= getDeviceStruct(receivePacket.netDevice);
	emberAfCorePrintln("getDeviceStruct");
	netResourceStruct netR;
	for(int i=0;i<receivePacket.netReadingsCount;i++)
	{
		netR = getResourceStruct(receivePacket.netReadings[i].netResource);
		emberAfCorePrintln("netR:%d",i);
		switch(netR.cluster)
		{
		case ZCL_ON_OFF_CLUSTER_ID:
			value = base64_decode(receivePacket.netReadings[i].netValue,&valueLeng);
			status=onOffLight(netD,netR,*(uint8_t*)value);
			emberAfCorePrintln("ZCL_ON_OFF_CLUSTER_ID:%d",*(uint8_t*)value);
			break;
		case ZCL_LEVEL_CONTROL_CLUSTER_ID:

			value = base64_decode(receivePacket.netReadings[i].netValue,&valueLeng);
			fix32bit(value,valueLeng);
			//status=onOffLight(netD,netR,*(uint8_t*)value);

			uint16_t value2 = *(uint16_t*)value;
			status = levelControl(netD,netR,(uint8_t)value2);
			emberAfCorePrintln("ZCL_LEVEL_CONTROL_CLUSTER_ID:%d%x--status:%d",value2,value2,status);
			break;
		case ZCL_MANAGER_ID:
			switch(netR.attribute)
			{
			case ZCL_OnOffSchedule_ATTRIBUTE_ID:
				value = base64_decode(receivePacket.netReadings[i].netValue,&valueLeng);
				putOnOffSchV2(value,valueLeng);
				emberAfCorePrintln("ZCL_OnOffSchedule_ATTRIBUTE_ID--value");
				free(value);
				break;
			case ZCL_Group_ATTRIBUTE_ID:
				break;
				//
			case ZCL_Realtime_ATTRIBUTE_ID:
				value = base64_decode(receivePacket.netReadings[i].netValue,&valueLeng);
				fix32bit(value,valueLeng);
				//status=onOffLight(netD,netR,*(uint8_t*)value);

				uint64_t value2 = *(uint64_t*)value;
				status = putRealtime(netD,netR,(uint64_t)value2);
				emberAfCorePrintln("ZCL_Realtime_ATTRIBUTE_ID:%d",value2);

				break;
			default:
				break;
			}
			break;
		default:
			break;
		}
	}
//	receivePacket.cmd = status;
//	char* respond= createRespondPacket(receivePacket);
//	uartSendRespon(respond);
//	free(respond);
	return status;
}
uint8_t getCommand()
{
	uint8_t status = 255;
	netDeviceStruct netD= getDeviceStruct(receivePacket.netDevice);
	netResourceStruct netR[receivePacket.netReadingsCount];
	for(int i=0;i<receivePacket.netReadingsCount;i++)
	{
		netR[i] = getResourceStruct(receivePacket.netReadings[i].netResource);
		switch(netR[i].cluster)
		{
			case ZCL_ON_OFF_CLUSTER_ID:
				emberAfCorePrintln("");
				status = readAttributeLight(netD,netR[i]);
				break;
			case ZCL_LEVEL_CONTROL_CLUSTER_ID:
				status = readDimmingAttributeLight(netD,netR[i]);
				break;
			case ZCL_MANAGER_ID:
				switch(netR[i].attribute)
				{
					default:
						break;
				}
				break;
			default:
				break;
		}
	}
	return status;
}
uint8_t reportCommand()
{
	return 1;
}
uint8_t addCommand()
{
	uint8_t status;

	emAfInstallCodeToKey(receiveInstallCode,18,&addDeviceKey);
	// debug
//	for(int i=0;i<EMBER_ENCRYPTION_KEY_SIZE;i++)
//		{
//		   emberAfCorePrintln("key:%x",addDeviceKey[i].contents[i]);
//		}
	status =emberAfPluginNetworkCreatorSecurityOpenNetworkWithKeyPair(receiveEUI64,addDeviceKey);
	return status;
}
uint8_t deleteCommand()
{
	return 1;
}

// report
void reportAttributes(EmberAfClusterId clusterId,
                                     uint8_t *buffer,
                                     uint16_t bufLen,
									 uint8_t typeCmd)
{
	packet p ={0,0,"","","","",0,"",0,""};
	p.cmd =typeCmd;
	p.statusCode =0;

	// can tao json:  netReading;  netDeviceStruct   netResourceStruct
	// trong do netResourceStruct thuoc netReading
	p.netReadingsCount =1;
	netDeviceStruct netD;
	netResourceStruct netR;

	netD.address = emberGetSender();
	netD.endpoint = 1; // default endpoint
	netD.type =0;      // default type
	p.netDevice =createDeviceJson(netD);

	netR.attribute = ((uint16_t)buffer[0]<<8) + (uint16_t)buffer[1];
	netR.cluster = clusterId;
	netR.profile = 0x0104;   // default profile
	p.netReadings = (netReading*)malloc(1*sizeof(netReading));
	p.netReadings[0].netResource =createResourceJson(netR);
	p.netReadings[0].netValueType = buffer[2];
	char value[2] ={buffer[4], 0};
	int len;
	if(netR.attribute==ZCL_LEVEL_CONTROL_CLUSTER_ID)
	{
		len = 2;
	}
	else
	{
		len = 1;
	}

	//itoa(buffer[4],value,10);
	int valueLength;
	char* encodeValue =base64_encode((const unsigned char *)value,len,&valueLength);
	emberAfCorePrintln("value%d,%s,%d",value[0],encodeValue,valueLength);
		p.netReadings[0].netValue = strdup(encodeValue);



	char* respond = createRespondPacket(p);
	uartSendRespon(respond);
	free(respond);
	if(encodeValue!=NULL)
	{
		free(encodeValue);
	}
}
//
void putRespondCommand(packet p,uint8_t status)
{
	//char respond[200];
	p.cmd =PUT_COMMAND;
	p.statusCode =status;
	char* respond =createRespondPacket(p);
	uartSendRespon(respond);
	free(respond);
}

void getRespondCommand(packet p,uint8_t status)
{
	p.cmd =GET_COMMAND;
	p.statusCode =status;
	char* respond =createRespondPacket(p);
	uartSendRespon(respond);
	free(respond);
}
void reportRespondCommand(packet p,netReading* net, uint8_t status)
{

}
void addRespondCommand(packet p,uint8_t status) // maybe need linkey
{
	p.cmd =ADD_COMMAND;
	p.statusCode =status;
//	createRespondPacket(p,respond);
//	uartSendRespon(respond);
}
void deleteRespondCommand(packet p,uint8_t status)
{
	p.cmd =DELETE_COMMAND;
	p.statusCode =status;
//	createRespondPacket(p,respond);
//	uartSendRespon(respond);
}
/*
 *
 * 	EmberStatus  test2;
	//emberAfCorePrintln("---testSendCommandUnicast--final: %d",test2);
	emberAfFillCommandOnOffClusterToggle();
	emAfCommandApsFrame->profileId=emberAfProfileIdFromIndex(0);
//	emAfCommandApsFrame->clusterId =0x0006;
	emAfCommandApsFrame->sourceEndpoint	=emberAfEndpointFromIndex(0);
	emAfCommandApsFrame->destinationEndpoint =0x0001;

	test2 =emberAfSendCommandUnicast(EMBER_OUTGOING_DIRECT,address);
	emberAfCorePrintln("---sendOnOffCommand--final: %d--address:%d",test2,address);
 */
uint8_t onOffLight(netDeviceStruct netD, netResourceStruct netR,uint8_t value)
{
	//uint8_t sendType;
	uint8_t status;
	if(value==0)
	{
		emberAfFillCommandOnOffClusterOff();  //(int)
	}
	else
	{
		emberAfFillCommandOnOffClusterOn();
	}
	emAfCommandApsFrame->profileId=netR.profile  ;//emberAfProfileIdFromIndex(0);
	emAfCommandApsFrame->clusterId =netR.cluster;
	emAfCommandApsFrame->sourceEndpoint	=emberAfEndpointFromIndex(0);
	emAfCommandApsFrame->destinationEndpoint =netD.endpoint;
	if(netD.type==EMBER_OUTGOING_DIRECT)
	{
		status =emberAfSendCommandUnicast(EMBER_OUTGOING_DIRECT,netD.address);

	}
	else
	{
		status =emberAfSendCommandMulticast(netD.address);
	}
		return status;
}

uint8_t putRealtime(netDeviceStruct netD, netResourceStruct netR,uint64_t value)
{
	//uint8_t sendType;
	uint8_t status;
	emberAfFillCommandManagerPutReportRealTime(value);
	emAfCommandApsFrame->profileId=netR.profile  ;//emberAfProfileIdFromIndex(0);
	emAfCommandApsFrame->clusterId =netR.cluster;
	emAfCommandApsFrame->sourceEndpoint	=emberAfEndpointFromIndex(0);
	emAfCommandApsFrame->destinationEndpoint =netD.endpoint;
	if(netD.type==EMBER_OUTGOING_DIRECT)
	{
		status =emberAfSendCommandUnicast(EMBER_OUTGOING_DIRECT,netD.address);

	}
	else
	{
		status =emberAfSendCommandMulticast(netD.address);
	}
		return status;
}
uint8_t putDimming(netDeviceStruct netD, netResourceStruct netR,uint16_t value)
{
	//uint8_t sendType;
	uint8_t status;
	emberAfFillCommandManagerPutDimming(value);
	emAfCommandApsFrame->profileId=netR.profile  ;//emberAfProfileIdFromIndex(0);
	emAfCommandApsFrame->clusterId =netR.cluster;
	emAfCommandApsFrame->sourceEndpoint	=emberAfEndpointFromIndex(0);
	emAfCommandApsFrame->destinationEndpoint =netD.endpoint;
	if(netD.type==EMBER_OUTGOING_DIRECT)
	{
		status =emberAfSendCommandUnicast(EMBER_OUTGOING_DIRECT,netD.address);

	}
	else
	{
		status =emberAfSendCommandMulticast(netD.address);
	}
		return status;
}


uint8_t onLight1()
{
	emberAfFillCommandOnOffClusterOn();
		emAfCommandApsFrame->profileId=emberAfProfileIdFromIndex(0);
		emAfCommandApsFrame->clusterId =0x0006;
		emAfCommandApsFrame->sourceEndpoint	=emberAfEndpointFromIndex(0);
		emAfCommandApsFrame->destinationEndpoint =0x0001;
	uint8_t status =emberAfSendCommandUnicast(EMBER_OUTGOING_DIRECT,0x0002);
	return status;
}

uint8_t offLight1()
{
	emberAfFillCommandOnOffClusterOff();
	emAfCommandApsFrame->profileId=emberAfProfileIdFromIndex(0);
	emAfCommandApsFrame->clusterId =0x0006;
	emAfCommandApsFrame->sourceEndpoint	=emberAfEndpointFromIndex(0);
	emAfCommandApsFrame->destinationEndpoint =0x0001;
uint8_t status =emberAfSendCommandUnicast(EMBER_OUTGOING_DIRECT,0x0002);
	return status;
}
// v2
uint8_t readAttributeLight(netDeviceStruct netD, netResourceStruct netR)
{
	uint8_t attributeId[2] ={0,0};
	emberAfFillCommandGlobalClientToServerReadAttributes(0x0006,attributeId,2);
	emAfCommandApsFrame->profileId=netR.profile;//emberAfProfileIdFromIndex(0);
	emAfCommandApsFrame->sourceEndpoint	=netD.endpoint;
	emAfCommandApsFrame->destinationEndpoint =1;   // default endpoint
	uint8_t status =emberAfSendCommandUnicast(EMBER_OUTGOING_DIRECT,netD.address);  // EMBER_OUTGOING_DIRECT =netD.type
	return status;
}

uint8_t readDimmingAttributeLight(netDeviceStruct netD, netResourceStruct netR)
{
	uint8_t attributeId[2] ={0,0};
	emberAfFillCommandGlobalClientToServerReadAttributes(0x0008,attributeId,2);
	emAfCommandApsFrame->profileId=netR.profile;//emberAfProfileIdFromIndex(0);
	emAfCommandApsFrame->sourceEndpoint	=netD.endpoint;
	emAfCommandApsFrame->destinationEndpoint =1;   // default endpoint
	uint8_t status =emberAfSendCommandUnicast(EMBER_OUTGOING_DIRECT,netD.address);  // EMBER_OUTGOING_DIRECT =netD.type
	return status;
}
void toggleLight(uint16_t address)
{
	EmberStatus  test2;
	//emberAfCorePrintln("---testSendCommandUnicast--final: %d",test2);
	emberAfFillCommandOnOffClusterToggle();
	emAfCommandApsFrame->profileId=emberAfProfileIdFromIndex(0);
//	emAfCommandApsFrame->clusterId =0x0006;
	emAfCommandApsFrame->sourceEndpoint	=emberAfEndpointFromIndex(0);
	emAfCommandApsFrame->destinationEndpoint =0x0001;

	test2 =emberAfSendCommandUnicast(EMBER_OUTGOING_DIRECT,address);
	emberAfCorePrintln("---sendOnOffCommand--final: %d--address:%d",test2,address);
}

uint8_t levelControl(netDeviceStruct netD, netResourceStruct netR,uint8_t value)
{
	EmberStatus  status;
	emberAfCorePrintln("level value",value);
	//emberAfCorePrintln("---testSendCommandUnicast--final: %d",test2);
//	emberAfFillCommandLevelControlClusterMove();
	emberAfFillCommandLevelControlClusterMoveToLevel(value, 10, 0, 0);
	emAfCommandApsFrame->profileId=netR.profile  ;//emberAfProfileIdFromIndex(0);
	emAfCommandApsFrame->clusterId =netR.cluster;
	emAfCommandApsFrame->sourceEndpoint	=emberAfEndpointFromIndex(0);
	emAfCommandApsFrame->destinationEndpoint =netD.endpoint;
	if(netD.type==EMBER_OUTGOING_DIRECT)
	{
		status =emberAfSendCommandUnicast(EMBER_OUTGOING_DIRECT,netD.address);

	}
	else
	{
		status =emberAfSendCommandMulticast(netD.address);
	}
		return status;
//	emAfCommandApsFrame->profileId=emberAfProfileIdFromIndex(0);
////	emAfCommandApsFrame->clusterId =0x0006;
//	emAfCommandApsFrame->sourceEndpoint	=emberAfEndpointFromIndex(0);
//	emAfCommandApsFrame->destinationEndpoint =0x0001;
//
//	test2 =emberAfSendCommandUnicast(EMBER_OUTGOING_DIRECT,address);
//	emberAfCorePrintln("---levelControl--final: %d--address:%d",test2,address);
}

// "2432434"

uint8_t readAttributeLightOntime()
{
	uint8_t attributeId[2] ={0x40,0x01};
	emberAfFillCommandGlobalClientToServerReadAttributes(0x0006,attributeId,2);
	emAfCommandApsFrame->profileId=emberAfProfileIdFromIndex(0);
	emAfCommandApsFrame->clusterId =0x0006;
	emAfCommandApsFrame->sourceEndpoint	=emberAfEndpointFromIndex(0);
	emAfCommandApsFrame->destinationEndpoint =0x0001;
uint8_t status =emberAfSendCommandUnicast(EMBER_OUTGOING_DIRECT,0x0002);
	return status;
}

uint8_t readAttributeLightStartUP()
{
	uint8_t attributeId[2] ={0x40,0x03};
	emberAfFillCommandGlobalClientToServerReadAttributes(0x0006,attributeId,2);
	emAfCommandApsFrame->profileId=emberAfProfileIdFromIndex(0);
	emAfCommandApsFrame->clusterId =0x0006;
	emAfCommandApsFrame->sourceEndpoint	=emberAfEndpointFromIndex(0);
	emAfCommandApsFrame->destinationEndpoint =0x0001;
uint8_t status =emberAfSendCommandUnicast(EMBER_OUTGOING_DIRECT,0x0002);
	return status;
}

uint8_t readAttributeLightOffWaitTime()
{
	uint8_t attributeId[2] ={0x40,0x02};
	emberAfFillCommandGlobalClientToServerReadAttributes(0x0006,attributeId,2);
	emAfCommandApsFrame->profileId=emberAfProfileIdFromIndex(0);
	emAfCommandApsFrame->clusterId =0x0006;
	emAfCommandApsFrame->sourceEndpoint	=emberAfEndpointFromIndex(0);
	emAfCommandApsFrame->destinationEndpoint =0x0001;
uint8_t status =emberAfSendCommandUnicast(EMBER_OUTGOING_DIRECT,0x0002);
	return status;
}

uint8_t readAttributeLightGlobal()
{
	uint8_t attributeId[2] ={0x40,0x00};
	emberAfFillCommandGlobalClientToServerReadAttributes(0x0006,attributeId,2);
	emAfCommandApsFrame->profileId=emberAfProfileIdFromIndex(0);
	emAfCommandApsFrame->clusterId =0x0006;
	emAfCommandApsFrame->sourceEndpoint	=emberAfEndpointFromIndex(0);
	emAfCommandApsFrame->destinationEndpoint =0x0001;
uint8_t status =emberAfSendCommandUnicast(EMBER_OUTGOING_DIRECT,0x0002);
	return status;
}

//
//void sendOnOffCommandGroup()
//{
//	EmberStatus  test2;
//	//emberAfCorePrintln("---testSendCommandUnicast--final: %d",test2);
//	emberAfFillCommandOnOffClusterToggle();
//	emAfCommandApsFrame->profileId=emberAfProfileIdFromIndex(0);
//	//emAfCommandApsFrame->clusterId =0x0006;
//	emAfCommandApsFrame->sourceEndpoint	=emberAfEndpointFromIndex(0);
//	//emAfCommandApsFrame->destinationEndpoint =0x0001;
//
//	test2 =emberAfSendCommandMulticast(0x0001);
//	emberAfCorePrintln("---sendOnOffCommandGroup--final: %d",test2);
//}
//
//void sendDeleteSceniorCommand()
//{
//	EmberStatus  test2;
//	//emberAfCorePrintln("---testSendCommandUnicast--final: %d",test2);
//	emberAfFillCommandOnOffClusterDeleteScenario("ABC");
//	emAfCommandApsFrame->profileId=emberAfProfileIdFromIndex(0);
//	emAfCommandApsFrame->clusterId =0x0006;
//	emAfCommandApsFrame->sourceEndpoint	=emberAfEndpointFromIndex(0);
//	emAfCommandApsFrame->destinationEndpoint =0x0001;
//
//	test2 =emberAfSendCommandUnicast(EMBER_OUTGOING_DIRECT,0x0002);
//	emberAfCorePrintln("---sendReadResourceCommand----final: %d",test2);
//}
//
//void sendReadResourceCommand(uint32_t data)
//{
//	EmberStatus  test2;
//	//emberAfCorePrintln("---testSendCommandUnicast--final: %d",test2);
//	emberAfFillCommandOnOffClusterWriteResource(data);
//	emAfCommandApsFrame->profileId=emberAfProfileIdFromIndex(0);
//	emAfCommandApsFrame->clusterId =0x0006;
//	emAfCommandApsFrame->sourceEndpoint	=emberAfEndpointFromIndex(0);
//	emAfCommandApsFrame->destinationEndpoint =0x0001;
//
//	test2 =emberAfSendCommandUnicast(EMBER_OUTGOING_DIRECT,0x0002);
//	emberAfCorePrintln("---sendWriteResourceCommand--final: %d",test2);
//}
//
//void sendWriteResourceCommand(uint32_t data)
//{
//	EmberStatus  test2;
//	//emberAfCorePrintln("---testSendCommandUnicast--final: %d",test2);
//	emberAfFillCommandOnOffClusterWriteResource(data);
//	emAfCommandApsFrame->profileId=emberAfProfileIdFromIndex(0);
//	emAfCommandApsFrame->clusterId =0x0006;
//	emAfCommandApsFrame->sourceEndpoint	=emberAfEndpointFromIndex(0);
//	emAfCommandApsFrame->destinationEndpoint =0x0001;
//
//	test2 =emberAfSendCommandUnicast(EMBER_OUTGOING_DIRECT,0x0002);
//	emberAfCorePrintln("---sendWriteResourceCommand--final: %d",test2);
//}
//void sendAddGroupCommand(uint16_t groupId,uint8_t* groupName,uint16_t destination)
//{
//	EmberStatus  test2;
//	//emberAfCorePrintln("---testSendCommandUnicast--final: %d",test2);
//	emberAfFillCommandGroupsClusterAddGroup(groupId, groupName);
//	emAfCommandApsFrame->profileId=emberAfProfileIdFromIndex(0);
//	emAfCommandApsFrame->clusterId =0x0004;
//	emAfCommandApsFrame->sourceEndpoint	=emberAfEndpointFromIndex(0);
//	emAfCommandApsFrame->destinationEndpoint =0x0001;
//
//	test2 =emberAfSendCommandUnicast(EMBER_OUTGOING_DIRECT,destination);
//	emberAfCorePrintln("---sendAddGroupCommand--final: %d",test2);
//}
//
uint8_t sendAddGroupCommand1(uint16_t nodeId,uint16_t groupId)
{
	EmberStatus  test2;
	//emberAfCorePrintln("---testSendCommandUnicast--final: %d",test2);
	emberAfFillCommandGroupsClusterAddGroup(groupId,"A");
	emAfCommandApsFrame->profileId=emberAfProfileIdFromIndex(0);
	emAfCommandApsFrame->clusterId =0x0004;
	emAfCommandApsFrame->sourceEndpoint	=emberAfEndpointFromIndex(0);
	emAfCommandApsFrame->destinationEndpoint =0x0001;

	test2 =emberAfSendCommandUnicast(EMBER_OUTGOING_DIRECT,nodeId);
	emberAfCorePrintln("---sendAddGroupCommand--final: %d",test2);
	return test2;
}
//
//void sendTestCommand()
//{
//	EmberApsFrame customApsFrame;
//	customApsFrame.profileId = emberAfProfileIdFromIndex(0);
//	customApsFrame.clusterId = 0x5566;
//	customApsFrame.sourceEndpoint = emberAfEndpointFromIndex(0);
//	customApsFrame.destinationEndpoint = 0x01;
//	customApsFrame.options = EMBER_APS_OPTION_RETRY|\
//							 EMBER_APS_OPTION_SOURCE_EUI64|\
//							 EMBER_APS_OPTION_ENABLE_ROUTE_DISCOVERY|\
//							 EMBER_APS_OPTION_DESTINATION_EUI64;
//
//	int16u destination = 0x0000;
//	int8u testBuffer[7] = {0x00,0x00,0xFF,0x11,0x14,0x45,0x54};
//	// 0 = framCounter
//	// 1 = seq
//	// 2 =command id
//	testBuffer[1] = emberAfNextSequence();
//
//	emberAfSendUnicast(EMBER_OUTGOING_DIRECT,
//	                                destination,
//	                                &customApsFrame,
//	                                sizeof(testBuffer),
//									testBuffer);
//
//
//}
//
//
//
// extended function
uint8_t putOnOffSchedual(char* scheString)
{
	uint8_t status;
	char buffer[128];
	buffer[0] = strlen(scheString);
	memcpy(&buffer[1], scheString, buffer[0]);
	//ZCL_PUT_ON_OFF_SCHEDULE_COMMAND_ID;
	emberAfFillCommandManagerPutOnOffSchedule(buffer);
	emAfCommandApsFrame->profileId=emberAfProfileIdFromIndex(0);//emberAfProfileIdFromIndex(0);
//	emAfCommandApsFrame->clusterId =netR.cluster;
	emAfCommandApsFrame->sourceEndpoint	=emberAfEndpointFromIndex(0);
	emAfCommandApsFrame->destinationEndpoint =1;
	status =emberAfSendCommandUnicast(EMBER_OUTGOING_DIRECT,userNewNodeId);
	return status;
}

/*
 * EmberStatus emAfFragmentationSendUnicast(EmberOutgoingMessageType type,
                                         uint16_t indexOrDestination,
                                         EmberApsFrame *apsFrame,
                                         uint8_t *buffer,
                                         uint16_t bufLen,
                                         uint8_t *messageTag)*/
uint8_t putOnOffSchV2(char* buffer,int leng)
{
	uint8_t status;
	uint8_t messageTag;
	EmberApsFrame customApsFrame;
	customApsFrame.profileId = emberAfProfileIdFromIndex(0);
	customApsFrame.clusterId = ZCL_MANAGER_ID; //0x5566;
	customApsFrame.sourceEndpoint = emberAfEndpointFromIndex(0);
	customApsFrame.destinationEndpoint = 0x01;
	customApsFrame.options = EMBER_APS_OPTION_FRAGMENT;

	char bufferSend[leng+5];   //
	// 0 = framCounter  ZCL_CLUSTER_SPECIFIC_COMMAND| ZCL_MANUFACTURER_SPECIFIC_MASK | ZCL_FRAME_CONTROL_CLIENT_TO_SERVER;
	// 1-2 = ZCL ID
	// 3 =emberAfNextSequence
	// 4 Command Id  // see fill command to fill right
	bufferSend[0]=5;
	bufferSend[1]=162;
	bufferSend[2]=16;
	bufferSend[3]=emberAfNextSequence();;
	bufferSend[4]=ZCL_PUT_ON_OFF_SCHEDULE_COMMAND_ID;
	memcpy(&bufferSend[5], buffer,leng );
	for(int i=0;i<sizeof(bufferSend);i++)
	{
		emberAfCorePrintln("bufferSend[%d]:%d",i,bufferSend[i]);
	}
	status = emAfFragmentationSendUnicast(EMBER_OUTGOING_DIRECT,userNewNodeId,&customApsFrame,bufferSend,sizeof(bufferSend),&messageTag);
	emberAfCorePrintln("message tag:%d",messageTag);
	return status;
}

uint8_t putDimmingSchedual(char* dimString)
{

//	EmberApsFrame customApsFrame;
//	customApsFrame.profileId = emberAfProfileIdFromIndex(0);
//	customApsFrame.clusterId = 0x5566;
//	customApsFrame.sourceEndpoint = emberAfEndpointFromIndex(0);
//	customApsFrame.destinationEndpoint = 0x01;

	//
	uint8_t status;
	emberAfFillCommandManagerPutDimmingSchedule(dimString);
	emAfCommandApsFrame->profileId=emberAfProfileIdFromIndex(0);//emberAfProfileIdFromIndex(0);
//	emAfCommandApsFrame->clusterId =netR.cluster;
	emAfCommandApsFrame->sourceEndpoint	=emberAfEndpointFromIndex(0);
	emAfCommandApsFrame->destinationEndpoint =1;
	status =emberAfSendCommandUnicast(EMBER_OUTGOING_DIRECT,userNewNodeId);
	return status;
}
