package com.dialog.suota.async;

import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothGatt;
import android.content.Context;
import android.os.AsyncTask;
import android.util.Log;

import com.dialog.suota.bluetooth.BluetoothGattSingleton;
import com.dialog.suota.bluetooth.Callback;

import java.lang.reflect.Method;

public class DeviceConnectTask extends AsyncTask<Void, BluetoothGatt, Boolean > {
	public static final String TAG = "DeviceGattTask";
	public Context context;
	private final BluetoothDevice mmDevice;
	private final Callback callback = new Callback(this);
    private boolean refreshResult;

	public DeviceConnectTask(Context context, BluetoothDevice device) {
		Log.d(TAG, "init");
		this.context = context;
		mmDevice = device;
	}

	/** Will cancel an in-progress connection, and close the socket */
	public void cancel() {
	}

	private boolean refreshDeviceCache(BluetoothGatt gatt){
		try {
            refreshResult = false;
			Method localMethod = gatt.getClass().getMethod("refresh", (Class[]) null);
			if (localMethod != null) {
                int attempt = 0;
                while (!refreshResult && ++attempt <= 200) {
                    Log.d(TAG, "refresh attempt " + attempt);
                    refreshResult = (Boolean) localMethod.invoke(gatt, (Object[]) null);
                    if (!refreshResult) {
                        try {
                            Thread.sleep(10);
                        } catch (InterruptedException exp) {}
                    }
                }
				return refreshResult;
			}
		}
		catch (Exception localException) {
			Log.e(TAG, "An exception occurred while refreshing device");
		}
		return false;
	}

    public boolean refreshSucceeded() {
        return refreshResult;
    }

	@Override
	protected Boolean doInBackground(Void... params) {
		BluetoothGatt gatt = mmDevice.connectGatt(context, false, callback);
        BluetoothGattSingleton.setGatt(gatt);
		//refreshDeviceCache(gatt);
		return true;
	}

	@Override
	protected void onPreExecute() {
		super.onPreExecute();
	}

	@Override
	protected void onPostExecute(Boolean aBoolean) {
		super.onPostExecute(aBoolean);
	}

	@Override
	protected void onProgressUpdate(BluetoothGatt... values) {
		super.onProgressUpdate(values);
	}

	@Override
	protected void onCancelled(Boolean aBoolean) {
		super.onCancelled(aBoolean);
		cancel();
	}

	@Override
	protected void onCancelled() {
		super.onCancelled();
		cancel();
	}
}