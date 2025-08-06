// main.cpp
/*
	IMU_ESP32 MINI
*/
#include "main.h"
#include "system_config.h"
#include "cli/CommandProcessor.h"
#include "device/ads/MPLEX.h"
#include "device/IMU/MPU6000/MPU6000_instance.h"
#include "api/MPLEXApi.h"
#include "system/SystemMonitor.h"
#include "Arduino.h"
#include <U8g2lib.h>

using namespace overseer;
using namespace overseer::device::ads;
using namespace overseer::device::environment;
using namespace overseer::client::core::string_table;

//using overseer::client::core::string_table::message_boot__system_config_load;

system_utils::SystemMonitor *sysMon = nullptr;
RUNNING_CONFIG running_config;
config::ConfigManager configManager(SPIFFS);
WiFiManager *wifi = nullptr;
WebServerManager *web = nullptr;
String inputBuffer;
CommandProcessor commandProcessor;
MPLEX mplex; //
DHTFAMILY dht1;
// U8X8_SSD1306_128X64_NONAME_HW_I2C u8x8(/* reset=*/U8X8_PIN_NONE); // Nope
// U8G2_SSD1306_128X64_NONAME_1_SW_I2C u8x8(U8G2_R0, /* SCL=*/ OVERSEER_SCL_PIN, /* SDA=*/ OVERSEER_SDA_PIN); //No Love
// U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2( U8G2_R0, /* reset=*/ U8X8_PIN_NONE, /* clock=*/ 9, /* data=*/ 8);
//U8G2_SSD1306_128X64_NONAME_F_HW_I2C* u8g2 = nullptr;
//std::unique_ptr<U8G2_SSD1306_128X64_NONAME_F_HW_I2C> u8g2;
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2 ( U8G2_R0, U8X8_PIN_NONE, 9, 8);
		
		
void led_blink(bool mode)
{
	digitalWrite(STATUS_LED_RED, mode); // LED ON
	delay(50);
	digitalWrite(STATUS_LED_YELLOW, mode); // LED ON
	delay(25);
	digitalWrite(STATUS_LED_GREEN, mode); // LED Off    
}

void set_system_failed(String message)
{    
	Log.fatalln ("Wire.begin FAILED");
	//Serial.flush();
	while (true)
	{
		digitalWrite(STATUS_LED_RED, HIGH); // LED ON
		delay(100);
		digitalWrite(STATUS_LED_RED, LOW); // LED ON
		delay(250);
		digitalWrite(STATUS_LED_RED, HIGH); // LED ON
		delay(100);
		digitalWrite(STATUS_LED_RED, LOW); // LED ON
		delay(100);        
		yield();
		delay(500);
	}
}

void blink_blue_led()
{
	digitalWrite(STATUS_LED_INDICATOR, HIGH); // Blue LED ON
	delay(250);
	digitalWrite(STATUS_LED_INDICATOR, LOW); // Blue LED off
	delay(500);
	digitalWrite(STATUS_LED_INDICATOR, HIGH); // Blue LED ON
	delay(250);
	digitalWrite(STATUS_LED_INDICATOR, LOW); // Blue LED off
}

