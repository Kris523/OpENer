/*******************************************************************************
 * Copyright (c) 2012, Rockwell Automation, Inc.
 * All rights reserved.
 *
 ******************************************************************************/

#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include "opener_api.h"
#include "appcontype.h"
#include "trace.h"
#include "cipidentity.h"

/* outputs are my reads */
/*inputs are my writes */


#define EIGEN_ADAPTER_WRITE_ONLY                100 //0x064
#define EIGEN_ADAPTER_READ_ONLY               150 //0x096

//Not used yet
#define DEMO_APP_CONFIG_ASSEMBLY_NUM               151 //0x097
#define DEMO_APP_HEARTBEAT_INPUT_ONLY_ASSEMBLY_NUM  152 //0x098
#define DEMO_APP_HEARTBEAT_LISTEN_ONLY_ASSEMBLY_NUM 153 //0x099
#define DEMO_APP_EXPLICT_ASSEMBLY_NUM              154 //0x09A

/* global variables for demo application (4 assembly data fields)  ************/

extern CipUint g_encapsulation_inactivity_timeout;

EipUint8 g_assembly_data064[96]; /* Input */
// Input Trigger       4
EipInt32 inputTrigger;
//Spare                4
EipInt32 s_inputTrigger;
//Part number Length   4
EipInt32 inputPartLength;
//Part number String   32
char inputPartString[32];
// Int Part Number     4
EipInt32 inputPartNumber;
// Polling Real        48
EipFloat inputPollingReal[12];



EipUint8 g_assembly_data096[96]; /* Output */
// output Trigger       4
EipInt32 outputTrigger;
//Spare                4
EipInt32 s_outputTrigger;
//Part number Length   4
EipInt32 outputPartLength;
//Part number String   32
char outputPartString[32];
// Int Part Number     4
EipInt32 outputPartNumber;
// Polling Real        48
EipFloat outputPollingReal[12];






EipUint8 g_assembly_data097[10]; /* Config */
EipUint8 g_assembly_data09A[32]; /* Explicit */

//PLC needs same type and size for explicit length

EipStatus ApplicationInitialization(void) {
  /* create 3 assembly object instances*/
  /*INPUT*/
  CreateAssemblyObject( EIGEN_ADAPTER_WRITE_ONLY, g_assembly_data064,
                        sizeof(g_assembly_data064) );
// input Trigger       4
    inputTrigger = *(EipInt32 *)&g_assembly_data064[0];
//Spare                4
    s_inputTrigger = *(EipInt32 *)&g_assembly_data064[4];
//Part number Length   4
    inputPartLength = *(EipInt32 *)&g_assembly_data064[8];
//Part number String   32
    *inputPartString = *&g_assembly_data064[12];
// Int Part Number     4
    inputPartNumber = *(EipInt32 *)&g_assembly_data064[44];
// Polling Real        48
    *inputPollingReal = *(EipFloat *)&g_assembly_data064[48];


  /*OUTPUT*/
  CreateAssemblyObject( EIGEN_ADAPTER_READ_ONLY, g_assembly_data096,
                        sizeof(g_assembly_data096) );
  // output Trigger       4
    outputTrigger = *(EipInt32 *)&g_assembly_data096[0];
//Spare                4
    s_outputTrigger = *(EipInt32 *)&g_assembly_data096[4];
//Part number Length   4
    outputPartLength = *(EipInt32 *)&g_assembly_data096[8];
//Part number String   32
    *outputPartString = *&g_assembly_data096[12];
// Int Part Number     4
    outputPartNumber = *(EipInt32 *)&g_assembly_data096[44];
// Polling Real        48
    *outputPollingReal = *(EipFloat *)&g_assembly_data096[48];


  /*CONFIG*/
  CreateAssemblyObject( DEMO_APP_CONFIG_ASSEMBLY_NUM, g_assembly_data097,
                        sizeof(g_assembly_data097) );

  /*Heart-beat output assembly for Input only connections */
  CreateAssemblyObject(DEMO_APP_HEARTBEAT_INPUT_ONLY_ASSEMBLY_NUM, NULL, 0);

  /*Heart-beat output assembly for Listen only connections */
  CreateAssemblyObject(DEMO_APP_HEARTBEAT_LISTEN_ONLY_ASSEMBLY_NUM, NULL, 0);

  /* assembly for explicit messaging */
  CreateAssemblyObject( DEMO_APP_EXPLICT_ASSEMBLY_NUM, g_assembly_data09A,
                        sizeof(g_assembly_data09A) );

  ConfigureExclusiveOwnerConnectionPoint(0, EIGEN_ADAPTER_READ_ONLY,
                                         EIGEN_ADAPTER_WRITE_ONLY,
                                         DEMO_APP_CONFIG_ASSEMBLY_NUM);
  ConfigureInputOnlyConnectionPoint(0,
                                    DEMO_APP_HEARTBEAT_INPUT_ONLY_ASSEMBLY_NUM,
                                    EIGEN_ADAPTER_WRITE_ONLY,
                                    DEMO_APP_CONFIG_ASSEMBLY_NUM);
  ConfigureListenOnlyConnectionPoint(0,
                                     DEMO_APP_HEARTBEAT_LISTEN_ONLY_ASSEMBLY_NUM,
                                     EIGEN_ADAPTER_WRITE_ONLY,
                                     DEMO_APP_CONFIG_ASSEMBLY_NUM);

  return kEipStatusOk;
}

