/* Standard include */
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

/* Simplelink includes */
#include "simplelink.h"
#include "netcfg.h"

/* Driverlib includes */
#include "hw_types.h"
#include "pin.h"
#include "interrupt.h"
#include "hw_ints.h"
#include "hw_apps_rcm.h"
#include "hw_common_reg.h"
#include "prcm.h"
#include "rom.h"
#include "rom_map.h"
#include "hw_memmap.h"
#include "timer.h"
#include "hw_timer.h"
#include "utils.h"
#include "gpio.h"
#include "uart.h"
#include "hw_i2c.h"
#include "i2c.h"
#include "netapp.h"
#include "common.h"

/* Common interface includes */
#include "timer_if.h"
#include "gpio_if.h"
#include "i2c_if.h"
#include "uart_if.h"
#include "smartconfig.h"

/* My libraries */
#include "MPU9150.h"
#include "Motors.h"
#include "imu_control.h"
#include "pidpod_gpio.h"
#include "odometer.h"

/* Definitions */
#define LED_ON_STRING			"12345678901234567890123456789012345"

/* Gloab variables */
float kp = 20;    // 10 is ok
float ki = 25;    // 25/30 is ok. Maybe even more
float kd = 0.5;   // 0.4/0.5 is ok, 1 is stability limit with others ok values

float kp_odo = 100;		// 20 stability limit, 10 is ok
float ki_odo = 3;	// 200 stability limit, 190/200 is ok
float kd_odo = 50;	// 300 stability limit, 100/150 is ok


const char myssid[10] = "PIDPODlink";
SlNetAppDhcpServerBasicOpt_t dhcpParams;
unsigned char outLen = sizeof(SlNetAppDhcpServerBasicOpt_t);
SlNetCfgIpV4Args_t ipV4;


unsigned long  g_ulStatus = 0;//SimpleLink Status
unsigned long  g_ulGatewayIP = 0; //Network Gateway IP address
unsigned char  g_ucConnectionSSID[SSID_LEN_MAX+1]; //Connection SSID
unsigned char  g_ucConnectionBSSID[BSSID_LEN_MAX]; //Connection BSSID
unsigned char POST_token[] = "__SL_P_ULD";
unsigned char GET_token[]  = "__SL_G_ULD";
unsigned char GET_token_STR[]  = "__SL_G_STR";								// length of the string
unsigned char GET_token_TIM[]  = "__SL_G_TIM";								// time delay between web refresh
//unsigned char GET_update_token[]  = "__SL_G_STATUS_UPDATE";
//unsigned char GET_settings_token[]  = "__SL_G_STATUS_SETTINGS";
signed int g_uiIpAddress = 0;


/* Variable related to Connection status */
volatile unsigned short g_usMCNetworkUstate = 0;
unsigned int g_uiDeviceModeConfig = ROLE_STA; //default is STA mode
unsigned char g_ucConnectTimeout =0;


/* Globals used by the timer interrupt handler */
#if defined(ccs)
extern void (* const g_pfnVectors[])(void);
#endif


static void BoardInit(void)
{

	/* In case of TI-RTOS vector table is initialize by OS itself */
	#ifndef USE_TIRTOS

	#if defined(ccs)
	    MAP_IntVTableBaseSet((unsigned long)&g_pfnVectors[0]);
	#endif
	#if defined(ewarm)
	    MAP_IntVTableBaseSet((unsigned long)&__vector_table);
	#endif
	#endif

    MAP_IntMasterEnable();
    MAP_IntEnable(FAULT_SYSTICK);

    PRCMCC3200MCUInit();
}


//*****************************************************************************
// SimpleLink Asynchronous Event Handlers -- Start
//*****************************************************************************