void blink_green_led()
{
	digitalWrite(STATUS_LED_GREEN, HIGH); // Blue LED ON
	delay(250);
	digitalWrite(STATUS_LED_GREEN, LOW); // Blue LED off
	delay(500);
	digitalWrite(STATUS_LED_GREEN, HIGH); // Blue LED ON
	delay(250);
	digitalWrite(STATUS_LED_GREEN, LOW); // Blue LED off
}
void overseer_led_indicator_configure()
{
	pinMode(STATUS_LED_GREEN, OUTPUT);
	pinMode(STATUS_LED_YELLOW, OUTPUT);
	pinMode(STATUS_LED_RED, OUTPUT);
	pinMode(STATUS_LED_INDICATOR, OUTPUT);

	digitalWrite(STATUS_LED_GREEN, LOW);      // LED OFF
	digitalWrite(STATUS_LED_YELLOW, LOW);     // LED OFF
	digitalWrite(STATUS_LED_RED, LOW);        // LED OFF
	digitalWrite(STATUS_LED_INDICATOR, HIGH); // Blue LED ON
}
void overseer_led_start_sequence()
{
	blink_blue_led();
	digitalWrite(STATUS_LED_GREEN, HIGH);
}
std::vector<int> get_i2c_device_list()
{
	std::vector<int> i2c_discovery;
	for (byte address = 1; address < 127; address++)
	{
		Wire.beginTransmission(address);
		if (Wire.endTransmission() == 0)
		{
			Log.verbose("Found device at 0x");
			// Log.verboseln(address);
			i2c_discovery.push_back(address);
		}
		delay(10);
	}
	return i2c_discovery;
}
bool oc_configure_i2c_hardware()
{
	bool ret = true;
	Log.infoln("___ oc_configure_i2c_hardware (i2c) Starting ___");
	Log.info("PINS SDA:: %d, \t", OVERSEER_SDA_PIN);
	Log.infoln("PINS SCL:: %d", OVERSEER_SCL_PIN);
	Log.infoln("CheckPoint 1a");
	
	Log.verboseln("Wire.Begin Return:: %b", ret);
	Log.infoln("CheckPoint 1b");
	if (ret == false)
	{
		Log.fatalln ("Wire.begin FAILED");
		return false;
	}

	Log.infoln("\n\nInitializing System Display.");    


	//u8g2 = std::make_unique<U8G2_SSD1306_128X64_NONAME_F_HW_I2C>(
	//  U8G2_R0, U8X8_PIN_NONE, 9, 8, &I2CBus);
	/**
	 * @brief Initialize the Screen
	 * 
	 * */ 
	u8g2.begin();
	u8g2.clearBuffer();	
	//u8g2.setFontMode(0);				// write solid glyphs
	//u8g2.setFont(u8g2_font_ncenB08_tr);
	u8g2.setFont(u8g2_font_6x10_tf);
	u8g2.drawStr(0, 24, "Meow Started");
	u8g2.sendBuffer();
	//u8g2.setFontMode(1);
	
	Log.infoln("Initializing System Display Finished.");

	/*
	Log.verboseln ("MPLEX(ADS1115)::Starting...");
	if (mplex.begin() == false)
	{
		Log.warningln("MPLEX::Begin returned False");
		return false;
	}
	else
	{
		Log.verboseln(F("MPLEX (ADS1115) Started"));
	}

	Log.infoln("Starting Calibrate DC Current Sensor");
	mplex.calibrateAllChannels();
	Log.infoln("MPLEX::All channels calibrated");    
	*/
	Log.infoln(" ___ oc_configure_i2c_hardware (i2c) Exit ___");
	return true;
}

bool start_overseer_webserver()
{
	Log.infoln("Web Server:: Starting");
	web = new WebServerManager(configManager);
	auto *imuApi = new overseer::device::api::IMUApi(web->getServer(), *running_config.hardware_config.imu.mpu, configManager);
	auto *mplexApi = new overseer::device::api::MPLEXApi(web->getServer(), mplex, configManager);
	web->setSystemMonitor(sysMon);
	web->registerDeviceApi(imuApi);
	Log.infoln("Web Server:: Registering MultiPlexer API");
	web->registerDeviceApi(mplexApi);

	web->begin();
	return true;
}
void print_wcs_current_data(ChannelData &channelData)
{
    Serial.print("ADS:0 Raw Reading:: ");
    Serial.println(channelData.raw_value);
    Log.verboseln("Print mplex/ADS Data...");
    Serial.print("\tAnalog voltage 0: ");
    Serial.print(channelData.voltage);
    Serial.print('\n');
    Serial.print("\tScaled value 0: ");
    Serial.print(channelData.scaled_value);
    Serial.print('\n');
    Serial.print("\tChannel valid: ");
    Serial.print(channelData.valid ? "true" : "false");
    Serial.print('\n');
}

//void update_simple_screen () {
	//float max_gforce = std::max(data.gx, std::max(data.gy, data.gz));
	//String msg_max_g = "MaxG: " + String(max_gforce, 3);
	//u8g2.setFont(u8x8_font_chroma48medium8_r);
	//u8g2.drawStr(0,1, msg_max_g.c_str());
	//u8g2.setInverseFont(1);
	////u8g2.drawString(0,0,"012345678901234567890123456789");
	////u8g2.setInverseFont(0);
	//u8x8.drawString(0,8,"Line 8");
	//u8x8.drawString(0,9,"Line 9");
	//u8g2.refreshDisplay();