void HandleApplication(void) {
  /* check if application needs to trigger an connection */
}

void CheckIoConnectionEvent(unsigned int output_assembly_id,
                            unsigned int input_assembly_id,
                            IoConnectionEvent io_connection_event) {
  /* maintain a correct output state according to the connection state*/

  (void) output_assembly_id; /* suppress compiler warning */
  (void) input_assembly_id; /* suppress compiler warning */
  (void) io_connection_event; /* suppress compiler warning */
}

EipStatus AfterAssemblyDataReceived(CipInstance *instance) {
  EipStatus status = kEipStatusOk;

  /*handle the data received e.g., update outputs of the device */
  switch (instance->instance_number) {
    case EIGEN_ADAPTER_READ_ONLY:


      break;
    case DEMO_APP_EXPLICT_ASSEMBLY_NUM:
        printf("EXPLICET SOMETHING!!\n");
      /* do something interesting with the new data from
       * the explicit set-data-attribute message */
      break;
    case DEMO_APP_CONFIG_ASSEMBLY_NUM:
      /* Add here code to handle configuration data and check if it is ok
       * The demo application does not handle config data.
       * However in order to pass the test we accept any data given.
       * EIP_ERROR
       */
      status = kEipStatusOk;
      break;
    default:
      OPENER_TRACE_INFO(
        "Unknown assembly instance ind AfterAssemblyDataReceived");
      break;
  }
//
//    printf("Input  Triggers: %d\t"
//           "Spare: %d\t"
//           "PartNumber Length %d\t"
//           "PartNumber String %s\t"
//           "PartNumber %d\t"
//           "Input  Polling %f %f %f %f %f %f %f %f %f %f %f %f\n",
//           inputTrigger,
//           s_inputTrigger,
//           inputPartLength,
//           inputPartString,
//           inputPartNumber,
//           inputPollingReal[0],
//           inputPollingReal[1],
//           inputPollingReal[2],
//           inputPollingReal[3],
//           inputPollingReal[4],
//           inputPollingReal[5],
//           inputPollingReal[6],
//           inputPollingReal[7],
//           inputPollingReal[8],
//           inputPollingReal[9],
//           inputPollingReal[10],
//           inputPollingReal[11]);
//
//    printf("Output Triggers: %d\t"
//           "Spare: %d\t"
//           "PartNumber Length %d\t"
//           "PartNumber String %s\t"
//           "PartNumber %d\t"
//           "Output Polling %f %f %f %f %f %f %f %f %f %f %f %f\n",
//           outputTrigger,
//           s_outputTrigger,
//           outputPartLength,
//           outputPartString,
//           outputPartNumber,
//           outputPollingReal[0],
//           outputPollingReal[1],
//           outputPollingReal[2],
//           outputPollingReal[3],
//           outputPollingReal[4],
//           outputPollingReal[5],
//           outputPollingReal[6],
//           outputPollingReal[7],
//           outputPollingReal[8],
//           outputPollingReal[9],
//           outputPollingReal[10],
//           outputPollingReal[11]);

//
//    printf("Current Assembly Data: INPUT \n");
//    int i = 0;
//    for(i=0;i<sizeof(g_assembly_data064);i++) {
//        printf("%d", (unsigned char) g_assembly_data064[i]);
//    }
//    printf("\n");
//    printf("Current Assembly Data: Output \n");
//    for(i=0;i<sizeof(g_assembly_data096);i++) {
//        printf("%d", (unsigned char) g_assembly_data096[i]);
//    }
//    printf("\n");
//    printf("Current Assembly Data: Config \n");
//    for(i=0;i<sizeof(g_assembly_data097);i++) {
//        printf("\\%02hhx", (unsigned char) g_assembly_data097[i]);
//    }
//    printf("\n");
//    printf("Current Assembly Data: Explicit \n");
//    for(i=0;i<sizeof(g_assembly_data09A);i++) {
//        printf("\\%02hhx", (unsigned char) g_assembly_data09A[i]);
//    }
//    printf("\n");
//    g_assembly_data064[2]='f';
//    g_assembly_data064[3]='u';
//    g_assembly_data064[4]='c';
//    g_assembly_data064[5]='k';
//    g_assembly_data064[6]=' ';
//    g_assembly_data064[7]='y';
//    g_assembly_data064[8]='o';
//    g_assembly_data064[9]='u';

  return status;
}