//*****************************************************************************
//
//! \brief The Function Handles WLAN Events
//!
//! \param[in]  pWlanEvent - Pointer to WLAN Event Info
//!
//! \return None
//!
//*****************************************************************************
void SimpleLinkWlanEventHandler(SlWlanEvent_t *pWlanEvent)
{
    if(!pWlanEvent)
    {
        return;
    }

    switch(pWlanEvent->Event)
    {
        case SL_WLAN_CONNECT_EVENT:
        {
            SET_STATUS_BIT(g_ulStatus, STATUS_BIT_CONNECTION);

            //
            // Information about the connected AP (like name, MAC etc) will be
            // available in 'slWlanConnectAsyncResponse_t'-Applications
            // can use it if required
            //
            //  slWlanConnectAsyncResponse_t *pEventData = NULL;
            // pEventData = &pWlanEvent->EventData.STAandP2PModeWlanConnected;
            //

            // Copy new connection SSID and BSSID to global parameters
            memcpy(g_ucConnectionSSID,pWlanEvent->EventData.
                   STAandP2PModeWlanConnected.ssid_name,
                   pWlanEvent->EventData.STAandP2PModeWlanConnected.ssid_len);
            memcpy(g_ucConnectionBSSID,
                   pWlanEvent->EventData.STAandP2PModeWlanConnected.bssid,
                   SL_BSSID_LENGTH);

            UART_PRINT("[WLAN EVENT] STA Connected to the AP: %s ,"
                        "BSSID: %x:%x:%x:%x:%x:%x\n\r",
                      g_ucConnectionSSID,g_ucConnectionBSSID[0],
                      g_ucConnectionBSSID[1],g_ucConnectionBSSID[2],
                      g_ucConnectionBSSID[3],g_ucConnectionBSSID[4],
                      g_ucConnectionBSSID[5]);
        }
        break;

        case SL_WLAN_DISCONNECT_EVENT:
        {
            slWlanConnectAsyncResponse_t*  pEventData = NULL;

            CLR_STATUS_BIT(g_ulStatus, STATUS_BIT_CONNECTION);
            CLR_STATUS_BIT(g_ulStatus, STATUS_BIT_IP_AQUIRED);

            pEventData = &pWlanEvent->EventData.STAandP2PModeDisconnected;

            // If the user has initiated 'Disconnect' request,
            //'reason_code' is SL_USER_INITIATED_DISCONNECTION
            if(SL_USER_INITIATED_DISCONNECTION == pEventData->reason_code)
            {
                UART_PRINT("[WLAN EVENT]Device disconnected from the AP: %s,"
                "BSSID: %x:%x:%x:%x:%x:%x on application's request \n\r",
                           g_ucConnectionSSID,g_ucConnectionBSSID[0],
                           g_ucConnectionBSSID[1],g_ucConnectionBSSID[2],
                           g_ucConnectionBSSID[3],g_ucConnectionBSSID[4],
                           g_ucConnectionBSSID[5]);
            }
            else
            {
                UART_PRINT("[WLAN ERROR]Device disconnected from the AP AP: %s,"
                "BSSID: %x:%x:%x:%x:%x:%x on an ERROR..!! \n\r",
                           g_ucConnectionSSID,g_ucConnectionBSSID[0],
                           g_ucConnectionBSSID[1],g_ucConnectionBSSID[2],
                           g_ucConnectionBSSID[3],g_ucConnectionBSSID[4],
                           g_ucConnectionBSSID[5]);
            }
            memset(g_ucConnectionSSID,0,sizeof(g_ucConnectionSSID));
            memset(g_ucConnectionBSSID,0,sizeof(g_ucConnectionBSSID));
        }
        break;

        case SL_WLAN_STA_CONNECTED_EVENT:
        {
            // when device is in AP mode and any client connects to device cc3xxx
            SET_STATUS_BIT(g_ulStatus, STATUS_BIT_CONNECTION);
            CLR_STATUS_BIT(g_ulStatus, STATUS_BIT_CONNECTION_FAILED);

            //
            // Information about the connected client (like SSID, MAC etc) will
            // be available in 'slPeerInfoAsyncResponse_t' - Applications
            // can use it if required
            //
            // slPeerInfoAsyncResponse_t *pEventData = NULL;
            // pEventData = &pSlWlanEvent->EventData.APModeStaConnected;
            //

        }
        break;

        case SL_WLAN_STA_DISCONNECTED_EVENT:
        {
            // when client disconnects from device (AP)
            CLR_STATUS_BIT(g_ulStatus, STATUS_BIT_CONNECTION);
            CLR_STATUS_BIT(g_ulStatus, STATUS_BIT_IP_LEASED);

            //
            // Information about the connected client (like SSID, MAC etc) will
            // be available in 'slPeerInfoAsyncResponse_t' - Applications
            // can use it if required
            //
            // slPeerInfoAsyncResponse_t *pEventData = NULL;
            // pEventData = &pSlWlanEvent->EventData.APModestaDisconnected;
            //
        }
        break;

        case SL_WLAN_SMART_CONFIG_COMPLETE_EVENT:
        {
            SET_STATUS_BIT(g_ulStatus, STATUS_BIT_SMARTCONFIG_START);

            //
            // Information about the SmartConfig details (like Status, SSID,
            // Token etc) will be available in 'slSmartConfigStartAsyncResponse_t'
            // - Applications can use it if required
            //
            //  slSmartConfigStartAsyncResponse_t *pEventData = NULL;
            //  pEventData = &pSlWlanEvent->EventData.smartConfigStartResponse;
            //

        }
        break;

        case SL_WLAN_SMART_CONFIG_STOP_EVENT:
        {
            // SmartConfig operation finished
            CLR_STATUS_BIT(g_ulStatus, STATUS_BIT_SMARTCONFIG_START);

            //
            // Information about the SmartConfig details (like Status, padding
            // etc) will be available in 'slSmartConfigStopAsyncResponse_t' -
            // Applications can use it if required
            //
            // slSmartConfigStopAsyncResponse_t *pEventData = NULL;
            // pEventData = &pSlWlanEvent->EventData.smartConfigStopResponse;
            //
        }
        break;

        case SL_WLAN_P2P_DEV_FOUND_EVENT:
        {
            SET_STATUS_BIT(g_ulStatus, STATUS_BIT_P2P_DEV_FOUND);

            //
            // Information about P2P config details (like Peer device name, own
            // SSID etc) will be available in 'slPeerInfoAsyncResponse_t' -
            // Applications can use it if required
            //
            // slPeerInfoAsyncResponse_t *pEventData = NULL;
            // pEventData = &pSlWlanEvent->EventData.P2PModeDevFound;
            //
        }
        break;

        case SL_WLAN_P2P_NEG_REQ_RECEIVED_EVENT:
        {
            SET_STATUS_BIT(g_ulStatus, STATUS_BIT_P2P_REQ_RECEIVED);

            //
            // Information about P2P Negotiation req details (like Peer device
            // name, own SSID etc) will be available in 'slPeerInfoAsyncResponse_t'
            //  - Applications can use it if required
            //
            // slPeerInfoAsyncResponse_t *pEventData = NULL;
            // pEventData = &pSlWlanEvent->EventData.P2PModeNegReqReceived;
            //
        }
        break;

        case SL_WLAN_CONNECTION_FAILED_EVENT:
        {
            // If device gets any connection failed event
            SET_STATUS_BIT(g_ulStatus, STATUS_BIT_CONNECTION_FAILED);
        }
        break;

        default:
        {
            UART_PRINT("[WLAN EVENT] Unexpected event [0x%x]\n\r",
                       pWlanEvent->Event);
        }
        break;
    }
}