//}
void setup()
{
	Serial.begin(115200);
	delay(200);
	overseer_led_start_sequence();

	// while (!Serial && !Serial.available()) {}
	// while (!Serial) {}    

	Log.begin(LOG_LEVEL_VERBOSE, &Serial);
	Log.notice(F(CR "******************************************" CR)); // Info string with Newline
	//Log.notice();  // Info string in flash memory
	Log.notice(F(CR "******************************************" CR));
	Serial.flush();

	Log.verbose(F(CR "Loading SPIFFS File System ..."));
	if (!SPIFFS.begin(FORMAT_SPIFFS_IF_FAILED))
	{
		Serial.println("SPIFFS Mount Failed");        
		set_system_failed("SPIFFS Mount Failed");
		return;
	}
	blink_green_led();
	//Log.verbose(message_boot__system_config_load);
	Serial.flush();
	if (!configManager.begin())
	{
		set_system_failed(F("PANIC!! Config load failed"));
	}

	// Set the Logging Level
	//Log.infoln(CR"Setting Log Level: " + String(configManager.getLogLevel()));
	//Log.setLevel(configManager.getLogLevel());
	Log.notice(F(CR "******************************************\n" CR));
	Log.infoln(CR"Starting System Monitor");
	sysMon = system_utils::createSystemMonitor(configManager);
	sysMon->begin();

	//Log.infoln(message_boot__system_wifi_init);
	wifi = new WiFiManager(configManager);
	wifi->begin();

	if (!wifi->isConnected()) {
		set_system_failed(F("WiFi connection failed. Halting."));
	}

	Log.notice(F(CR "******************************************\n" CR));
	Log.infoln("\tConfigure LED Hardware");
	overseer_led_indicator_configure();
	Log.infoln("CheckPoint 1");

	Log.infoln("Configure and Start I2C.");
	if (oc_configure_i2c_hardware() == false)
	{
		Log.errorln("ERROR: I2C startup failed!!");
		std::vector<int> data = get_i2c_device_list(); // <-- FIXME: Changge to get address
		set_system_failed(F("I2C Device Initialization Failed!"));
		// for (const auto& address : data) {
		//     Serial.print(F("Found device at 0x"));
		//     Serial.println(address, HEX);  // Proper hex print
		// }
	}
	Log.infoln("CheckPoint 2");
	/**
	 * Init Display
	 */
	
	//u8x8.begin();
	//u8x8.clearDisplay();
	//u8x8.clearBuffer();					// clear the internal memory
	//u8x8.setFont(u8g2_font_ncenB08_tr);	// choose a suitable font    
	//u8x8.draw1x2String(0,10, "Me So Horny!"); // Let'm know!
	
	//u8x8.setCursor(0,1);
	//u8x8.drawStr(0,1, "Hello");
	/*
	u8g2.begin();
	u8g2.clearBuffer();
	u8g2.setFont(u8g2_font_ncenB08_tr);
	u8g2.drawStr(0, 24, "Me So Horny!");
	u8g2.sendBuffer();
	*/
	delay(1000);  
	/*
	if (running_config.debug_enable == true)
	{
		Log.infoln("i2c: I2C Info");
		std::vector<int> data = get_i2c_device_list();
		Log.verboseln("I2C : %d active devices", data.size());
		for (const auto &address : data)
		{
			Log.verboseln("I2C Address: %F", address);
			// Serial.print(F("Found device:: 0x"));
			// Serial.println(address, HEX);  // Proper hex print
		}
	}
	*/

	led_blink(HIGH);
	delay(500);
	led_blink(LOW);

	Log.infoln("\t\t IMU Device");
	Log.infoln("enable_imu_print::%s", running_config.debug_options.enable_imu_print ? "true" : "false");
	// Log.infoln("enable_imu_print: %s", RUNNING_CONFIG::DEBUG_OPTIONS::enable_imu_print ? "true" : "false");
	// Log.infoln("enable_imu_print: %s", RUNNING_CONFIG::DEBUG_OPTIONS::enable_imu_print ? "true" : "false");

	Log.infoln("imu_log_message_interval::%u", running_config.debug_options.imu_log_message_interval);
	Log.infoln("smoothing_alpha:: %F", running_config.hardware_config.imu.filter_config.smoothing_alpha);
	Log.infoln("spike_threshold:: %F", running_config.hardware_config.imu.filter_config.spike_threshold);
	Log.infoln("window_smoothing_alpha:: %F", running_config.hardware_config.imu.filter_config.window_smoothing_alpha);    
	/*
	  TODO: Check that we have the right i2c devices...
	*/
	Log.infoln("Initializing IMU Device");
	running_config.hardware_config.imu.mpu = &overseer::device::imu::getInstance();
	Log.infoln("Starting IMU");
	if (running_config.hardware_config.imu.mpu)
	{
		running_config.hardware_config.imu.mpu->begin();
	}
	
	
	// Start App (web) Server
	if (start_overseer_webserver() == false)
	{
		Log.errorln(F("Webserver Failed to Start"));
		set_system_failed(F("Webserver Failed to Start"));
	}
	else
	{
		Log.infoln("Web Started");
	}

	Serial.println("READY");
}



