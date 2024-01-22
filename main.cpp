/* 
Autore: Fabbri Simone
Data: 14/11/2022
Titolo: Condizionatore di segnale celle di carico Modbus
Descrizione: Si implementa un condizionatore di segnale di 4 celle di carico in grado di condividere i dati via Modbus
Ultime modifiche: 
Comandi Modpoll utili:
-dyrectory: cd C:\Users\fabbr\Downloads\modpoll-3.10\win
-richiesta dati: modpoll -b 9600 -p none -m rtu -a 1 -r 1 -c 13 COM4
-scrittura pesoCalib: modpoll -b 9600 -p none -m rtu -a 1 -r 12 COM4 5000
-richiesta tara: modpoll -b 9600 -p none -m rtu -a 1 -r 1 -t 0 COM4 1
-richiesta calibrazione: modpoll -b 9600 -p none -m rtu -a 1 -r 9 -t 0 COM4 1
*Problemi:
    -Un dato su due scartato per settare il guadagno (possibile lettura alternata?)
ToDo:
    1. Differenziare celle staccate da celle attaccate;
    2. Modificare libreria HX711 per lettura dei due canali (no due Hx711, ma un HX711 con due canali);
    3. Gestire logging multithreading su scheda microSd;
    4. Comprendere eventuali situazioni di default.
*/

//INCLUSIONE LIBRERIE
#include <stdio.h>
#include <stdint.h>
#include "esp_err.h"
#include "mbcontroller.h"       // for mbcontroller defines and api
#include "modbus_params.h"      // for modbus parameters structures
#include "esp_log.h"            // for log_write
#include "sdkconfig.h"
#include "DAISY_PINOUT.hpp"
#include "HX711.hpp"
#include "COND.hpp"
#include "ALGO.hpp"
#include "PERSISTENCE.hpp"

#define times 1 //Numero medie per algoritmi di tara e calibrazione
static const char *SLAVE_TAG = "DAISY_COND_CISAGROUP";
static portMUX_TYPE param_lock = portMUX_INITIALIZER_UNLOCKED;  //?????
int err=0;
//#define DIAGNOSTIC_MODBUS