void PrintCurrentStates(){

    printf("Input  Triggers: %d\t"
           "Spare: %d\t"
           "PartNumber Length %d\t"
           "PartNumber String %s\t"
           "PartNumber %d\t"
           "Input  Polling %f %f %f %f %f %f %f %f %f %f %f %f\n",
           inputTrigger,
           s_inputTrigger,
           inputPartLength,
           inputPartString,
           inputPartNumber,
           inputPollingReal[0],
           inputPollingReal[1],
           inputPollingReal[2],
           inputPollingReal[3],
           inputPollingReal[4],
           inputPollingReal[5],
           inputPollingReal[6],
           inputPollingReal[7],
           inputPollingReal[8],
           inputPollingReal[9],
           inputPollingReal[10],
           inputPollingReal[11]);

    printf("Output Triggers: %d\t"
           "Spare: %d\t"
           "PartNumber Length %d\t"
           "PartNumber String %s\t"
           "PartNumber %d\t"
           "Output Polling %f %f %f %f %f %f %f %f %f %f %f %f\n",
           outputTrigger,
           s_outputTrigger,
           outputPartLength,
           outputPartString,
           outputPartNumber,
           outputPollingReal[0],
           outputPollingReal[1],
           outputPollingReal[2],
           outputPollingReal[3],
           outputPollingReal[4],
           outputPollingReal[5],
           outputPollingReal[6],
           outputPollingReal[7],
           outputPollingReal[8],
           outputPollingReal[9],
           outputPollingReal[10],
           outputPollingReal[11]);

}

EipBool8 BeforeAssemblyDataSend(CipInstance *pa_pstInstance) {
  /*update data to be sent e.g., read outputs of the device */
  /*In this sample app we mirror the data from out to inputs on data receive
   * therefore we need nothing to do here. Just return true to inform that
   * the data is new.
   */

  if (pa_pstInstance->instance_number == DEMO_APP_EXPLICT_ASSEMBLY_NUM) {
    /* do something interesting with the existing data
     * for the explicit get-data-attribute message */
  }
  return true;
}

EipStatus ResetDevice(void) {
  /* add reset code here*/
  CloseAllConnections();
  return kEipStatusOk;
}

EipStatus ResetDeviceToInitialConfiguration(void) {
  /*rest the parameters */
  g_encapsulation_inactivity_timeout = 120;
  /*than perform device reset*/
  ResetDevice();
  return kEipStatusOk;
}

void *
CipCalloc(size_t number_of_elements,
          size_t size_of_element) {
  return calloc(number_of_elements, size_of_element);
}

void CipFree(void *data) {
  free(data);
}

void RunIdleChanged(EipUint32 run_idle_value) {
  OPENER_TRACE_INFO("Run/Idle handler triggered\n");
  if( (0x0001 & run_idle_value) == 1 ) {
    CipIdentitySetExtendedDeviceStatus(kAtLeastOneIoConnectionInRunMode);
  } else {
    CipIdentitySetExtendedDeviceStatus(
      kAtLeastOneIoConnectionEstablishedAllInIdleMode);
  }
  (void) run_idle_value;
}

