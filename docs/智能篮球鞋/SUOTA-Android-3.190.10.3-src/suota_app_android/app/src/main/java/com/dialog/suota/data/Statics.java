package com.dialog.suota.data;

import android.content.Context;
import android.util.SparseArray;

import com.dialog.suota.R;

import java.util.UUID;

public class Statics {
	public static final String BLUETOOTH_GATT_UPDATE = "BluetoothGattUpdate";
	public static final String PROGRESS_UPDATE = "ProgressUpdate";
	public static final String CONNECTION_STATE_UPDATE = "ConnectionState";
    public static final int fileChunkSize = 20;

	public static final int MEMORY_TYPE_SYSTEM_RAM = 1;
	public static final int MEMORY_TYPE_RETENTION_RAM = 2;
	public static final int MEMORY_TYPE_SPI = 3;
	public static final int MEMORY_TYPE_I2C = 4;

	public static final UUID SPOTA_SERVICE_UUID = UUID.fromString("0000fef5-0000-1000-8000-00805f9b34fb");
	public static final UUID SPOTA_MEM_DEV_UUID = UUID.fromString("8082caa8-41a6-4021-91c6-56f9b954cc34");
	public static final UUID SPOTA_GPIO_MAP_UUID = UUID.fromString("724249f0-5eC3-4b5f-8804-42345af08651");
	public static final UUID SPOTA_MEM_INFO_UUID = UUID.fromString("6c53db25-47a1-45fe-a022-7c92fb334fd4");
	public static final UUID SPOTA_PATCH_LEN_UUID = UUID.fromString("9d84b9a3-000c-49d8-9183-855b673fda31");
	public static final UUID SPOTA_PATCH_DATA_UUID = UUID.fromString("457871e8-d516-4ca1-9116-57d0b17b9cb2");
	public static final UUID SPOTA_SERV_STATUS_UUID = UUID.fromString("5f78df94-798c-46f5-990a-b3eb6a065c88");

    public static final UUID SPOTA_DESCRIPTOR_UUID = UUID.fromString("00002902-0000-1000-8000-00805f9b34fb");

    public static final UUID ORG_BLUETOOTH_SERVICE_DEVICE_INFORMATION              = UUID.fromString("0000180a-0000-1000-8000-00805f9b34fb");
    public static final UUID ORG_BLUETOOTH_CHARACTERISTIC_MANUFACTURER_NAME_STRING = UUID.fromString("00002A29-0000-1000-8000-00805f9b34fb");
    public static final UUID ORG_BLUETOOTH_CHARACTERISTIC_MODEL_NUMBER_STRING      = UUID.fromString("00002A24-0000-1000-8000-00805f9b34fb");
    public static final UUID ORG_BLUETOOTH_CHARACTERISTIC_SERIAL_NUMBER_STRING     = UUID.fromString("00002A25-0000-1000-8000-00805f9b34fb");
    public static final UUID ORG_BLUETOOTH_CHARACTERISTIC_HARDWARE_REVISION_STRING = UUID.fromString("00002A27-0000-1000-8000-00805f9b34fb");
    public static final UUID ORG_BLUETOOTH_CHARACTERISTIC_FIRMWARE_REVISION_STRING = UUID.fromString("00002A26-0000-1000-8000-00805f9b34fb");
    public static final UUID ORG_BLUETOOTH_CHARACTERISTIC_SOFTWARE_REVISION_STRING = UUID.fromString("00002A28-0000-1000-8000-00805f9b34fb");
    public static final UUID ORG_BLUETOOTH_CHARACTERISTIC_SYSTEM_ID                = UUID.fromString("00002A23-0000-1000-8000-00805f9b34fb");
    public static final UUID ORG_BLUETOOTH_CHARACTERISTIC_IEEE_11073               = UUID.fromString("00002A2A-0000-1000-8000-00805f9b34fb");
    public static final UUID ORG_BLUETOOTH_CHARACTERISTIC_PNP_ID                   = UUID.fromString("00002A50-0000-1000-8000-00805f9b34fb");

    // Default memory type
    public static final int DEFAULT_MEMORY_TYPE = MEMORY_TYPE_SPI;

