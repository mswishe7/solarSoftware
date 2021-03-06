/*
 *  suresine.c - This program reads all the RAM and EEPROM registers on a Moringstar SureSine-300 and prints the results.
 *  

/* Compile with: cc `pkg-config --cflags --libs libmodbus` suresine.c -o suresine */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <modbus.h>
#include <my_global.h>
#include <mysql.h>

#define SURESINE    0x02	/* MODBUS Address of the SureSine-300 */


void finisherror(MYSQL *con)
{
  fprintf(stderr, "%s\n", mysql_error(con));
  mysql_close(con);
  exit(1);
}

void finisherror(MYSQL*);

int main(int argc, char* argv[])
{
	modbus_t *ctx;
	int rc;
	float adc_battery_voltage; //Battery Voltage (Unfiltered) ------------ Stored in DB
	float adc_ac_output_current; //Ac Output Current (unfiltered) ------------ Stored in DB
	float adc_ths; //Heatsink Thermistor Voltage (unfiltered)
	float adc_remon;//Remote On Terminal Voltage (unfiltered)
	float Vb; //Battery Voltage (Slow Voltage) ------------ Stored in DB
	float Iac; //AC Output Current (filtered) ------------ Stored in DB
	float mod_index; //Modulation Index (slow) (0xFF = 100%)
	short Ths; //Heatsink Temperature (Celcius) ------------ Stored in DB
	unsigned short load_state; //Load State
	unsigned short volts; //Output Voltage Setting ------------ Stored in DB
	unsigned short  hertz; //Output Frequency Setting  ------------ Stored in DB
	unsigned short m_disconnect; //(Writeable) NON-ZERO value will turn off control
	unsigned short modbus_reset; //(Writeable) Writing NON-ZERO value will reset the control
	unsigned short fault; //Fault Bitfield
	unsigned short alarm; //Alarm Bitfield 
	unsigned short dip_switch; //DIP Switch Settings at Power On   Switch[1..8] in bits [0..7]
	float EVb_min; //Minimum Battery Voltage
	float EVb_max; //Maximum Battery Voltage 
	float EV_lvd2; //Low Voltage Disconnect 2
	float EV_lvr2; //Low Voltage Reconnect 2
	float EV_hvd2; //High Voltage Disconnect 2
	float EV_hvr2; //High Voltage Reconnect 2
	float Et_lvd_warn2; //LVD Warning Timer 2
	float EV_lvdwarn_beep2; //LVD Beeper Limit 2
	float EV_lvrwarn_beep2; //LVD Beeper Reset Limit 2
	float EV_startlvd2; //Custom start-up low voltage disconnect setpoint. Voltage at which the SureSine will start directly into the LVD state.
	unsigned short Emodbus_id; //Modbus Address/ID. Factory default = 1.
	unsigned short Emeter_id; //Morningstar Meterbus address. Factory Default = 1 
	unsigned short Ehourmeter; // [Hrs] Hours of Operation
	char Eserial_no[9]; //Serial Number of this SureSine-300 Inverter
	uint16_t data[20];
	int while_control; //Control the while loop
	int whilestop = 0;


	

	while_control = atoi(argv[argc-1]);
	if(while_control != 1)
	  while_control = 0;
	
	/* Set up a new MODBUS context */
	ctx = modbus_new_rtu("/dev/ttyUSB0", 9600, 'N', 8, 2);
	if (ctx == NULL) {
		fprintf(stderr, "Unable to create the libmodbus context\n");
		return -1;
	}
	
	/* Set the slave id to the SureSine-300 MODBUS id */
	modbus_set_slave(ctx, SURESINE);
	
	/* Open the MODBUS connection to the SureSine-300 */
    if (modbus_connect(ctx) == -1) {
        fprintf(stderr, "Connection failed: %s\n", modbus_strerror(errno));
        modbus_free(ctx);
        return -1;
    }
	
    while(whilestop == 0)
      {
	/* Read the RAM Registers and convert the results to their proper values */
	rc = modbus_read_registers(ctx, 0x0000, 17, data);
	
	printf("------------------\n");
	printf("RAM Registers\n");
	printf("------------------\n\n");
	
	adc_battery_voltage=data[0]*16.92/65536.0;
	printf("%-70s = %.2f V\n","Battery Voltage (unfiltered)",adc_battery_voltage);
	
	adc_ac_output_current=data[1]*16.92/32768.0;
	printf("%-70s = %.4f A\n","AC Output Current (unfiltered)",adc_ac_output_current);
	
	//Thermistor Voltage
	adc_ths=data[2]*16.92/65536.0;
	printf("%-70s = %.2f V\n","Heatsink Thermistor Voltage (unfiltered)",adc_ths);
	
	adc_remon=data[3]*16.92/65536.0;
	printf("%-70s = %.2f V\n","Remote On Terminal Voltage (unfiltered)",adc_remon);
	
	Vb=data[4]*16.92/65536.0;
	printf("%-70s = %.2f V\n","Battery Voltage (slow filter)",Vb);
	
	Iac=data[5]*16.92/32768.0;
	printf("%-70s = %.4f A\n","AC Output Current (filtered)",Iac);
	
	Ths=data[6];
	printf("%-70s = %d °C\n","Heatsink Temperature (Celcius)",Ths);
	
	fault=data[7];
	printf("%-70s = %d\n", "Suresine Fault Bitfield", fault);
	if (fault == 0) {
		printf("\tNo faults\n");
	} else {
		if (fault & 1) printf("\tReset\n");
		if ((fault & (1 << 1)) >> 1) printf("\tOver-current\n");
		if ((fault & (1 << 2)) >> 2) printf("\tnot used\n");
		if ((fault & (1 << 3)) >> 3) printf("\tSoftware\n");
		if ((fault & (1 << 4)) >> 4) printf("\tHVD\n");
		if ((fault & (1 << 5)) >> 5) printf("\tHot (heatsink temp. over 95 C)\n");
		if ((fault & (1 << 6)) >> 6) printf("\tDIP switch\n");
		if ((fault & (1 << 7)) >> 7) printf("\tSettings Edit\n");
	}
	
	alarm=data[8];
	printf("%-70s = %d\n","SureSine Alarm Bitfield", alarm);
	if (alarm == 0) {
		printf("\tNo alarms\n");
	} else {
		if (alarm & 1) printf("\tHeatsink temp. sensor open\n");
		if ((alarm & (1 << 1)) >> 1) printf("\tHeatsink temp. sensor shorted\n");
		if ((alarm & (1 << 2)) >> 2) printf("\tnot used\n");
		if ((alarm & (1 << 3)) >> 3) printf("\tHeatsink hot (above 80 C)\n");
		if ((alarm & (1 << 5)) >> 5) printf("\tHeatsink hot (above 80 C)\n");  /* On my system the heatsink hot alarm is the 5th bit ??? */
	}
	
	dip_switch=data[10];
	printf("dip_switch = DIP switch settings\n");
	if (dip_switch & 1) {
		printf("\tSwitch 1 ON - Power Mode = Standby Mode\n");
	} else {
		printf("\tSwitch 1 OFF - Power Mode = Always On\n");
	}
	if ((dip_switch & (1 << 1)) >> 1) {
		printf("\tSwitch 2 ON - LVD = 10.5 V, LVR = 11.6 V or custom settings\n");
	} else {
		printf("\tSwitch 2 OFF - LVD = 11.5 V, LVR = 12.6 V\n");
	}
	if ((dip_switch & (1 << 2)) >> 2) {
		printf("\tSwitch 3 ON - Beeper Warning Off\n");
	} else {
		printf("\tSwitch 3 OFF - Beeper Warning On\n");
	}
	if ((dip_switch & (1 << 3)) >> 3) {
		printf("\tSwitch 4 ON - MODBUS Protocol\n");
	} else {
		printf("\tSwitch 4 OFF - Meterbus Protocol\n");
	}
	
	load_state=data[11];
	switch (load_state) {
		case 0:
			printf("load_state = %d Start-up\n",load_state);
			break;
		case 1:
			printf("load_state = %d Load On\n",load_state);
			break;
		case 2:
			printf("load_state = %d LVD Warning\n",load_state);
			break;
		case 3:
			printf("load_state = %d LVD (Low Voltage Disconnect)\n",load_state);
			break;
		case 4:
			printf("load_state = %d Fault State\n",load_state);
			break;
		case 5:
			printf("load_state = %d Load Disconnected\n",load_state);
			break;
		case 6:
			printf("load_state = %d Load Off\n",load_state);
			break;
		case 7:
			printf("load_state = %d not used\n",load_state);
			break;
		case 8:
			printf("load_state = %d Standby\n",load_state);
			break;
	}
	
	mod_index=data[12]*100.0/256.0;
	printf("%-70s = %.2f %%\n","Modulation Index",mod_index);
	
	volts=data[13];
	printf("%-70s = %d V\n","Output Voltage Setting",volts);
	
	hertz=data[14];
	printf("%-70s = %d Hz\n","Output Frequency Setting",hertz);
	
	m_disconnect=data[15];
	printf("%-70s = %d\n","Write a NON-ZERO value here to turn off control",m_disconnect);
	
	modbus_reset=data[16];
	printf("%-70s = %d\n","Write a NON-ZERO value here to reset the control",modbus_reset);
	
	/* Read the EEPROM Registers and convert the results to their proper values */
	rc = modbus_read_registers(ctx, 0xE000, 12, data);
	
	printf("\n----------------\n");
	printf("EEPROM Registers\n");
	printf("-------------------\n\n");
	
	EVb_min=data[0]*16.92/65536.0;
	printf("%-70s = %.2f V\n","Minimum Battery Voltage",EVb_min);
	
	EVb_max=data[1]*16.92/65536.0;
	printf("%-70s = %.2f V\n","Maximum Battery Voltage",EVb_max);
	
	Emodbus_id=data[2];
	printf("%-70s = %d\n","SureSine Modbus ID",Emodbus_id);
	
	Emeter_id=data[3];
	printf("%-70s = %d\n","SureSine Meter ID",Emeter_id);
	
	EV_lvd2=data[4]*16.92/65536.0;
	printf("%-70s = %.2f V\n","Custom Low Voltage Disconnect Setpoint",EV_lvd2);
	
	EV_lvr2=data[5]*16.92/65536.0;
	printf("%-70s = %.2f V\n","Custom Low Voltage Reconnect Setpoint",EV_lvr2);
	
	EV_hvd2=data[6]*16.92/65536.0;
	printf("%-70s = %.2f V\n","Custom High Voltage Disconnect Setpoint",EV_hvd2);
	
	EV_hvr2=data[7]*16.92/65536.0;
	printf("%-70s = %.2f V\n","Custom High Voltage Reconnect Setpoint",EV_hvr2);
	
	Et_lvd_warn2=data[8]*0.1;
	printf("%-70s = %.1f s\n","Custom Low Voltage Disconnect Warning Threshold",Et_lvd_warn2);
	
	EV_lvdwarn_beep2=data[9]*16.92/65536.0;
	printf("%-70s = %.2f V\n","Custom Low Voltage Beeper Warning Threshold (Disable - 0)",EV_lvdwarn_beep2);
	
	EV_lvrwarn_beep2=data[10]*16.92/65536.0;
	printf("%-70s = %.2f V\n","Custom threshold voltage where beeper warning is reset (Disable - 0)",EV_lvrwarn_beep2);
	
	EV_startlvd2=data[11]*16.92/65536.0;
	printf("%-70s = %.2f V\n","Custom Start-Up Low Voltage Setpoint",EV_startlvd2);
	
	/* Read the EEPROM Registers and convert the results to their proper values */
	rc = modbus_read_registers(ctx, 0xE040, 8, data);
	
	Ehourmeter=data[0];
	printf("%-70s = %d %s\n","Hours of Operation",Ehourmeter,"hrs");
	
	Eserial_no[0]=data[4] & 0x00FF;
	Eserial_no[1]=data[4] >> 8;
	Eserial_no[2]=data[5] & 0x00FF;
	Eserial_no[3]=data[5] >> 8;
	Eserial_no[4]=data[6] & 0x00FF;
	Eserial_no[5]=data[6] >> 8;
	Eserial_no[6]=data[7] & 0x00FF;
	Eserial_no[7]=data[7] >> 8;
	Eserial_no[8]='\0';
	printf("%-20s = %s\n","Factory Serial Number",Eserial_no);
	
	if(while_control == 0)
	  whilestop = 1;
      
	/* Close the MODBUS connection */
	modbus_close(ctx);
	modbus_free(ctx);
	
	MYSQL *con = mysql_init(NULL);
	
	if (con == NULL)
	  {
	    fprintf(stderr,"%s\n", mysql_error(con));
	    exit(1);
	  }
	if (mysql_real_connect(con, "localhost", "root", "solar", "solar_tree", 0, NULL, 0) == NULL)
	  finisherror(con);

	//TEST VARIABLES
	int tint;
	float tfloat;
	unsigned short tushort;
	short tshort;

	tint = 32767;
	tfloat = 1234.567;
	tushort = 45000;
	tshort = 32766;
	char query[1024] = {0};

	sprintf(query,"INSERT into stats(BatteryVoltageUnfiltered, ACOutputCurrentUnfiltered, BatteryVoltageSlowFiltered, ACOutputCurrentFiltered, HeatsinkTemp, OutputVoltageSetting, OutputFrequencySetting) VALUES(%f, %f, %f, %f,  %hi, %i, %i)",adc_battery_voltage, adc_ac_output_current, Vb, Iac, Ths, volts, hertz);
	if (mysql_query(con,query))
	  finisherror(con);

	printf("MySQL client version: %s\n\n", mysql_get_client_info());
      }

	return(0);
}

