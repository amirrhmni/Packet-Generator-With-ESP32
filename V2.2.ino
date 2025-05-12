#include <Arduino.h>
#include <SD.h>
#include <SPI.h>
#include <random>

struct DataPacket {
  int index;                  // 4 bytes
  uint32_t Time;             // 4 bytes (Unix timestamp)
  bool Status;               // 1 byte
  float Sensor1;             // 4 bytes
  float Sensor2;             // 4 bytes
};

const int chipSelect = 5;
const char* binFilename = "/data.bin";

void setup() {
  Serial.begin(115200);
  while (!Serial);

  if (!SD.begin(chipSelect)) {
    Serial.println("SD initialization failed!");
    while (1);
  }
  Serial.println("SD card ready.");

  Serial.print("Struct size: ");
  Serial.println(sizeof(DataPacket));  // Should print 17

  // Generate and store 100 million packets
  // This will use approximately 1.7GB of space
  generateAndStorePackets("/data.bin", 1000000);
  
  // Search for packets between 1700001000 and 1700002000 (1000 packets)
  searchPacketsByTimeRange("/data.bin", "1700001000", "1700999999");
}

void loop() {
  // Do nothing
}

// Utility function to print a data packet
void printPacket(const DataPacket& packet) {
  Serial.print("Index: "); Serial.print(packet.index);
  Serial.print(", Time: "); Serial.print(packet.Time);
  Serial.print(", Status: "); Serial.print(packet.Status ? 1 : 0);
  Serial.print(", Sensor1: "); Serial.print(packet.Sensor1, 2);
  Serial.print(", Sensor2: "); Serial.println(packet.Sensor2, 2);
}

void searchPacketsByTimeRange(const char* filename, const char* startTimeStr, const char* endTimeStr) {
  File dataFile = SD.open(filename, FILE_READ);
  if (!dataFile) {
    Serial.println("Error opening file.");
    return;
  }

  Serial.print("File size (bytes): ");
  Serial.println(dataFile.size());

  // Convert string timestamps to uint32_t
  uint32_t startTime = atol(startTimeStr);
  uint32_t endTime = atol(endTimeStr);

  Serial.print("Searching from timestamp ");
  Serial.print(startTime);
  Serial.print(" to ");
  Serial.println(endTime);

  DataPacket packet;
  int matched = 0;

  while (dataFile.available() >= sizeof(DataPacket)) {
    if (dataFile.read((uint8_t*)&packet, sizeof(DataPacket)) != sizeof(DataPacket)) {
      Serial.println("Read error!");
      break;
    }

    if (packet.Time >= startTime && packet.Time <= endTime) {
      printPacket(packet);
      matched++;
    }
  }

  dataFile.close();
  Serial.print("Search complete. Matched packets: ");
  Serial.println(matched);
}

// Function to generate random float between min and max
float randomFloat(float min, float max) {
  return min + (float)random(0, 1000) / 1000.0 * (max - min);
}

// Function to generate and store packets
void generateAndStorePackets(const char* filename, int numPackets) {
  // Remove existing file if it exists
  if (SD.exists(filename)) {
    SD.remove(filename);
  }

  File dataFile = SD.open(filename, FILE_WRITE);
  if (!dataFile) {
    Serial.println("Error creating file!");
    return;
  }

  Serial.print("Generating ");
  Serial.print(numPackets);
  Serial.println(" packets...");
  Serial.print("Estimated file size: ");
  Serial.print((float)numPackets * sizeof(DataPacket) / (1024.0 * 1024.0 * 1024.0), 2);
  Serial.println(" GB");

  // Initialize random seed
  randomSeed(analogRead(0));

  // Start timestamp from 1700000000
  uint32_t currentTimestamp = 1700000000;
  
  for (int i = 0; i < numPackets; i++) {
    DataPacket packet;
    
    // Generate data with sequential timestamp
    packet.index = i;
    packet.Time = currentTimestamp++;
    packet.Status = random(0, 2); // Random boolean
    packet.Sensor1 = randomFloat(20.0, 30.0); // Random temperature between 20-30
    packet.Sensor2 = randomFloat(40.0, 60.0); // Random humidity between 40-60

    // Write packet to file
    dataFile.write((uint8_t*)&packet, sizeof(DataPacket));

    // Print progress every 1% complete
    if ((i + 1) % (numPackets / 100) == 0) {
      Serial.print("Progress: ");
      Serial.print((i + 1) * 100 / numPackets);
      Serial.println("%");
    }
  }

  dataFile.close();
  Serial.println("Packet generation complete!");
}
