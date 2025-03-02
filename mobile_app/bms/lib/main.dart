import 'package:flutter/material.dart';
import 'package:bluetooth_classic/bluetooth_classic.dart';
import 'package:bluetooth_classic/models/device.dart';
import 'package:fl_chart/fl_chart.dart';
import 'dart:math';
import 'dart:async';

void main() {
  runApp(const MyApp());
}

class MyApp extends StatelessWidget {
  const MyApp({Key? key}) : super(key: key);

  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      title: 'Battery Monitor',
      theme: ThemeData(
        primarySwatch: Colors.blue,
      ),
      home: const BatteryMonitorPage(),
    );
  }
}

class BatteryMonitorPage extends StatefulWidget {
  const BatteryMonitorPage({Key? key}) : super(key: key);

  @override
  _BatteryMonitorPageState createState() => _BatteryMonitorPageState();
}

class _BatteryMonitorPageState extends State<BatteryMonitorPage> {
  final BluetoothClassic _bluetoothClassicPlugin = BluetoothClassic();
  List<Device> _pairedDevices = [];
  Device? _selectedDevice;
  bool _isConnected = false;
  String _buffer = '';
  final String _delimiter = '\n';
  StreamSubscription? _dataSubscription;

  List<FlSpot> currentData = [];
  List<FlSpot> voltageData = [];
  List<FlSpot> temperatureData = [];
  double batteryHealth = 0;
  double batteryPercentage = 0;

  @override
  void initState() {
    super.initState();
    _getPairedDevices();
  }

  Future<void> _getPairedDevices() async {
    var devices = await _bluetoothClassicPlugin.getPairedDevices();
    setState(() {
      _pairedDevices = devices;
    });
  }

  void _connectToDevice(Device device) async {
    await _bluetoothClassicPlugin.connect(device.address, "00001101-0000-1000-8000-00805f9b34fb");
    setState(() {
      _isConnected = true;
    });
    _startListeningForData();
  }

  void _startListeningForData() {
    _dataSubscription = _bluetoothClassicPlugin.onDeviceDataReceived().listen((data) {
      String receivedString = String.fromCharCodes(data);
      _buffer += receivedString;

      while (_buffer.contains(_delimiter)) {
        int index = _buffer.indexOf(_delimiter);
        String completeData = _buffer.substring(0, index);
        _buffer = _buffer.substring(index + 1);

        _processCompleteData(completeData);
      }
    });
  }

  void _processCompleteData(String data) {
    List<double> parsedValues = [];
    List<String> values = data.split(',');

    if (values.length == 5) {
      for (String value in values) {
        try {
          parsedValues.add(double.parse(value.trim()));
        } on FormatException {
          print('Invalid number: $value');
          parsedValues.add(0.0);
        }
      }

      setState(() {
        double timestamp = DateTime.now().millisecondsSinceEpoch / 1000;
        currentData.add(FlSpot(timestamp, parsedValues[0]));
        voltageData.add(FlSpot(timestamp, parsedValues[1]));
        temperatureData.add(FlSpot(timestamp, parsedValues[2]));
        batteryHealth = parsedValues[3];
        batteryPercentage = parsedValues[4];

        // Keep only the last 60 data points
        if (currentData.length > 60) {
          currentData.removeAt(0);
          voltageData.removeAt(0);
          temperatureData.removeAt(0);
        }
      });
      print("Received data: $parsedValues");
    }
  }

  @override
  void dispose() {
    _dataSubscription?.cancel();
    super.dispose();
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(
        title: const Text('Battery Monitor'),
      ),
      body: SingleChildScrollView(
        child: Padding(
          padding: const EdgeInsets.all(16.0),
          child: Column(
            crossAxisAlignment: CrossAxisAlignment.start,
            children: [
              DropdownButton<Device>(
                hint: const Text('Select a paired device'),
                value: _selectedDevice,
                onChanged: (Device? newValue) {
                  setState(() {
                    _selectedDevice = newValue;
                  });
                  if (newValue != null) {
                    _connectToDevice(newValue);
                  }
                },
                items: _pairedDevices.map<DropdownMenuItem<Device>>((Device device) {
                  return DropdownMenuItem<Device>(
                    value: device,
                    child: Text(device.name ?? device.address),
                  );
                }).toList(),
              ),
              const SizedBox(height: 20),
              Text('Connection Status: ${_isConnected ? "Connected" : "Disconnected"}'),
              const SizedBox(height: 20),
              _buildChart("Current", currentData, Colors.blue),
              const SizedBox(height: 20),
              _buildChart("Voltage", voltageData, Colors.green),
              const SizedBox(height: 20),
              _buildChart("Temperature", temperatureData, Colors.red),
              const SizedBox(height: 20),
              _buildBatteryInfo(),
            ],
          ),
        ),
      ),
    );
  }

  Widget _buildChart(String title, List<FlSpot> data, Color color) {
    return Container(
      height: 200,
      padding: const EdgeInsets.all(8),
      child: Column(
        children: [
          Text(title, style: const TextStyle(fontSize: 18, fontWeight: FontWeight.bold)),
          Expanded(
            child: LineChart(
              LineChartData(
                minX: data.isNotEmpty ? data.first.x : 0,
                maxX: data.isNotEmpty ? data.last.x : 1,
                minY: data.isNotEmpty ? data.map((e) => e.y).reduce(min) : 0,
                maxY: data.isNotEmpty ? data.map((e) => e.y).reduce(max) : 1,
                lineBarsData: [
                  LineChartBarData(
                    spots: data,
                    isCurved: true,
                    color: color,
                    barWidth: 3,
                    isStrokeCapRound: true,
                    dotData: FlDotData(show: false),
                    belowBarData: BarAreaData(show: false),
                  ),
                ],
                titlesData: FlTitlesData(show: false),
                gridData: FlGridData(show: false),
                borderData: FlBorderData(show: false),
              ),
            ),
          ),
        ],
      ),
    );
  }

  Widget _buildBatteryInfo() {
    return Column(
      crossAxisAlignment: CrossAxisAlignment.start,
      children: [
        Text('Battery Health: ${batteryHealth.toStringAsFixed(2)}%'),
        const SizedBox(height: 10),
        Text('Battery Percentage: ${batteryPercentage.toStringAsFixed(2)}%'),
      ],
    );
  }
}