extern "C" void app_main(void)
{
    //-----------------------------SETUP MODBUS-----------------------------------------------------------------------------------------
    mb_param_info_t reg_info; // keeps the Modbus registers access information
    mb_communication_info_t comm_info; // Modbus communication parameters
    mb_register_area_descriptor_t reg_area; // Modbus register area descriptor structure
    // Set UART log level
    esp_log_level_set(SLAVE_TAG, ESP_LOG_INFO);
    void* mbc_slave_handler = NULL;
    ESP_ERROR_CHECK(mbc_slave_init(MB_PORT_SERIAL_SLAVE, &mbc_slave_handler)); // Initialization of Modbus controller
    //SETUP COMMUNICATION PARAMETERS
    comm_info.mode = MB_MODE_RTU,
    comm_info.slave_addr = MB_SLAVE_ADDR;
    comm_info.port = MB_PORT_NUM;
    comm_info.baudrate = MB_DEV_SPEED;
    comm_info.parity = MB_PARITY_NONE;
    ESP_ERROR_CHECK(mbc_slave_setup((void*)&comm_info));
    //SETUP REGISTER AREA
    reg_area.type = MB_PARAM_HOLDING; // Set type of register area
    reg_area.start_offset = MB_REG_HOLDING_START; // Offset of register area in Modbus protocol
    reg_area.address = (void*)&holding_reg_params.holding_cell1MS; // Set pointer to storage instance
    reg_area.size = sizeof(holding_reg_params);
    ESP_ERROR_CHECK(mbc_slave_set_descriptor(reg_area));
    // Initialization of Coils register area
    reg_area.type = MB_PARAM_COIL;
    reg_area.start_offset = MB_REG_COILS_START;
    reg_area.address = (void*)&coil_reg_params;
    reg_area.size = sizeof(coil_reg_params);
    ESP_ERROR_CHECK(mbc_slave_set_descriptor(reg_area));
    // Starts of modbus controller and stack
    ESP_ERROR_CHECK(mbc_slave_start());
    //UART SETTING
    ESP_ERROR_CHECK(uart_set_pin(MB_PORT_NUM, CONFIG_MB_UART_TXD,
                            CONFIG_MB_UART_RXD, CONFIG_MB_UART_RTS,
                            UART_PIN_NO_CHANGE));  // Set UART pin numbers
    ESP_ERROR_CHECK(uart_set_mode(MB_PORT_NUM, UART_MODE_RS485_HALF_DUPLEX)); // Set UART driver mode to Half Duplex
    ESP_LOGI(SLAVE_TAG, "Modbus slave stack initialized.");
    ESP_LOGI(SLAVE_TAG, "Start modbus test...");


    //-----------------------------------------------------PROTOCOL ROUTINE-----------------------------------------
    uint32_t cell1_data=0;
    uint32_t cell2_data=0;
    uint32_t cell3_data=0;
    uint32_t cell4_data=0;
    uint32_t pesoTot_data=0;
    uint32_t pesoCalib = 0;
    bool prevTare=0;
    bool prevCalib=0;
    holding_reg_params.diagnostic=0;
    //ADC1, ChanA (gain 64)
    HX711* cond1=new HX711();
    cond1->init(ADC1_DATA, ADC1_CLOCK);
    cond1->set_gain(eGAIN_64); 
    //ADC1, ChanB (gain 32)
    HX711* cond2=new HX711();
    cond2->init(ADC1_DATA, ADC1_CLOCK);
    cond2->set_gain(eGAIN_32); 
    //ADC2, ChanA (gain 64)
    HX711* cond3=new HX711();
    cond3->init(ADC2_DATA, ADC2_CLOCK);
    cond3->set_gain(eGAIN_64); 
    //ADC2, ChanB (gain 32)
    HX711* cond4=new HX711();
    cond4->init(ADC2_DATA, ADC2_CLOCK);
    cond4->set_gain(eGAIN_32); 
    //CONDIZIONATORE DI SEGNALE
    COND* bilancia=new COND();
    HX711** conds = (HX711**) malloc(sizeof(HX711*)*1);
    conds[0]=cond3;
    bilancia->init(conds, 1); //Uso solo 1 canali: ChanA_128
    //Defining a filtering algorithm
    ALGO* algo = new Dummy();
    bilancia->set_algo(algo);
    //Defining a memory saving system
    SPIFFS* mem = new SPIFFS();
    if(mem->systemOK()) bilancia->set_mem(mem);
    holding_reg_params.dummy=1;

    while(true) {
        //LETTURA CELLE DI CARICO ED AGGIORNAMENTO REGISTRI INTERNI
        pesoTot_data = bilancia->get_units(times);
        pesoCalib = holding_reg_params.holding_pesoCalibMS*(2^16) + holding_reg_params.holding_pesoCalibLS;
        cell1_data = bilancia->get_last_read(0);
        cell2_data = bilancia->get_last_read(1);
        cell3_data = bilancia->get_last_read(2);
        cell4_data = bilancia->get_last_read(3);
        printf("Peso TOT: %d\n", pesoTot_data);
        printf("Peso CALIB: %d\n", pesoCalib);
        printf("Cell1 data: %d\n", cell1_data);
        printf("Cell2 data: %d\n", cell2_data);
        printf("Cell3 data: %d\n", cell3_data);
        printf("Cell4 data: %d\n", cell4_data);

        portENTER_CRITICAL(&param_lock);
            holding_reg_params.holding_cell1LS=uint16_t(cell1_data);
            holding_reg_params.holding_cell1MS=cell1_data>>16;
            holding_reg_params.holding_cell2LS=uint16_t(cell2_data);
            holding_reg_params.holding_cell2MS=cell2_data>>16;
            holding_reg_params.holding_cell3LS=uint16_t(cell3_data);
            holding_reg_params.holding_cell3MS=cell3_data>>16;
            holding_reg_params.holding_cell4LS=uint16_t(cell4_data);
            holding_reg_params.holding_cell4MS=cell4_data>>16;
            holding_reg_params.holding_pesoTotLS=uint16_t(pesoTot_data);
            holding_reg_params.holding_pesoTotMS=pesoTot_data>>16;
            //Diagnosic code
            if(cell1_data==READ_ERROR_CODE) holding_reg_params.diagnostic |= 1UL << 0;
            else holding_reg_params.diagnostic &= ~(1UL << 0);
            if(cell2_data==READ_ERROR_CODE) holding_reg_params.diagnostic |= 1UL << 1;
            else holding_reg_params.diagnostic &= ~(1UL << 1);
            if(cell3_data==READ_ERROR_CODE) holding_reg_params.diagnostic |= 1UL << 2;
            else holding_reg_params.diagnostic &= ~(1UL << 2);
            if(cell4_data==READ_ERROR_CODE) holding_reg_params.diagnostic |= 1UL << 3;
            else holding_reg_params.diagnostic &= ~(1UL << 3);
            if(!mem->systemOK()) holding_reg_params.diagnostic |= 1UL << 4;
            else holding_reg_params.diagnostic &= ~(1UL << 4);
        portEXIT_CRITICAL(&param_lock);
        //DEFINIZIONE PROTOCOLLO DI CALIBRAZIONE
        //Intercettazione comando di tara
        if(coil_reg_params.coil_TareCommand==1) {
            if(prevTare==0) { //Rising edge
                bilancia->tare(times*10);
                printf("Tare Command requested!\n");
                prevTare=1;
            }
            else {
                //printf("Tare Command already requested!\n");
            }
        }
        else prevTare=0;
        //Intercettazione comando di calibrazione
        if(coil_reg_params.coil_CalibCommand==1) {
            if(prevCalib==0) { //Rising edge
                pesoCalib = holding_reg_params.holding_pesoCalibMS*(2^16) + holding_reg_params.holding_pesoCalibLS;
                bilancia->calib(pesoCalib, times*10);
                printf("Calib Command requested!: %f\n", bilancia->get_calfact());
                prevCalib=1;
            }
            else {
                //printf("Calib Command already requested!\n");
            }
        }
        else prevCalib=0;
        //LOGGING RICHIESTE SU DISPLAY
        #ifdef DIAGNOSTIC_MODBUS
        // Check for read/write events of Modbus master for certain events
        mb_event_group_t event = mbc_slave_check_event(mb_event_group_t(MB_READ_WRITE_MASK));
        const char* rw_str = (event & MB_READ_MASK) ? "READ" : "WRITE";
        // Filter events and process them accordingly
        if(event & (MB_EVENT_HOLDING_REG_WR | MB_EVENT_HOLDING_REG_RD)) {
            // Get parameter information from parameter queue
            ESP_ERROR_CHECK(mbc_slave_get_param_info(&reg_info, MB_PAR_INFO_GET_TOUT));
            ESP_LOGI(SLAVE_TAG, "HOLDING %s (%u us), ADDR:%u, TYPE:%u, INST_ADDR:0x%.4x, SIZE:%u",
                    rw_str,
                    (uint32_t)reg_info.time_stamp,
                    (uint32_t)reg_info.mb_offset,
                    (uint32_t)reg_info.type,
                    (uint32_t)reg_info.address,
                    (uint32_t)reg_info.size);
        } else if (event & (MB_EVENT_COILS_RD | MB_EVENT_COILS_WR)) {
            ESP_ERROR_CHECK(mbc_slave_get_param_info(&reg_info, MB_PAR_INFO_GET_TOUT));
            ESP_LOGI(SLAVE_TAG, "COILS %s (%u us), ADDR:%u, TYPE:%u, INST_ADDR:0x%.4x, SIZE:%u",
                                rw_str,
                                (uint32_t)reg_info.time_stamp,
                                (uint32_t)reg_info.mb_offset,
                                (uint32_t)reg_info.type,
                                (uint32_t)reg_info.address,
                                (uint32_t)reg_info.size);
            printf("Coil Tare: %d\n", coil_reg_params.coil_TareCommand);
            printf("Coil Calib: %d\n", coil_reg_params.coil_CalibCommand);
        }
        #endif
    }
    // Destroy of Modbus controller on alarm
    ESP_LOGI(SLAVE_TAG,"Modbus controller destroyed.");
    vTaskDelay(100);
    ESP_ERROR_CHECK(mbc_slave_destroy());
    //Deallocate data structures
    free(conds);
    algo->free_mem();
    bilancia->free_mem();
}