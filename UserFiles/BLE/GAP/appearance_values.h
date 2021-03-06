

#ifndef APPEARANCE_VALUES_H_
#define APPEARANCE_VALUES_H_


/****************************************************************/
/* Includes                                                     */
/****************************************************************/
#include "Types.h"


/****************************************************************/
/* Defines                                                      */
/****************************************************************/
/* Appearance Values according to Bluetooth Assigned Numbers Revision Date: 2021�04�20 */
#define PARSE_APPEARANCE_VALUE(Category,Sub_category) ( ((Category) << 6) | ((Sub_category) & 0x3F) )


typedef enum
{
	UNKNOWN 					= 0x000,
	PHONE  						= 0x001,
	COMPUTER 					= 0x002,
	WATCH						= 0x003,
	CLOCK						= 0x004,
	DISPLAY						= 0x005,
	REMOTE_CONTROL 				= 0x006,
	EYE_GLASSES					= 0x007,
	TAG 						= 0x008,
	KEYRING 					= 0x009,
	MEDIA_PLAYER 				= 0x00A,
	BARCODE_SCANNER 			= 0x00B,
	THERMOMETER 				= 0x00C,
	HEART_RATE_SENSOR 			= 0x00D,
	BLOOD_PRESSURE 				= 0x00E,
	HUMAN_INTERFACE_DEVICE 		= 0x00F,
	GLUCOSE_METER 				= 0x010,
	RUNNING_WALKING_SENSOR 		= 0x011,
	CYCLING 					= 0x012,
	CONTROL_DEVICE 				= 0x013,
	NETWORK_DEVICE 				= 0x014,
	SENSOR 						= 0x015,
	LIGHT_FIXTURES 				= 0x016,
	FAN 						= 0x017,
	HVAC						= 0x018,
	AIR_CONDITIONING 			= 0x019,
	HUMIDIFIER 					= 0x01A,
	HEATING 					= 0x01B,
	ACCESS_CONTROL 				= 0x01C,
	MOTORIZED_DEVICE 			= 0x01D,
	POWER_DEVICE 				= 0x01E,
	LIGHT_SOURCE 				= 0x01F,
	WINDOW_COVERING 			= 0x020,
	AUDIO_SINK 					= 0x021,
	AUDIO_SOURCE 				= 0x022,
	MOTORIZED_VEHICLE 			= 0x023,
	DOMESTIC_APPLIANCE 			= 0x024,
	WEARABLE_AUDIO_DEVICE 		= 0x025,
	AIRCRAFT 					= 0x026,
	AV_EQUIPMENT 				= 0x027,
	DISPLAY_EQUIPMENT 			= 0x028,
	HEARING_AID 				= 0x029,
	GAMING 						= 0x02A,
	SIGNAGE 					= 0x02B,
	PULSE_OXIMETER				= 0x031,
	WEIGHT_SCALE 				= 0x032,
	PERSONAL_MOBILITY_DEVICE 	= 0x033,
	CONTINUOUS_GLUCOSE_MONITOR 	= 0x034,
	INSULIN_PUMP 				= 0x035,
	MEDICATION_DELIVERY 		= 0x036,
	OUTDOOR_SPORTS_ACTIVITY 	= 0x051,
}GAP_APPEARANCE_CATEGORY;