void loop()
{
    unsigned long currentMillis = millis();
    
    // Handle serial input
    while (Serial.available())
    {
        char c = Serial.read();
        if (c == '\n' || c == '\r')
        {
            if (inputBuffer.length() > 0)
            {
                commandProcessor.processLine(inputBuffer);
                inputBuffer = "";
            }
        }
        else
        {
            inputBuffer += c;
        }
    }
    
    // System monitor update
    if (currentMillis - running_config.debug_options.log_last_print_time >= running_config.debug_options.log_message_interval)
    {
        if (sysMon) sysMon->update();
        running_config.debug_options.log_last_print_time = currentMillis;
    }
    
    // Read sensor data
    overseer::device::ads::ChannelData channelData = mplex.getChannelData(WCS_SENSOR_1_ADS_CHANNEL);
    ChannelData channelData_2 = mplex.getChannelData(A1);
    
    running_config.hardware_config.imu.mpu->update();
    auto sensor_data = running_config.hardware_config.imu.mpu->getData();
    running_config.hardware_config.imu.mpu->smoothAndFilterMPUData(sensor_data);
    
    // Display update every 5 seconds (separate from debug logging)
    static unsigned long last_display_update = 0;
    const unsigned long DISPLAY_UPDATE_INTERVAL = 5000; // 5 seconds
    
    if (currentMillis - last_display_update >= DISPLAY_UPDATE_INTERVAL)
    {
        last_display_update = currentMillis;
        
        // Clear and update display
        u8g2.clearBuffer();
        
        float max_gforce = std::max(sensor_data.gx, std::max(sensor_data.gy, sensor_data.gz));
        String msg_max_g = "MaxG: " + String(max_gforce, 3);
        
        // Display on OLED
        u8g2.drawStr(0, 12, msg_max_g.c_str());
        
        u8g2.setCursor(0, 26);
        u8g2.print("Max V: ");
        u8g2.print(channelData.raw_value, 1);
        u8g2.print("v");
        
        u8g2.setCursor(0, 40);
        u8g2.print("A1: ");
        u8g2.print(channelData_2.raw_value, 1);
        
        u8g2.setCursor(0, 54);
        u8g2.print("Status: normal");
        
        u8g2.sendBuffer();
    }
    
    // Debug logging (separate timing from display)
    if (running_config.debug_enable == true && running_config.debug_options.enable_imu_print == true)
    {
        if (currentMillis - running_config.debug_options.imu_log_last_print_time >= running_config.debug_options.imu_log_message_interval)
        {
            running_config.debug_options.imu_log_last_print_time = currentMillis;
            running_config.hardware_config.imu.mpu->printMPUData(sensor_data);
            Log.verboseln("Done with IMU");
            print_wcs_current_data(channelData);
            
			String raw = String(channelData_2.raw_value, 1);
            String msg_sensor_a1 = "A1 Reading: " + raw;
            Serial.println (msg_sensor_a1);
        }
    }
    
    yield();
}











