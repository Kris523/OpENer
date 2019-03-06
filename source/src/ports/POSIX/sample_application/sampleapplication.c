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

#define EIGEN_ADAPTER_READ_ONLY_REAL       1
#define EIGEN_ADAPTER_READ_ONLY_DINT       1
#define EIGEN_ADAPTER_READ_ONLY_STRING     1


#define EIGEN_ADAPTER_WRITE_ONLY_REAL       1
#define EIGEN_ADAPTER_WRITE_ONLY_DINT       1
#define EIGEN_ADAPTER_WRITE_ONLY_STRING     1
/* global variables for demo application (4 assembly data fields)  ************/

extern CipUint g_encapsulation_inactivity_timeout;


struct ioAssembly {
    EipInt32 Trigger;
    EipInt32 s_Trigger;
    EipInt32 partLength;
    char partString[32];
    EipInt32 partNumber;
    EipFloat pollingReal[12];
};

EipUint8 g_assembly_data064[96]; /* Input */
EipUint8 g_assembly_data096[96]; /* Output */

EipUint8 g_assembly_data097[10]; /* Config */
EipUint8 g_assembly_data09A[32]; /* Explicit */


EipStatus ApplicationInitialization(void) {
  /* create 3 assembly object instances*/
  /*INPUT*/
  CreateAssemblyObject( EIGEN_ADAPTER_WRITE_ONLY, g_assembly_data064,
                        sizeof(g_assembly_data064) );

  /*OUTPUT*/
  CreateAssemblyObject( EIGEN_ADAPTER_READ_ONLY, g_assembly_data096,
                        sizeof(g_assembly_data096) );

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
        //Handle read message of the data.

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
  return status;
}

void setInputAssembly(struct ioAssembly input){

    memcpy(&g_assembly_data064[0], &input.Trigger, sizeof(input.Trigger));
    memcpy(&g_assembly_data064[4], &input.s_Trigger, sizeof(input.s_Trigger));
    memcpy(&g_assembly_data064[8], &input.partLength, sizeof(input.partLength));
    memcpy(&g_assembly_data064[12], &input.partString, input.partLength*sizeof(EipInt8));
    memcpy(&g_assembly_data064[44], &input.partNumber, sizeof(input.partNumber));
    memcpy(&g_assembly_data064[48], &input.pollingReal, 48);

}
struct ioAssembly getOutputAssembly(){
    struct ioAssembly output;

    output.Trigger = *(EipInt32 *)&g_assembly_data096[0];
    output.s_Trigger = *(EipInt32 *)&g_assembly_data096[4];
    output.partLength = *(EipInt32 *)&g_assembly_data096[8];
    memcpy(output.partString, &g_assembly_data096[12], output.partLength*sizeof(EipInt8));
    output.partNumber = *(EipInt32 *)&g_assembly_data096[44];
    memcpy(output.pollingReal, &g_assembly_data096[48], 48);

    return output;
}


void PrintAssembly(struct ioAssembly assembly){
    printf("Triggers: %d\t"
           "Spare: %d\t"
           "PartNumber Length %d\t"
           "PartNumber String %s\t"
           "PartNumber %d\t"
           "Polling %f %f %f %f %f %f %f %f %f %f %f %f\n\n",
           assembly.Trigger,
           assembly.s_Trigger,
           assembly.partLength,
           assembly.partString,
           assembly.partNumber,
           assembly.pollingReal[0],
           assembly.pollingReal[1],
           assembly.pollingReal[2],
           assembly.pollingReal[3],
           assembly.pollingReal[4],
           assembly.pollingReal[5],
           assembly.pollingReal[6],
           assembly.pollingReal[7],
           assembly.pollingReal[8],
           assembly.pollingReal[9],
           assembly.pollingReal[10],
           assembly.pollingReal[11]);
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