typedef enum
{
	GENERIC_UNKNOWN						= (PARSE_APPEARANCE_VALUE( UNKNOWN,  					0x00 )),

	GENERIC_PHONE						= (PARSE_APPEARANCE_VALUE( PHONE,    					0x00 )),

	GENERIC_COMPUTER					= (PARSE_APPEARANCE_VALUE( COMPUTER, 					0x00 )),
	DESKTOP_WORKSTATION					= (PARSE_APPEARANCE_VALUE( COMPUTER, 					0x01 )),
	SERVER_CLASS_COMPUTER				= (PARSE_APPEARANCE_VALUE( COMPUTER, 					0x02 )),
	LAPTOP								= (PARSE_APPEARANCE_VALUE( COMPUTER, 					0x03 )),
	HANDHELD_PC_PDA_CLAMSHELL			= (PARSE_APPEARANCE_VALUE( COMPUTER, 					0x04 )),
	PALMSIZE_PC_PDA						= (PARSE_APPEARANCE_VALUE( COMPUTER, 					0x05 )),
	WEARABLE_COMPUTER					= (PARSE_APPEARANCE_VALUE( COMPUTER, 					0x06 )),
	TABLET								= (PARSE_APPEARANCE_VALUE( COMPUTER, 					0x07 )),
	DOCKING_STATION						= (PARSE_APPEARANCE_VALUE( COMPUTER, 					0x08 )),
	ALL_IN_ONE							= (PARSE_APPEARANCE_VALUE( COMPUTER, 					0x09 )),
	BLADE_SERVER						= (PARSE_APPEARANCE_VALUE( COMPUTER, 					0x0A )),
	CONVERTIBLE							= (PARSE_APPEARANCE_VALUE( COMPUTER, 					0x0B )),
	DETACHABLE							= (PARSE_APPEARANCE_VALUE( COMPUTER, 					0x0C )),
	IOT_GATEWAY							= (PARSE_APPEARANCE_VALUE( COMPUTER, 					0x0D )),
	MINI_PC								= (PARSE_APPEARANCE_VALUE( COMPUTER, 					0x0E )),
	STICK_PC							= (PARSE_APPEARANCE_VALUE( COMPUTER, 					0x0F )),

	GENERIC_WATCH						= (PARSE_APPEARANCE_VALUE( WATCH, 	 					0x00 )),
	SPORTS_WATCH						= (PARSE_APPEARANCE_VALUE( WATCH, 	 					0x01 )),
	SMARTWATCH							= (PARSE_APPEARANCE_VALUE( WATCH, 	 					0x02 )),

	GENERIC_CLOCK						= (PARSE_APPEARANCE_VALUE( CLOCK, 	 					0x00 )),

	GENERIC_DISPLAY						= (PARSE_APPEARANCE_VALUE( DISPLAY,  					0x00 )),

	GENERIC_REMOTE_CONTROL				= (PARSE_APPEARANCE_VALUE( REMOTE_CONTROL, 				0x00 )),

	GENERIC_EYE_GLASSES					= (PARSE_APPEARANCE_VALUE( EYE_GLASSES,    				0x00 )),

	GENERIC_TAG							= (PARSE_APPEARANCE_VALUE( TAG, 	 					0x00 )),

	GENERIC_KEYRING						= (PARSE_APPEARANCE_VALUE( KEYRING, 	 				0x00 )),

	GENERIC_MEDIA_PLAYER				= (PARSE_APPEARANCE_VALUE( MEDIA_PLAYER, 				0x00 )),

	GENERIC_BARCODE_SCANNER				= (PARSE_APPEARANCE_VALUE( BARCODE_SCANNER, 			0x00 )),

	GENERIC_THERMOMETER					= (PARSE_APPEARANCE_VALUE( THERMOMETER, 				0x00 )),
	EAR_THERMOMETER						= (PARSE_APPEARANCE_VALUE( THERMOMETER, 				0x01 )),

	GENERIC_HEART_RATE_SENSOR			= (PARSE_APPEARANCE_VALUE( HEART_RATE_SENSOR, 			0x00 )),
	HEART_RATE_BELT						= (PARSE_APPEARANCE_VALUE( HEART_RATE_SENSOR, 			0x01 )),

	GENERIC_BLOOD_PRESSURE				= (PARSE_APPEARANCE_VALUE( BLOOD_PRESSURE, 				0x00 )),
	ARM_BLOOD_PRESSURE					= (PARSE_APPEARANCE_VALUE( BLOOD_PRESSURE, 				0x01 )),
	WRIST_BLOOD_PRESSURE				= (PARSE_APPEARANCE_VALUE( BLOOD_PRESSURE, 				0x02 )),

	GENERIC_HUMAN_INTERFACE_DEVICE 		= (PARSE_APPEARANCE_VALUE( HUMAN_INTERFACE_DEVICE, 		0x00 )),
	KEYBOARD							= (PARSE_APPEARANCE_VALUE( HUMAN_INTERFACE_DEVICE, 		0x01 )),
	MOUSE								= (PARSE_APPEARANCE_VALUE( HUMAN_INTERFACE_DEVICE, 		0x02 )),
	JOYSTICK							= (PARSE_APPEARANCE_VALUE( HUMAN_INTERFACE_DEVICE, 		0x03 )),
	GAMEPAD								= (PARSE_APPEARANCE_VALUE( HUMAN_INTERFACE_DEVICE, 		0x04 )),
	DIGITIZER_TABLET					= (PARSE_APPEARANCE_VALUE( HUMAN_INTERFACE_DEVICE, 		0x05 )),
	CARD_READER							= (PARSE_APPEARANCE_VALUE( HUMAN_INTERFACE_DEVICE, 		0x06 )),
	DIGITAL_PEN							= (PARSE_APPEARANCE_VALUE( HUMAN_INTERFACE_DEVICE, 		0x07 )),
	BARCODE_SCANNER_HID					= (PARSE_APPEARANCE_VALUE( HUMAN_INTERFACE_DEVICE, 		0x08 )),

	GENERIC_GLUCOSE_METER				= (PARSE_APPEARANCE_VALUE( GLUCOSE_METER, 				0x00 )),

	GENERIC_RUNNING_WALKING_SENSOR  	= (PARSE_APPEARANCE_VALUE( RUNNING_WALKING_SENSOR, 		0x00 )),
	IN_SHOE_RUNNING_WALKING_SENSOR  	= (PARSE_APPEARANCE_VALUE( RUNNING_WALKING_SENSOR, 		0x01 )),
	ON_SHOE_RUNNING_WALKING_SENSOR  	= (PARSE_APPEARANCE_VALUE( RUNNING_WALKING_SENSOR, 		0x02 )),
	ON_HIP_RUNNING_WALKING_SENSOR   	= (PARSE_APPEARANCE_VALUE( RUNNING_WALKING_SENSOR, 		0x03 )),

	GENERIC_CYCLING						= (PARSE_APPEARANCE_VALUE( CYCLING, 					0x00 )),
	CYCLING_COMPUTER					= (PARSE_APPEARANCE_VALUE( CYCLING, 					0x01 )),
	SPEED_SENSOR						= (PARSE_APPEARANCE_VALUE( CYCLING, 					0x02 )),
	CADENCE_SENSOR						= (PARSE_APPEARANCE_VALUE( CYCLING, 					0x03 )),
	POWER_SENSOR						= (PARSE_APPEARANCE_VALUE( CYCLING, 					0x04 )),
	SPEED_AND_CADENCE_SENSOR			= (PARSE_APPEARANCE_VALUE( CYCLING, 					0x05 )),

	GENERIC_CONTROL_DEVICE				= (PARSE_APPEARANCE_VALUE( CONTROL_DEVICE, 				0x00 )),
	SWITCH								= (PARSE_APPEARANCE_VALUE( CONTROL_DEVICE, 				0x01 )),
	MULTI_SWITCH						= (PARSE_APPEARANCE_VALUE( CONTROL_DEVICE, 				0x02 )),
	BUTTON								= (PARSE_APPEARANCE_VALUE( CONTROL_DEVICE, 				0x03 )),
	SLIDER								= (PARSE_APPEARANCE_VALUE( CONTROL_DEVICE, 				0x04 )),
	ROTARY_SWITCH						= (PARSE_APPEARANCE_VALUE( CONTROL_DEVICE, 				0x05 )),
	TOUCH_PANEL							= (PARSE_APPEARANCE_VALUE( CONTROL_DEVICE, 				0x06 )),
	SINGLE_SWITCH						= (PARSE_APPEARANCE_VALUE( CONTROL_DEVICE, 				0x07 )),
	DOUBLE_SWITCH						= (PARSE_APPEARANCE_VALUE( CONTROL_DEVICE, 				0x08 )),
	TRIPLE_SWITCH						= (PARSE_APPEARANCE_VALUE( CONTROL_DEVICE, 				0x09 )),
	BATTERY_SWITCH						= (PARSE_APPEARANCE_VALUE( CONTROL_DEVICE, 				0x0A )),
	ENERGY_HARVESTING_SWITCH			= (PARSE_APPEARANCE_VALUE( CONTROL_DEVICE, 				0x0B )),
	PUSH_BUTTON							= (PARSE_APPEARANCE_VALUE( CONTROL_DEVICE, 				0x0C )),

	GENERIC_NETWORK_DEVICE				= (PARSE_APPEARANCE_VALUE( NETWORK_DEVICE, 				0x00 )),
	ACCESS_POINT						= (PARSE_APPEARANCE_VALUE( NETWORK_DEVICE, 				0x01 )),
	MESH_DEVICE							= (PARSE_APPEARANCE_VALUE( NETWORK_DEVICE, 				0x02 )),
	MESH_NETWORK_PROXY					= (PARSE_APPEARANCE_VALUE( NETWORK_DEVICE, 				0x03 )),

	GENERIC_SENSOR						= (PARSE_APPEARANCE_VALUE( SENSOR, 						0x00 )),
	MOTION_SENSOR						= (PARSE_APPEARANCE_VALUE( SENSOR, 						0x01 )),
	AIR_QUALITY_SENSOR					= (PARSE_APPEARANCE_VALUE( SENSOR, 						0x02 )),
	TEMPERATURE_SENSOR					= (PARSE_APPEARANCE_VALUE( SENSOR, 						0x03 )),
	HUMIDITY_SENSOR						= (PARSE_APPEARANCE_VALUE( SENSOR, 						0x04 )),
	LEAK_SENSOR							= (PARSE_APPEARANCE_VALUE( SENSOR, 						0x05 )),
	SMOKE_SENSOR						= (PARSE_APPEARANCE_VALUE( SENSOR, 						0x06 )),
	OCCUPANCY_SENSOR					= (PARSE_APPEARANCE_VALUE( SENSOR, 						0x07 )),
	CONTACT_SENSOR						= (PARSE_APPEARANCE_VALUE( SENSOR, 						0x08 )),
	CARBON_MONOXIDE_SENSOR				= (PARSE_APPEARANCE_VALUE( SENSOR, 						0x09 )),
	CARBON_DIOXIDE_SENSOR				= (PARSE_APPEARANCE_VALUE( SENSOR, 						0x0A )),
	AMBIENT_LIGHT_SENSOR				= (PARSE_APPEARANCE_VALUE( SENSOR, 						0x0B )),
	ENERGY_SENSOR						= (PARSE_APPEARANCE_VALUE( SENSOR, 						0x0C )),
	COLOR_LIGHT_SENSOR					= (PARSE_APPEARANCE_VALUE( SENSOR, 						0x0D )),
	RAIN_SENSOR							= (PARSE_APPEARANCE_VALUE( SENSOR, 						0x0E )),
	FIRE_SENSOR							= (PARSE_APPEARANCE_VALUE( SENSOR, 						0x0F )),
	WIND_SENSOR							= (PARSE_APPEARANCE_VALUE( SENSOR, 						0x10 )),
	PROXIMITY_SENSOR					= (PARSE_APPEARANCE_VALUE( SENSOR, 						0x11 )),
	MULTI_SENSOR						= (PARSE_APPEARANCE_VALUE( SENSOR, 						0x12 )),
	FLUSH_MOUNTED_SENSOR				= (PARSE_APPEARANCE_VALUE( SENSOR, 						0x13 )),
	CEILING_MOUNTED_SENSOR				= (PARSE_APPEARANCE_VALUE( SENSOR, 						0x14 )),
	WALL_MOUNTED_SENSOR					= (PARSE_APPEARANCE_VALUE( SENSOR, 						0x15 )),
	MULTISENSOR							= (PARSE_APPEARANCE_VALUE( SENSOR, 						0x16 )),
	ENERGY_METER						= (PARSE_APPEARANCE_VALUE( SENSOR, 						0x17 )),
	FLAME_DETECTOR						= (PARSE_APPEARANCE_VALUE( SENSOR, 						0x18 )),
	VEHICLE_TIRE_PRESSURE_SENSOR		= (PARSE_APPEARANCE_VALUE( SENSOR, 						0x19 )),

	GENERIC_LIGHT_FIXTURES				= (PARSE_APPEARANCE_VALUE( LIGHT_FIXTURES, 				0x00 )),
	WALL_LIGHT							= (PARSE_APPEARANCE_VALUE( LIGHT_FIXTURES, 				0x01 )),
	CEILING_LIGHT						= (PARSE_APPEARANCE_VALUE( LIGHT_FIXTURES, 				0x02 )),
	FLOOR_LIGHT							= (PARSE_APPEARANCE_VALUE( LIGHT_FIXTURES, 				0x03 )),
	CABINET_LIGHT						= (PARSE_APPEARANCE_VALUE( LIGHT_FIXTURES, 				0x04 )),
	DESK_LIGHT							= (PARSE_APPEARANCE_VALUE( LIGHT_FIXTURES, 				0x05 )),
	TROFFER_LIGHT						= (PARSE_APPEARANCE_VALUE( LIGHT_FIXTURES, 				0x06 )),
	PENDANT_LIGHT						= (PARSE_APPEARANCE_VALUE( LIGHT_FIXTURES, 				0x07 )),
	IN_GROUND_LIGHT						= (PARSE_APPEARANCE_VALUE( LIGHT_FIXTURES, 				0x08 )),
	FLOOD_LIGHT							= (PARSE_APPEARANCE_VALUE( LIGHT_FIXTURES, 				0x09 )),
	UNDERWATER_LIGHT					= (PARSE_APPEARANCE_VALUE( LIGHT_FIXTURES, 				0x0A )),
	BOLLARD_WITH_LIGHT					= (PARSE_APPEARANCE_VALUE( LIGHT_FIXTURES, 				0x0B )),
	PATHWAY_LIGHT						= (PARSE_APPEARANCE_VALUE( LIGHT_FIXTURES, 				0x0C )),
	GARDEN_LIGHT						= (PARSE_APPEARANCE_VALUE( LIGHT_FIXTURES, 				0x0D )),
	POLE_TOP_LIGHT						= (PARSE_APPEARANCE_VALUE( LIGHT_FIXTURES, 				0x0E )),
	SPOTLIGHT							= (PARSE_APPEARANCE_VALUE( LIGHT_FIXTURES, 				0x0F )),
	LINEAR_LIGHT						= (PARSE_APPEARANCE_VALUE( LIGHT_FIXTURES, 				0x10 )),
	STREET_LIGHT						= (PARSE_APPEARANCE_VALUE( LIGHT_FIXTURES, 				0x11 )),
	SHELVES_LIGHT						= (PARSE_APPEARANCE_VALUE( LIGHT_FIXTURES, 				0x12 )),
	BAY_LIGHT							= (PARSE_APPEARANCE_VALUE( LIGHT_FIXTURES, 				0x13 )),
	EMERGENCY_EXIT_LIGHT				= (PARSE_APPEARANCE_VALUE( LIGHT_FIXTURES, 				0x14 )),
	LIGHT_CONTROLLER					= (PARSE_APPEARANCE_VALUE( LIGHT_FIXTURES, 				0x15 )),
	LIGHT_DRIVER						= (PARSE_APPEARANCE_VALUE( LIGHT_FIXTURES, 				0x16 )),
	BULB								= (PARSE_APPEARANCE_VALUE( LIGHT_FIXTURES, 				0x17 )),
	LOWBAY_LIGHT						= (PARSE_APPEARANCE_VALUE( LIGHT_FIXTURES, 				0x18 )),
	HIGHBAY_LIGHT						= (PARSE_APPEARANCE_VALUE( LIGHT_FIXTURES, 				0x19 )),

	GENERIC_FAN							= (PARSE_APPEARANCE_VALUE( FAN, 						0x00 )),
	CEILING_FAN							= (PARSE_APPEARANCE_VALUE( FAN, 						0x01 )),
	AXIAL_FAN							= (PARSE_APPEARANCE_VALUE( FAN, 						0x02 )),
	EXHAUST_FAN							= (PARSE_APPEARANCE_VALUE( FAN, 						0x03 )),
	PEDESTAL_FAN						= (PARSE_APPEARANCE_VALUE( FAN, 						0x04 )),
	DESK_FAN							= (PARSE_APPEARANCE_VALUE( FAN, 						0x05 )),
	WALL_FAN							= (PARSE_APPEARANCE_VALUE( FAN, 						0x06 )),

	GENERIC_HVAC						= (PARSE_APPEARANCE_VALUE( HVAC, 						0x00 )),
	THERMOSTAT							= (PARSE_APPEARANCE_VALUE( HVAC, 						0x01 )),
	HUMIDIFIER_HVAC						= (PARSE_APPEARANCE_VALUE( HVAC, 						0x02 )),
	DE_HUMIDIFIER						= (PARSE_APPEARANCE_VALUE( HVAC, 						0x03 )),
	HEATER								= (PARSE_APPEARANCE_VALUE( HVAC, 						0x04 )),
	RADIATOR_HVAC						= (PARSE_APPEARANCE_VALUE( HVAC, 						0x05 )),
	BOILER_HVAC							= (PARSE_APPEARANCE_VALUE( HVAC, 						0x06 )),
	HEAT_PUMP_HVAC						= (PARSE_APPEARANCE_VALUE( HVAC, 						0x07 )),
	INFRARED_HEATER_HVAC				= (PARSE_APPEARANCE_VALUE( HVAC, 						0x08 )),
	RADIANT_PANEL_HEATER_HVAC			= (PARSE_APPEARANCE_VALUE( HVAC, 						0x09 )),
	FAN_HEATER_HVAC						= (PARSE_APPEARANCE_VALUE( HVAC, 						0x0A )),
	AIR_CURTAIN_HVAC					= (PARSE_APPEARANCE_VALUE( HVAC, 						0x0B )),

	GENERIC_AIR_CONDITIONING 			= (PARSE_APPEARANCE_VALUE( AIR_CONDITIONING, 			0x00 )),

	GENERIC_HUMIDIFIER		 			= (PARSE_APPEARANCE_VALUE( HUMIDIFIER, 					0x00 )),

	GENERIC_HEATING						= (PARSE_APPEARANCE_VALUE( HEATING, 					0x00 )),
	RADIATOR							= (PARSE_APPEARANCE_VALUE( HEATING, 					0x01 )),
	BOILER								= (PARSE_APPEARANCE_VALUE( HEATING, 					0x02 )),
	HEAT_PUMP							= (PARSE_APPEARANCE_VALUE( HEATING, 					0x03 )),
	INFRARED_HEATER						= (PARSE_APPEARANCE_VALUE( HEATING, 					0x04 )),
	RADIANT_PANEL_HEATER				= (PARSE_APPEARANCE_VALUE( HEATING, 					0x05 )),
	FAN_HEATER							= (PARSE_APPEARANCE_VALUE( HEATING, 					0x06 )),
	AIR_CURTAIN							= (PARSE_APPEARANCE_VALUE( HEATING, 					0x07 )),

	GENERIC_ACCESS_CONTROL				= (PARSE_APPEARANCE_VALUE( ACCESS_CONTROL, 				0x00 )),
	ACCESS_DOOR							= (PARSE_APPEARANCE_VALUE( ACCESS_CONTROL, 				0x01 )),
	GARAGE_DOOR							= (PARSE_APPEARANCE_VALUE( ACCESS_CONTROL, 				0x02 )),
	EMERGENCY_EXIT_DOOR					= (PARSE_APPEARANCE_VALUE( ACCESS_CONTROL, 				0x03 )),
	ACCESS_LOCK							= (PARSE_APPEARANCE_VALUE( ACCESS_CONTROL, 				0x04 )),
	ELEVATOR							= (PARSE_APPEARANCE_VALUE( ACCESS_CONTROL, 				0x05 )),
	WINDOW								= (PARSE_APPEARANCE_VALUE( ACCESS_CONTROL, 				0x06 )),
	ENTRANCE_GATE						= (PARSE_APPEARANCE_VALUE( ACCESS_CONTROL, 				0x07 )),
	DOOR_LOCK							= (PARSE_APPEARANCE_VALUE( ACCESS_CONTROL, 				0x08 )),
	LOCKER								= (PARSE_APPEARANCE_VALUE( ACCESS_CONTROL, 				0x09 )),

	GENERIC_MOTORIZED_DEVICE			= (PARSE_APPEARANCE_VALUE( MOTORIZED_DEVICE, 			0x00 )),
	MOTORIZED_GATE						= (PARSE_APPEARANCE_VALUE( MOTORIZED_DEVICE, 			0x01 )),
	AWNING								= (PARSE_APPEARANCE_VALUE( MOTORIZED_DEVICE, 			0x02 )),
	BLINDS_OR_SHADES					= (PARSE_APPEARANCE_VALUE( MOTORIZED_DEVICE, 			0x03 )),
	CURTAINS							= (PARSE_APPEARANCE_VALUE( MOTORIZED_DEVICE, 			0x04 )),
	SCREEN								= (PARSE_APPEARANCE_VALUE( MOTORIZED_DEVICE, 			0x05 )),

	GENERIC_POWER_DEVICE				= (PARSE_APPEARANCE_VALUE( POWER_DEVICE, 				0x00 )),
	POWER_OUTLET						= (PARSE_APPEARANCE_VALUE( POWER_DEVICE, 				0x01 )),
	POWER_STRIP							= (PARSE_APPEARANCE_VALUE( POWER_DEVICE, 				0x02 )),
	PLUG								= (PARSE_APPEARANCE_VALUE( POWER_DEVICE, 				0x03 )),
	POWER_SUPPLY						= (PARSE_APPEARANCE_VALUE( POWER_DEVICE, 				0x04 )),
	LED_DRIVER							= (PARSE_APPEARANCE_VALUE( POWER_DEVICE, 				0x05 )),
	FLUORESCENT_LAMP_GEAR				= (PARSE_APPEARANCE_VALUE( POWER_DEVICE, 				0x06 )),
	HID_LAMP_GEAR						= (PARSE_APPEARANCE_VALUE( POWER_DEVICE, 				0x07 )),
	CHARGE_CASE							= (PARSE_APPEARANCE_VALUE( POWER_DEVICE, 				0x08 )),
	POWER_BANK							= (PARSE_APPEARANCE_VALUE( POWER_DEVICE, 				0x09 )),

	GENERIC_LIGHT_SOURCE				= (PARSE_APPEARANCE_VALUE( LIGHT_SOURCE, 				0x00 )),
	INCANDESCENT_LIGHT_BULB				= (PARSE_APPEARANCE_VALUE( LIGHT_SOURCE, 				0x01 )),
	LED_LAMP							= (PARSE_APPEARANCE_VALUE( LIGHT_SOURCE, 				0x02 )),
	HID_LAMP							= (PARSE_APPEARANCE_VALUE( LIGHT_SOURCE, 				0x03 )),
	FLUORESCENT_LAMP					= (PARSE_APPEARANCE_VALUE( LIGHT_SOURCE, 				0x04 )),
	LED_ARRAY							= (PARSE_APPEARANCE_VALUE( LIGHT_SOURCE, 				0x05 )),
	MULTI_COLOR_LED_ARRAY				= (PARSE_APPEARANCE_VALUE( LIGHT_SOURCE, 				0x06 )),

	GENERIC_WINDOW_COVERING				= (PARSE_APPEARANCE_VALUE( WINDOW_COVERING, 			0x00 )),
	WINDOW_SHADES						= (PARSE_APPEARANCE_VALUE( WINDOW_COVERING, 			0x01 )),
	WINDOW_BLINDS						= (PARSE_APPEARANCE_VALUE( WINDOW_COVERING, 			0x02 )),
	WINDOW_AWNING						= (PARSE_APPEARANCE_VALUE( WINDOW_COVERING, 			0x03 )),
	WINDOW_CURTAIN						= (PARSE_APPEARANCE_VALUE( WINDOW_COVERING, 			0x04 )),
	EXTERIOR_SHUTTER					= (PARSE_APPEARANCE_VALUE( WINDOW_COVERING, 			0x05 )),
	EXTERIOR_SCREEN						= (PARSE_APPEARANCE_VALUE( WINDOW_COVERING, 			0x06 )),

	GENERIC_AUDIO_SINK					= (PARSE_APPEARANCE_VALUE( AUDIO_SINK, 					0x00 )),
	STANDALONE_SPEAKER					= (PARSE_APPEARANCE_VALUE( AUDIO_SINK, 					0x01 )),
	SOUNDBAR							= (PARSE_APPEARANCE_VALUE( AUDIO_SINK, 					0x02 )),
	BOOKSHELF_SPEAKER					= (PARSE_APPEARANCE_VALUE( AUDIO_SINK, 					0x03 )),
	STANDMOUNTED_SPEAKER				= (PARSE_APPEARANCE_VALUE( AUDIO_SINK, 					0x04 )),
	SPEAKERPHONE						= (PARSE_APPEARANCE_VALUE( AUDIO_SINK, 					0x05 )),

	GENERIC_AUDIO_SOURCE				= (PARSE_APPEARANCE_VALUE( AUDIO_SOURCE, 				0x00 )),
	MICROPHONE							= (PARSE_APPEARANCE_VALUE( AUDIO_SOURCE, 				0x01 )),
	ALARM								= (PARSE_APPEARANCE_VALUE( AUDIO_SOURCE, 				0x02 )),
	BELL								= (PARSE_APPEARANCE_VALUE( AUDIO_SOURCE, 				0x03 )),
	HORN								= (PARSE_APPEARANCE_VALUE( AUDIO_SOURCE, 				0x04 )),
	BROADCASTING_DEVICE					= (PARSE_APPEARANCE_VALUE( AUDIO_SOURCE, 				0x05 )),
	SERVICE_DESK						= (PARSE_APPEARANCE_VALUE( AUDIO_SOURCE, 				0x06 )),
	KIOSK								= (PARSE_APPEARANCE_VALUE( AUDIO_SOURCE, 				0x07 )),
	BROADCASTING_ROOM					= (PARSE_APPEARANCE_VALUE( AUDIO_SOURCE, 				0x08 )),
	AUDITORIUM							= (PARSE_APPEARANCE_VALUE( AUDIO_SOURCE, 				0x09 )),

	GENERIC_MOTORIZED_VEHICLE			= (PARSE_APPEARANCE_VALUE( MOTORIZED_VEHICLE, 			0x00 )),
	CAR									= (PARSE_APPEARANCE_VALUE( MOTORIZED_VEHICLE, 			0x01 )),
	LARGE_GOODS_VEHICLE					= (PARSE_APPEARANCE_VALUE( MOTORIZED_VEHICLE, 			0x02 )),
	TWO_WHEELED_VEHICLE					= (PARSE_APPEARANCE_VALUE( MOTORIZED_VEHICLE, 			0x03 )),
	MOTORBIKE							= (PARSE_APPEARANCE_VALUE( MOTORIZED_VEHICLE, 			0x04 )),
	SCOOTER								= (PARSE_APPEARANCE_VALUE( MOTORIZED_VEHICLE, 			0x05 )),
	MOPED								= (PARSE_APPEARANCE_VALUE( MOTORIZED_VEHICLE, 			0x06 )),
	THREE_WHEELED_VEHICLE				= (PARSE_APPEARANCE_VALUE( MOTORIZED_VEHICLE, 			0x07 )),
	LIGHT_VEHICLE						= (PARSE_APPEARANCE_VALUE( MOTORIZED_VEHICLE, 			0x08 )),
	QUAD_BIKE							= (PARSE_APPEARANCE_VALUE( MOTORIZED_VEHICLE, 			0x09 )),
	MINIBUS								= (PARSE_APPEARANCE_VALUE( MOTORIZED_VEHICLE, 			0x0A )),
	BUS									= (PARSE_APPEARANCE_VALUE( MOTORIZED_VEHICLE, 			0x0B )),
	TROLLEY								= (PARSE_APPEARANCE_VALUE( MOTORIZED_VEHICLE, 			0x0C )),
	AGRICULTURAL_VEHICLE				= (PARSE_APPEARANCE_VALUE( MOTORIZED_VEHICLE, 			0x0D )),
	CAMPER_CARAVAN						= (PARSE_APPEARANCE_VALUE( MOTORIZED_VEHICLE, 			0x0E )),
	RECREATIONAL_VEHICLE_MOTOR_HOME		= (PARSE_APPEARANCE_VALUE( MOTORIZED_VEHICLE, 			0x0F )),

	GENERIC_DOMESTIC_APPLIANCE			= (PARSE_APPEARANCE_VALUE( DOMESTIC_APPLIANCE, 			0x00 )),
	REFRIGERATOR						= (PARSE_APPEARANCE_VALUE( DOMESTIC_APPLIANCE, 			0x01 )),
	FREEZER								= (PARSE_APPEARANCE_VALUE( DOMESTIC_APPLIANCE, 			0x02 )),
	OVEN								= (PARSE_APPEARANCE_VALUE( DOMESTIC_APPLIANCE, 			0x03 )),
	MICROWAVE							= (PARSE_APPEARANCE_VALUE( DOMESTIC_APPLIANCE, 			0x04 )),
	TOASTER								= (PARSE_APPEARANCE_VALUE( DOMESTIC_APPLIANCE, 			0x05 )),
	WASHING_MACHINE						= (PARSE_APPEARANCE_VALUE( DOMESTIC_APPLIANCE, 			0x06 )),
	DRYER								= (PARSE_APPEARANCE_VALUE( DOMESTIC_APPLIANCE, 			0x07 )),
	COFFEE_MAKER						= (PARSE_APPEARANCE_VALUE( DOMESTIC_APPLIANCE, 			0x08 )),
	CLOTHES_IRON						= (PARSE_APPEARANCE_VALUE( DOMESTIC_APPLIANCE, 			0x09 )),
	CURLING_IRON						= (PARSE_APPEARANCE_VALUE( DOMESTIC_APPLIANCE, 			0x0A )),
	HAIR_DRYER							= (PARSE_APPEARANCE_VALUE( DOMESTIC_APPLIANCE, 			0x0B )),
	VACUUM_CLEANER						= (PARSE_APPEARANCE_VALUE( DOMESTIC_APPLIANCE, 			0x0C )),
	ROBOTIC_VACUUM_CLEANER				= (PARSE_APPEARANCE_VALUE( DOMESTIC_APPLIANCE, 			0x0D )),
	RICE_COOKER							= (PARSE_APPEARANCE_VALUE( DOMESTIC_APPLIANCE, 			0x0E )),
	CLOTHES_STEAMER						= (PARSE_APPEARANCE_VALUE( DOMESTIC_APPLIANCE, 			0x0F )),

	GENERIC_WEARABLE_AUDIO_DEVICE		= (PARSE_APPEARANCE_VALUE( WEARABLE_AUDIO_DEVICE, 		0x00 )),
	EARBUD								= (PARSE_APPEARANCE_VALUE( WEARABLE_AUDIO_DEVICE, 		0x01 )),
	HEADSET								= (PARSE_APPEARANCE_VALUE( WEARABLE_AUDIO_DEVICE, 		0x02 )),
	HEADPHONES							= (PARSE_APPEARANCE_VALUE( WEARABLE_AUDIO_DEVICE, 		0x03 )),
	NECK_BAND							= (PARSE_APPEARANCE_VALUE( WEARABLE_AUDIO_DEVICE, 		0x04 )),

	GENERIC_AIRCRAFT					= (PARSE_APPEARANCE_VALUE( AIRCRAFT, 					0x00 )),
	LIGHT_AIRCRAFT						= (PARSE_APPEARANCE_VALUE( AIRCRAFT, 					0x01 )),
	MICROLIGHT							= (PARSE_APPEARANCE_VALUE( AIRCRAFT, 					0x02 )),
	PARAGLIDER							= (PARSE_APPEARANCE_VALUE( AIRCRAFT, 					0x03 )),
	LARGE_PASSENGER_AIRCRAFT			= (PARSE_APPEARANCE_VALUE( AIRCRAFT, 					0x04 )),

	GENERIC_AV_EQUIPMENT				= (PARSE_APPEARANCE_VALUE( AV_EQUIPMENT, 				0x00 )),
	AMPLIFIER							= (PARSE_APPEARANCE_VALUE( AV_EQUIPMENT, 				0x01 )),
	RECEIVER							= (PARSE_APPEARANCE_VALUE( AV_EQUIPMENT, 				0x02 )),
	RADIO								= (PARSE_APPEARANCE_VALUE( AV_EQUIPMENT, 				0x03 )),
	TUNER								= (PARSE_APPEARANCE_VALUE( AV_EQUIPMENT, 				0x04 )),
	TURNTABLE							= (PARSE_APPEARANCE_VALUE( AV_EQUIPMENT, 				0x05 )),
	CD_PLAYER							= (PARSE_APPEARANCE_VALUE( AV_EQUIPMENT, 				0x06 )),
	DVD_PLAYER							= (PARSE_APPEARANCE_VALUE( AV_EQUIPMENT, 				0x07 )),
	BLURAY_PLAYER						= (PARSE_APPEARANCE_VALUE( AV_EQUIPMENT, 				0x08 )),
	OPTICAL_DISC_PLAYER					= (PARSE_APPEARANCE_VALUE( AV_EQUIPMENT, 				0x09 )),
	SET_TOP_BOX							= (PARSE_APPEARANCE_VALUE( AV_EQUIPMENT, 				0x0A )),

	GENERIC_DISPLAY_EQUIPMENT			= (PARSE_APPEARANCE_VALUE( DISPLAY_EQUIPMENT, 			0x00 )),
	TELEVISION							= (PARSE_APPEARANCE_VALUE( DISPLAY_EQUIPMENT, 			0x01 )),
	MONITOR								= (PARSE_APPEARANCE_VALUE( DISPLAY_EQUIPMENT, 			0x02 )),
	PROJECTOR							= (PARSE_APPEARANCE_VALUE( DISPLAY_EQUIPMENT, 			0x03 )),

	GENERIC_HEARING_AID					= (PARSE_APPEARANCE_VALUE( HEARING_AID, 				0x00 )),
	IN_EAR_HEARING_AID					= (PARSE_APPEARANCE_VALUE( HEARING_AID, 				0x01 )),
	BEHIND_EAR_HEARING_AID				= (PARSE_APPEARANCE_VALUE( HEARING_AID, 				0x02 )),
	COCHLEAR_IMPLANT					= (PARSE_APPEARANCE_VALUE( HEARING_AID, 				0x03 )),

	GENERIC_GAMING						= (PARSE_APPEARANCE_VALUE( GAMING, 						0x00 )),
	HOME_VIDEO_GAME_CONSOLE				= (PARSE_APPEARANCE_VALUE( GAMING, 						0x01 )),
	PORTABLE_HANDHELD_CONSOLE			= (PARSE_APPEARANCE_VALUE( GAMING, 						0x02 )),

	GENERIC_SIGNAGE						= (PARSE_APPEARANCE_VALUE( SIGNAGE, 					0x00 )),
	DIGITAL_SIGNAGE						= (PARSE_APPEARANCE_VALUE( SIGNAGE, 					0x01 )),
	ELECTRONIC_LABEL					= (PARSE_APPEARANCE_VALUE( SIGNAGE, 					0x02 )),

	GENERIC_PULSE_OXIMETER				= (PARSE_APPEARANCE_VALUE( PULSE_OXIMETER, 				0x00 )),
	FINGERTIP_PULSE_OXIMETER			= (PARSE_APPEARANCE_VALUE( PULSE_OXIMETER, 				0x01 )),
	WRIST_WORN_PULSE_OXIMETER			= (PARSE_APPEARANCE_VALUE( PULSE_OXIMETER, 				0x02 )),

	GENERIC_WEIGHT_SCALE				= (PARSE_APPEARANCE_VALUE( WEIGHT_SCALE, 				0x00 )),

	GENERIC_PERSONAL_MOBILITY_DEVICE	= (PARSE_APPEARANCE_VALUE( PERSONAL_MOBILITY_DEVICE, 	0x00 )),
	POWERED_WHEELCHAIR					= (PARSE_APPEARANCE_VALUE( PERSONAL_MOBILITY_DEVICE, 	0x01 )),
	MOBILITY_SCOOTER					= (PARSE_APPEARANCE_VALUE( PERSONAL_MOBILITY_DEVICE, 	0x02 )),

	GENERIC_CONTINUOUS_GLUCOSE_MONITOR  = (PARSE_APPEARANCE_VALUE( CONTINUOUS_GLUCOSE_MONITOR, 	0x00 )),

	GENERIC_INSULIN_PUMP				= (PARSE_APPEARANCE_VALUE( INSULIN_PUMP, 				0x00 )),
	INSULIN_PUMP_DURABLE_PUMP			= (PARSE_APPEARANCE_VALUE( INSULIN_PUMP, 				0x01 )),
	INSULIN_PUMP_PATCH_PUMP				= (PARSE_APPEARANCE_VALUE( INSULIN_PUMP, 				0x04 )),
	INSULIN_PEN							= (PARSE_APPEARANCE_VALUE( INSULIN_PUMP, 				0x08 )),

	GENERIC_MEDICATION_DELIVERY			= (PARSE_APPEARANCE_VALUE( MEDICATION_DELIVERY, 		0x00 )),

	GENERIC_OUTDOOR_SPORTS_ACTIVITY		= (PARSE_APPEARANCE_VALUE( OUTDOOR_SPORTS_ACTIVITY, 	0x00 )),
	LOCATION_DISPLAY					= (PARSE_APPEARANCE_VALUE( OUTDOOR_SPORTS_ACTIVITY, 	0x01 )),
	LOCATION_AND_NAVIGATION_DISPLAY		= (PARSE_APPEARANCE_VALUE( OUTDOOR_SPORTS_ACTIVITY, 	0x02 )),
	LOCATION_POD						= (PARSE_APPEARANCE_VALUE( OUTDOOR_SPORTS_ACTIVITY, 	0x03 )),
	LOCATION_AND_NAVIGATION_POD			= (PARSE_APPEARANCE_VALUE( OUTDOOR_SPORTS_ACTIVITY, 	0x04 )),
}GAP_APPEARANCE;


/****************************************************************/
/* Type Defines 				               		            */
/****************************************************************/
typedef union
{
	struct Appearance_Value_Bits
	{
		uint16_t Sub_category :6;
		uint16_t Category	  :10;
	}__attribute__((packed)) Bits;
	uint16_t Value;
}__attribute__((packed)) Appearance_Value;


/****************************************************************/
/* External functions declaration (Interface functions)         */
/****************************************************************/


/****************************************************************/
/* External variables declaration                               */
/****************************************************************/


#endif /* APPEARANCE_VALUES_H_ */


/****************************************************************/
/* End of file	                                                */
/****************************************************************/
