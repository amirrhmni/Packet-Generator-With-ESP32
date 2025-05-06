#include <Arduino.h>
#include <SD.h>
#include <SPI.h>

// Define the data packet structure
struct DataPacket {
  int index;
  bool Status;
  int Protocol;
  char Time[15];
  float Sensor_Temperature;
  float Sensor_Humidity;
};

const int chipSelect = 5; // Adjust this to your SD card module's CS pin
const char* filename = "/data.txt";

void setup() {
  Serial.begin(115200);
  while (!Serial); // Wait for Serial to be ready (if needed)

  if (!SD.begin(chipSelect)) {
    Serial.println("SD card initialization failed!");
    while (1);
  }
  Serial.println("SD card initialized.");

  // Optional: Create the file with a header if it doesn't exist
  // if (!SD.exists(filename)) {
  //   File dataFile = SD.open(filename, FILE_WRITE);
  //   if (dataFile) {
  //     dataFile.println("Index,Status,Protocol,Time,Temperature,Humidity");
  //     dataFile.close();
  //   }
  // }

  // Example: Save a sample data packet
  DataPacket packet;
  packet.index = 1;
  packet.Status = true;
  packet.Protocol = 1;
  strcpy(packet.Time, "20250506100000");
  packet.Sensor_Temperature = 25.3;
  packet.Sensor_Humidity = 50.0;
  saveDataPacket(packet, filename);
}

void loop() {
  // Call the function to search for Status == 1
  searchPacketsWithStatusOne(filename);
  delay(10000); // Wait 10 seconds before next search
}

// Save a data packet to SD card
void saveDataPacket(DataPacket packet, const char* filename) {
  File dataFile = SD.open(filename, FILE_APPEND);
  if (dataFile) {
    dataFile.print(packet.index);
    dataFile.print(",");
    dataFile.print(packet.Status ? 1 : 0); // Convert bool to int
    dataFile.print(",");
    dataFile.print(packet.Protocol);
    dataFile.print(",");
    dataFile.print(packet.Time);
    dataFile.print(",");
    dataFile.print(packet.Sensor_Temperature);
    dataFile.print(",");
    dataFile.println(packet.Sensor_Humidity);
    dataFile.close();
    Serial.println("Data packet saved.");
  } else {
    Serial.println("Error opening file for writing.");
  }
}

// Search and print all packets where Status == 1
void searchPacketsWithStatusOne(const char* filename) {
  File dataFile = SD.open(filename);
  if (!dataFile) {
    Serial.println("Error opening data file for reading.");
    return;
  }

  Serial.println("Packets with Status = 1:");

  bool headerSkipped = false;
  while (dataFile.available()) {
    String line = dataFile.readStringUntil('\n');
    if (line.length() > 0) {
      // Skip the header line if present
      if (!headerSkipped && line.startsWith("Index,")) {
        headerSkipped = true;
        continue;
      }

      int firstComma = line.indexOf(',');
      int secondComma = line.indexOf(',', firstComma + 1);

      if (firstComma != -1 && secondComma != -1) {
        String statusStr = line.substring(firstComma + 1, secondComma);
        if (statusStr.toInt() == 1) {
          Serial.println(line);
        }
      }
    }
  }

  dataFile.close();
  Serial.println("Search complete.");
}