//*****************************************************************************
//
//! This function handles socket events indication
//!
//! \param[in]      pSock - Pointer to Socket Event Info
//!
//! \return None
//!
//*****************************************************************************
void SimpleLinkSockEventHandler(SlSockEvent_t *pSock)
{
    //
    // This application doesn't work w/ socket - Events are not expected
    //

}


//*****************************************************************************
//
//! This function gets triggered when HTTP Server receives Application
//! defined GET and POST HTTP Tokens.
//!
//! \param pHttpServerEvent Pointer indicating http server event
//! \param pHttpServerResponse Pointer indicating http server response
//!
//! \return None
//!
//*****************************************************************************
void SimpleLinkHttpServerCallback(SlHttpServerEvent_t *pSlHttpServerEvent,
                               SlHttpServerResponse_t *pSlHttpServerResponse)
{
    unsigned char strLenVal = 0;
    unsigned char *ptr;
    char parameter[6];
    uint8_t nChar, nParam;

    /* Received void pointer */
    if(!pSlHttpServerEvent || !pSlHttpServerResponse)
    {
        return;
    }

    /* Received either a GET or POST event/request*/
    switch (pSlHttpServerEvent->Event)
    {
    	/*  GET event */
        case SL_NETAPP_HTTPGETTOKENVALUE_EVENT:
        {


			/* Point to the first char of the response string */
			ptr = pSlHttpServerResponse->ResponseData.token_value.data;
			pSlHttpServerResponse->ResponseData.token_value.len = 0;




			/* Look at the kind of token after a GET event/request */
			if(memcmp(pSlHttpServerEvent->EventData.httpTokenName.data, GET_token,
					strlen((const char *)GET_token)) == 0)
			{
				// TBD send data, fill response string + ??
				strLenVal = strlen(LED_ON_STRING);
				memcpy(ptr, LED_ON_STRING, strLenVal);
				pSlHttpServerResponse->ResponseData.token_value.len = strLenVal;
				ptr += strLenVal;
				*ptr = '\0';
			}

			else if(memcmp(pSlHttpServerEvent->EventData.httpTokenName.data, GET_token_STR,
					strlen((const char *)GET_token_STR)) == 0)
			{
				// TBD send data, fill response string + ??
				strLenVal = 1;
				memcpy(ptr, "5", strLenVal);
				pSlHttpServerResponse->ResponseData.token_value.len = strLenVal;
				ptr += strLenVal;
				*ptr = '\0';
			}

			else if(memcmp(pSlHttpServerEvent->EventData.httpTokenName.data, GET_token_TIM,
					strlen((const char *)GET_token_TIM)) == 0)
			{
				// TBD send data, fill response string + ??
				strLenVal = strlen("500");
				memcpy(ptr, "500", strLenVal);
				pSlHttpServerResponse->ResponseData.token_value.len = strLenVal;
				ptr += strLenVal;
				*ptr = '\0';
			}





        }
        break;

        /*  POST event */
        case SL_NETAPP_HTTPPOSTTOKENVALUE_EVENT:
        {

        	/* Point to the first char of the post request string */
        	ptr = pSlHttpServerEvent->EventData.httpPostData.token_name.data;

        	/* Look at the kind of token after a POST event/request */
        	/* Here the new PID values are parsed */
        	if(memcmp(ptr, POST_token, strlen((const char *)POST_token)) == 0)
        	{
        		/* Get out token */
        		for(nChar = 0; nChar < 10; nChar++)
        			*ptr++;
        		/* Get parameters, 6 parameters, each with 5 numbers and a random separator "." */
        		for(nParam = 0; nParam < 6; nParam++)
        		{
        			for(nChar = 0; nChar < 6; nChar++)
        				parameter[nChar] = *ptr++;

        			switch (nParam)
        			{
        				case 0:
        				{
        					kp = atof(parameter);
        				}
        				break;

        				case 1:
						{
							ki = atof(parameter);
						}
						break;

        				case 2:
						{
							kd = atof(parameter);
						}
						break;

        				case 3:
						{
							kp_odo = atof(parameter);
						}
						break;

        				case 4:
						{
							ki_odo = atof(parameter);
						}
						break;

        				case 5:
						{
							kd_odo = atof(parameter);
						}
						break;

        				default:
        				{}
        					break;
        			}

        		}
        	    set_controller_parameters(kp, ki, kd);
        	    set__odo_controller_parameters(kp_odo, ki_odo, kd_odo);
        	}
        	/* END controller parameters update event */

        }
        break;

        default:
        break;
    }
}



