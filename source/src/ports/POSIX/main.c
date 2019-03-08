/*******************************************************************************
 * Copyright (c) 2009, Rockwell Automation, Inc.
 * All rights reserved.
 *
 ******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/capability.h>

#ifdef OPENER_RT
#include <pthread.h>
#include <sys/mman.h>
#include <sched.h>
#include <limits.h>
#endif

#include "generic_networkhandler.h"
#include "opener_api.h"
#include "cipcommon.h"
#include "trace.h"
#include "networkconfig.h"
#include "doublylinkedlist.h"
#include "cipconnectionobject.h"

#include "sample_application/sampleapplication.c"
#include "time.h"
/******************************************************************************/
/** @brief Signal handler function for ending stack execution
 *
 * @param signal the signal we received
 */
void LeaveStack(int signal);

/******************************************************************************/
/** @brief Signal handler function for ending stack execution
 *
 * @param signal the signal we received
 */
void *executeEventLoop(
  );

/*****************************************************************************/
/** @brief Flag indicating if the stack should end its execution
 */
int g_end_stack = 0;

/******************************************************************************/
int main(int argc,
         char *arg[]) {

  cap_t capabilities;
  cap_value_t capabilies_list[1];

  capabilities = cap_get_proc();
  if (NULL == capabilities) {
    printf("Could not get capabilities\n");
    exit(0);
  }

  capabilies_list[0] = CAP_NET_RAW;
  if (-1
      == cap_set_flag(capabilities, CAP_EFFECTIVE, 1, capabilies_list,
                      CAP_SET) ) {
    cap_free(capabilities);
    printf("Could not set CAP_NET_RAW capability\n");
    exit(0);
  }

  if (-1 == cap_set_proc(capabilities) ) {
    cap_free(capabilities);
    printf("Could not push CAP_NET_RAW capability to process\n");
    exit(0);
  }

  if (-1 == cap_free(capabilities) ) {
    printf("Could not free capabilites value\n");
    exit(0);
  }

  if (argc != 2) {
    printf("Wrong number of command line parameters!\n");
    printf("The correct command line parameters are:\n");
    printf("./OpENer interfacename\n");
    printf("    e.g. ./OpENer eth1\n");
    exit(0);
  } else {
    DoublyLinkedListInitialize(&connection_list,
                               CipConnectionObjectListArrayAllocator,
                               CipConnectionObjectListArrayFree);
    /* fetch Internet address info from the platform */
    if (kEipStatusError == ConfigureNetworkInterface(arg[1]) ) {
      printf("Network interface %s not found.\n", arg[1]);
      exit(0);
    }
    ConfigureDomainName();
    ConfigureHostName();

    ConfigureMacAddress(arg[1]);
  }

  /* for a real device the serial number should be unique per device */
  SetDeviceSerialNumber(123456789);

  /* unique_connection_id should be sufficiently random or incremented and stored
   *  in non-volatile memory each time the device boots.
   */
  EipUint16 unique_connection_id = rand();

  /* Setup the CIP Layer */
  CipStackInit(unique_connection_id);

  /* Setup Network Handles */
  if (kEipStatusOk == NetworkHandlerInitialize() ) {
    g_end_stack = 0;
#ifndef WIN32
    /* register for closing signals so that we can trigger the stack to end */
    signal(SIGHUP, LeaveStack);
#endif
#ifdef OPENER_RT
    /* Memory lock all*/
    if (mlockall(MCL_CURRENT | MCL_FUTURE) == -1) {
      OPENER_TRACE_ERR("mlockall failed: %m\n");
      exit(-2);
    }

    struct sched_param param;
    pthread_attr_t attr;
    pthread_t thread;
    CipUint ret = pthread_attr_init(&attr);
    if (ret) {
      OPENER_TRACE_ERR("init pthread attributes failed\n");
      exit(-2);
    }

    /* Set stack size  */
    ret = pthread_attr_setstacksize(&attr,
                                    PTHREAD_STACK_MIN + OPENER_RT_THREAD_SIZE);
    if (ret) {
      OPENER_TRACE_ERR("setstacksize failed\n");
      exit(-2);
    }

    /* Set policy and priority of the thread */
    ret = pthread_attr_setschedpolicy(&attr, SCHED_RR);
    if (ret) {
      OPENER_TRACE_ERR("setschedpolicy failed\n");
      exit(-2);
    }
    param.sched_priority = 80;
    ret = pthread_attr_setschedparam(&attr, &param);
    if (ret) {
      OPENER_TRACE_ERR("pthread setschedparam failed\n");
      exit(-2);
    }
    /* scheduling parameters */
    ret = pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
    if (ret) {
      OPENER_TRACE_ERR("setinheritsched failed\n");
      exit(-2);
    }

    /* Create a thread with the specified attributes */
    ret = pthread_create(&thread, &attr, executeEventLoop, NULL);
    if (ret) {
      OPENER_TRACE_ERR("create pthread failed\n");
      exit(-2);
    }

    /* Join the thread */
    ret = pthread_join(thread, NULL);
    if (ret) {
      OPENER_TRACE_ERR("join pthread failed: %m\n");
    }
    /* Unlock memory */
    munlockall();
#else
    executeEventLoop();
#endif
    /* clean up network state */
    NetworkHandlerFinish();
  }
  /* close remaining sessions and connections, cleanup used data */
  ShutdownCipStack();

  return -1;
}

void LeaveStack(int signal) {
  (void) signal;       /* kill unused parameter warning */
  OPENER_TRACE_STATE("got signal HUP\n");
  g_end_stack = 1;
}

void *executeEventLoop() {
  /* The event loop. Put other processing you need done continually in here */
    time_t begin;
    begin = time(NULL);
    while (1 != g_end_stack) {

      if(difftime(time(NULL), begin) > 1.0) {
          begin=time(NULL);

          printf("\n");
          struct ioAssembly input = getOutputImplicitAssembly();
          PrintImplicitAssembly(input);
          setInputImplicitAssembly(input);

          EipFloat floats[1000];
          int size = getReadOnlyRealAssembly(floats);
          printf("Reals:");
          for(unsigned int i = 0; i<size; i++){
            printf("%f ", floats[i]);
          }
          printf("\n");

          EipInt32 dints[1000];
          int dsize = getReadOnlyDintAssembly(dints);
          printf("Dints:");
          for(unsigned int i = 0; i<dsize; i++){
              printf("%d ", dints[i]);
          }
          printf("\n");


          char* strings[30];
          int ssize = getReadOnlyStringAssembly(strings);
          printf("Strings: " );
          for(unsigned int i = 0; i<ssize; i++){
              if(strings[i])
                printf("%s ", strings[i]);
              else
                  printf("NULL ");
          }
          printf("\n");

          EipFloat ofloats[1000] = {0};
          ofloats[0] = 1.012;
          ofloats[1] = 2.012;
          ofloats[2] = 3.032;
          setWriteOnlyRealAssembly(ofloats);

          EipInt32 odints[1000] = {0};
          odints[0]=1;
          odints[1]=2;
          odints[2]=3;
          odints[3]=4;
          setWriteOnlyDintAssembly(odints);

          char* ostrings[30] = {0};
          ostrings[0] = "jakes";
          ostrings[1] = "a";
          ostrings[2] = "banana";
          setWriteOnlyStringAssembly(ostrings);


      }

    if (kEipStatusOk != NetworkHandlerProcessOnce() ) {
      OPENER_TRACE_ERR("Error in NetworkHandler loop! Exiting OpENer!\n");
      break;
    }
  }
}
