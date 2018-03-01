package com.dialog.suota;

import android.app.Activity;
import android.app.AlertDialog;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.content.Intent;
import android.content.pm.PackageInfo;
import android.content.pm.PackageManager;
import android.os.Bundle;
import android.os.Handler;
import android.util.Log;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.view.Window;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemClickListener;
import android.widget.ArrayAdapter;
import android.widget.ListView;
import android.widget.TextView;
import android.widget.Toast;

import com.crashlytics.android.Crashlytics;
import com.dialog.suota.data.File;
import com.dialog.suota.data.Statics;
import com.dialog.suota.data.Uuid;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.UUID;

import io.fabric.sdk.android.Fabric;

public class ScanActivity extends SuotaActivity implements OnItemClickListener {
    private final static String TAG = "ScanActivity";
    private final static int REQUEST_ENABLE_BT = 1;

    private boolean isScanning = false;
    private boolean showBondedDevices;

    private BluetoothAdapter mBluetoothAdapter;
    private HashMap<String, BluetoothDevice> scannedDevices;

    private ArrayList<BluetoothDevice> bluetoothDeviceList;
    private ArrayAdapter<String> mArrayAdapter;

    private ListView deviceListView;
    private MenuItem menuItemRefresh;

    private Handler handler;

    private BluetoothAdapter.LeScanCallback mLeScanCallback = new BluetoothAdapter.LeScanCallback() {
        @Override
        public void onLeScan(final BluetoothDevice device, int rssi,
                             final byte[] scanRecord) {

            runOnUiThread(new Runnable() {
                @Override
                public void run() {
                    List<UUID> uuids = Uuid.parseFromAdvertisementData(scanRecord);
                    for (UUID uuid : uuids) {
                        if (uuid.equals(Statics.SPOTA_SERVICE_UUID) && !scannedDevices.containsKey(device.getAddress())) {
                            scannedDevices.put(device.getAddress(), device);
                            if (!bluetoothDeviceList.contains(device)) {
                                bluetoothDeviceList.add(device);
                                mArrayAdapter.add(device.getName() + "\n"
                                        + device.getAddress());
                            }
                        }
                    }
                }
            });
        }
    };

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        Fabric.with(this, new Crashlytics());
        requestWindowFeature(Window.FEATURE_INDETERMINATE_PROGRESS);
        setContentView(R.layout.activity_scan);
        String prevShowBondedDevices = Statics.getPreviousInput(this, R.id.show_bonded_devices);
        showBondedDevices = prevShowBondedDevices != null && Boolean.parseBoolean(prevShowBondedDevices);
        this.initialize();
		if(!Statics.fileDirectoriesCreated(this) || true) {
			File.createFileDirectories(this);
			Statics.setFileDirectoriesCreated(this);
		}
    }

	@Override
	protected void onActivityResult(int requestCode, int resultCode, Intent data) {
		// Check which request we're responding to
		if (requestCode == REQUEST_ENABLE_BT) {
			// Make sure the request was successful
			if (resultCode == Activity.RESULT_OK) {
				this.startDeviceScan();
			}
		}
	}

    @Override
    protected void onDestroy() {
        stopDeviceScan();
        super.onDestroy();
    }

    private void initialize() {
        // Initialize layout variables
        setTitle(getResources().getString(R.string.app_devices_title));
        deviceListView = (ListView) findViewById(R.id.device_list);
        scannedDevices = new HashMap<String, BluetoothDevice>();
        bluetoothDeviceList = new ArrayList<BluetoothDevice>();
        mArrayAdapter = new ArrayAdapter<String>(this,
                android.R.layout.simple_list_item_1);

        // Initialize Bluetooth adapter
        mBluetoothAdapter = BluetoothAdapter.getDefaultAdapter();
        if (mBluetoothAdapter == null || !getPackageManager().hasSystemFeature(PackageManager.FEATURE_BLUETOOTH_LE)) {
            // Device does not support Bluetooth Low Energy
            Log.e(TAG, "Bluetooth Low Energy not supported.");
            Toast.makeText(getApplicationContext(), "Bluetooth Low Energy is not supported on this device", Toast.LENGTH_LONG).show();
            finish();
        }

        handler = new Handler();

        // If the bluetooth adapter is not enabled, request to enable it
        if (!mBluetoothAdapter.isEnabled()) {
            Intent enableBtIntent = new Intent(
                    BluetoothAdapter.ACTION_REQUEST_ENABLE);
            startActivityForResult(enableBtIntent, REQUEST_ENABLE_BT);
        }
        // Otherwise, start scanning right away
        else {
            this.startDeviceScan();
        }

        deviceListView.setAdapter(mArrayAdapter);
        deviceListView.setOnItemClickListener(this);

    }

    private void startDeviceScan() {
        isScanning = true;
        mArrayAdapter.clear();
        bluetoothDeviceList.clear();
        scannedDevices.clear();
        if(menuItemRefresh != null) {
            menuItemRefresh.setVisible(false);
        }
        Log.d(TAG, "Start scanning");
        setProgressBarIndeterminateVisibility(true);
        updateBondedDevices();

        mBluetoothAdapter.startLeScan(mLeScanCallback);
        handler.postDelayed(new Runnable() {
            @Override
            public void run() {
                stopDeviceScan();
            }
        }, 7000);
    }

    private void stopDeviceScan() {
        if(isScanning) {
            isScanning = false;
            Log.d(TAG, "Stop scanning");
            setProgressBarIndeterminateVisibility(false);
            mBluetoothAdapter.stopLeScan(mLeScanCallback);
            if (menuItemRefresh != null) {
                menuItemRefresh.setVisible(true);
            }
        }
    }

    private void updateBondedDevices() {
        if (showBondedDevices) {
            for (BluetoothDevice device : mBluetoothAdapter.getBondedDevices()) {
                if (!bluetoothDeviceList.contains(device)) {
                    bluetoothDeviceList.add(device);
                    mArrayAdapter.add(device.getName() + "\n"
                            + device.getAddress() + " (paired)");
                }
            }
        }
        else {
            // Remove bonded devices that were not found during scan
            for (BluetoothDevice device : mBluetoothAdapter.getBondedDevices()) {
                if (!scannedDevices.containsKey(device.getAddress())) {
                    bluetoothDeviceList.remove(device);
                }
            }
            mArrayAdapter.clear();
            for (BluetoothDevice device : bluetoothDeviceList) {
                mArrayAdapter.add(device.getName() + "\n"
                        + device.getAddress());
            }
        }
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        boolean status = super.onCreateOptionsMenu(menu);
        menuItemRefresh = menu.findItem(R.id.restart_scan);
        if(isScanning) {
            menuItemRefresh.setVisible(false);
        }
        MenuItem menuItemShowBonded = menu.findItem(R.id.show_bonded_devices);
        menuItemShowBonded.setVisible(true);
        menuItemShowBonded.setChecked(showBondedDevices);
        MenuItem menuItemAbout = menu.findItem(R.id.about);
        menuItemAbout.setVisible(true);
        return status;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        int id = item.getItemId();
        if (id == R.id.restart_scan) {
            this.startDeviceScan();
            return true;
        }
        if (id == R.id.show_bonded_devices) {
            showBondedDevices = !showBondedDevices;
            Statics.setPreviousInput(this, R.id.show_bonded_devices, String.valueOf(showBondedDevices));
            item.setChecked(showBondedDevices);
            updateBondedDevices();
            return true;
        }
        if (id == R.id.about) {
            PackageInfo info;
            try {
                info = getPackageManager().getPackageInfo(getPackageName(), 0);
            } catch (PackageManager.NameNotFoundException e) {
                return true;
            }
            View view = getLayoutInflater().inflate(R.layout.dialog_about, null);
            final AlertDialog dialog = new AlertDialog.Builder(this).setView(view).create();
            TextView version = (TextView) view.findViewById(R.id.version_text);
            version.setText(String.format(getString(R.string.about_version), info.versionName, info.versionCode));
            view.setOnClickListener(new View.OnClickListener() {
                @Override
                public void onClick(View v) {
                    dialog.dismiss();
                }
            });
            dialog.show();
            return true;
        }
        return super.onOptionsItemSelected(item);
    }

    /**
     * On click listener for scanned devices
     *
     * @param parent
     * @param view
     * @param position
     * @param id
     */
    @Override
    public void onItemClick(AdapterView<?> parent, View view, int position,
                            long id) {
        stopDeviceScan();
        BluetoothDevice device = bluetoothDeviceList.get(position);
        Intent i = new Intent(ScanActivity.this, DeviceActivity.class);
        i.putExtra("device", device);
        startActivity(i);
    }
}