//*****************************************************************************
//
//! \brief This function handles network events such as IP acquisition, IP
//!           leased, IP released etc.
//!
//! \param[in]  pNetAppEvent - Pointer to NetApp Event Info
//!
//! \return None
//!
//*****************************************************************************
void SimpleLinkNetAppEventHandler(SlNetAppEvent_t *pNetAppEvent)
{
    if(!pNetAppEvent)
    {
        return;
    }

    switch(pNetAppEvent->Event)
    {
        case SL_NETAPP_IPV4_IPACQUIRED_EVENT:
        {
            SlIpV4AcquiredAsync_t *pEventData = NULL;

            SET_STATUS_BIT(g_ulStatus, STATUS_BIT_IP_AQUIRED);

            //Ip Acquired Event Data
            pEventData = &pNetAppEvent->EventData.ipAcquiredV4;
            g_uiIpAddress = pEventData->ip;

            //Gateway IP address
            g_ulGatewayIP = pEventData->gateway;

            /*UART_PRINT("[NETAPP EVENT] IP Acquired: IP=%d.%d.%d.%d , "
            "Gateway=%d.%d.%d.%d\n\r",
            SL_IPV4_BYTE(pNetAppEvent->EventData.ipAcquiredV4.ip,3),
            SL_IPV4_BYTE(pNetAppEvent->EventData.ipAcquiredV4.ip,2),
            SL_IPV4_BYTE(pNetAppEvent->EventData.ipAcquiredV4.ip,1),
            SL_IPV4_BYTE(pNetAppEvent->EventData.ipAcquiredV4.ip,0),
            SL_IPV4_BYTE(pNetAppEvent->EventData.ipAcquiredV4.gateway,3),
            SL_IPV4_BYTE(pNetAppEvent->EventData.ipAcquiredV4.gateway,2),
            SL_IPV4_BYTE(pNetAppEvent->EventData.ipAcquiredV4.gateway,1),
            SL_IPV4_BYTE(pNetAppEvent->EventData.ipAcquiredV4.gateway,0));
            */
        }
        break;

        case SL_NETAPP_IP_LEASED_EVENT:
        {
            SET_STATUS_BIT(g_ulStatus, STATUS_BIT_IP_LEASED);

            //
            // Information about the IP-Leased details(like IP-Leased,lease-time,
            // mac etc) will be available in 'SlIpLeasedAsync_t' - Applications
            // can use it if required
            //
            // SlIpLeasedAsync_t *pEventData = NULL;
            // pEventData = &pNetAppEvent->EventData.ipLeased;
            //

        }
        break;

        case SL_NETAPP_IP_RELEASED_EVENT:
        {
            CLR_STATUS_BIT(g_ulStatus, STATUS_BIT_IP_LEASED);

            //
            // Information about the IP-Released details (like IP-address, mac
            // etc) will be available in 'SlIpReleasedAsync_t' - Applications
            // can use it if required
            //
            // SlIpReleasedAsync_t *pEventData = NULL;
            // pEventData = &pNetAppEvent->EventData.ipReleased;
            //
        }
		break;

        default:
        {
            UART_PRINT("[NETAPP EVENT] Unexpected event [0x%x] \n\r",
                       pNetAppEvent->Event);
        }
        break;
    }
}