    // Default SPI memory settings
	public static final int DEFAULT_MISO_VALUE = 5;
	public static final int DEFAULT_MOSI_VALUE = 6;
	public static final int DEFAULT_CS_VALUE = 3;
	public static final int DEFAULT_SCK_VALUE = 0;
	public static final String DEFAULT_BLOCK_SIZE_VALUE = "240";

    // Default I2C memory settings
	public static final int DEFAULT_MEMORY_BANK = 0;
    public static final String DEFAULT_I2C_DEVICE_ADDRESS = "0x50";
    public static final int DEFAULT_SCL_GPIO_VALUE = 2;
    public static final int DEFAULT_SDA_GPIO_VALUE = 3;

    public static final int MEMORY_TYPE_SUOTA_INDEX = 100;
    public static final int MEMORY_TYPE_SPOTA_INDEX = 101;

    // Application error codes (must be greater than 255 in order not to conflict with SUOTA error codes)
    public static final int ERROR_COMMUNICATION = 0xffff; // ble communication error
    public static final int ERROR_SUOTA_NOT_FOUND = 0xfffe; // suota service was not found

//    public static final int ERROR_SPOTAR_SRV_EXIT = 3;
//    public static final int ERROR_SPOTAR_CRC_ERR = 4;
//    public static final int ERROR_SPOTAR_PATCH_LEN_ERR = 5;
//    public static final int ERROR_SPOTAR_EXT_MEM_ERR = 6;
//    public static final int ERROR_SPOTAR_INT_MEM_ERR = 7;
//    public static final int ERROR_INVALID_MEM_TYPE = 8;
//    public static final int ERROR_SPOTAR_APP_ERROR = 9;
//    public static final int ERROR_SPOTAR_IMG_STARTED = 10;
//    public static final int ERROR_SPOTAR_INVAL_IMG_BANKE = 11;
//    public static final int ERROR_SPOTAR_INVAL_IMG_HDR = 12;

    private static SparseArray<String> prevInputPrefKeyMap = new SparseArray<String>() {{
        put(R.id.misoGpioSpinner, "misoGpioSpinner");
        put(R.id.mosiGpioSpinner, "mosiGpioSpinner");
        put(R.id.csGpioSpinner, "csGpioSpinner");
        put(R.id.sckGpioSpinner, "sckGpioSpinner");
        put(R.id.sclGpioSpinner, "sclGpioSpinner");
        put(R.id.sdaGpioSpinner, "sdaGpioSpinner");
        put(R.id.imageBank, "imageBank");
        put(R.id.blockSize, "blockSize");
        put(R.id.patchBaseAddress, "patchBaseAddress");
        put(R.id.I2CDeviceAddress, "I2CDeviceAddress");
        put(MEMORY_TYPE_SUOTA_INDEX, "MEMORY_TYPE_SUOTA_INDEX");
        put(MEMORY_TYPE_SPOTA_INDEX, "MEMORY_TYPE_SPOTA_INDEX");
        put(R.id.show_bonded_devices, "show_bonded_devices");
    }};

	public static int gpioStringToInt(String spioValue) {
		String strippedValue = "0x" + spioValue.replace("P", "").replace("_", "");
		return Integer.decode(strippedValue);
	}

	public static String getPreviousInput(Context c, int resourceID) {
        String key = prevInputPrefKeyMap.get(resourceID);
		return c.getSharedPreferences("settings", c.MODE_PRIVATE).getString(key, null);
	}

	public static void setPreviousInput(Context c, int resourceID, String value) {
        String key = prevInputPrefKeyMap.get(resourceID);
		c.getSharedPreferences("settings", c.MODE_PRIVATE).edit().putString(key, value.trim()).apply();
	}

	public static boolean fileDirectoriesCreated(Context c) {
		return c.getSharedPreferences("settings", c.MODE_PRIVATE).getBoolean("fileDirectoriesCreated", false);
	}

	public static void setFileDirectoriesCreated(Context c) {
		c.getSharedPreferences("settings", c.MODE_PRIVATE).edit().putBoolean("fileDirectoriesCreated", true).apply();
	}
}