int main(void)
{

	/* Init board */
	BoardInit();

	/* Init GPIO */
	InitGPIO();

	/* Init UART */
    InitTerm();

    /* Init I2C */
    I2C_IF_Open(I2C_MASTER_MODE_FST);

    /* Init the internet! */
    // http://azug.minpet.unibas.ch/~lukas/bricol/ti_simplelink/resources/swru368.pdf SECTION 10
	dhcpParams.lease_time = 1000;
	dhcpParams.ipv4_addr_start = 0xc0a80102;
	dhcpParams.ipv4_addr_last = 0xc0a801fe;


	sl_Start(NULL,NULL,NULL);
	MAP_UtilsDelay(8000000);

	// config IP etc
	ipV4.ipV4 = 0xc0a80101;
	ipV4.ipV4Mask = 0xFFFFFF00;
	ipV4.ipV4Gateway = 0xc0a80101;
	ipV4.ipV4DnsServer = 0xc0a80101;
	sl_NetCfgSet(SL_IPV4_AP_P2P_GO_STATIC_ENABLE, 1 ,sizeof(SlNetCfgIpV4Args_t), (unsigned char*) &ipV4);

	sl_WlanSetMode(ROLE_AP);

	// config SSID
	sl_WlanSet(SL_WLAN_CFG_AP_ID,WLAN_AP_OPT_SSID, strlen(myssid), (unsigned char*) myssid);

	sl_Stop(100);
	sl_Start(NULL,NULL,NULL);

	// start DHCP server
	sl_NetAppStop(SL_NET_APP_DHCP_SERVER_ID);
	sl_NetAppSet(SL_NET_APP_DHCP_SERVER_ID, NETAPP_SET_DHCP_SRV_BASIC_OPT, outLen,(unsigned char*) &dhcpParams);
	sl_NetAppStart(SL_NET_APP_DHCP_SERVER_ID);


	//Stop Internal HTTP Serve
	long lRetVal = -1;
	lRetVal = sl_NetAppStop(SL_NET_APP_HTTP_SERVER_ID);
	ASSERT_ON_ERROR( lRetVal);

	//Start Internal HTTP Server
	lRetVal = sl_NetAppStart(SL_NET_APP_HTTP_SERVER_ID);
	ASSERT_ON_ERROR( lRetVal);

	 /* Finished init the internet! */

	setRLED();

    /* Wait for operator */
    while(!readDIP1());

    clearRLED();

    /* Init motors (alongside motor GPIO, timers and interrupt */
    motorSetup();

    odometer_setup();

    /* Init IMU (alongside timers and interrupt */
    imu_setup();
    set_controller_parameters(kp, ki, kd);
    set__odo_controller_parameters(kp_odo, ki_odo, kd_odo);

    controller_setup();
    odometer_controller_setup();      // MUST be the last init called!


	while(1)
	{
		_SlNonOsMainLoopTask();
	}
}
