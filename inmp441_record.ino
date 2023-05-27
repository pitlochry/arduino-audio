#include "Arduino.h"
#include "AudioTools.h"
#include "SD_MMC.h"

const uint16_t sample_rate = 44100;
const char filename[] = "/AudioFile.wav"; 
unsigned long runtime; 

I2SStream i2sStream_in; // Access I2S as a stream
File audioFile; 

void setup(void) {
  Serial.begin(115200);
  AudioLogger::instance().begin(Serial, AudioLogger::Warning); 

  // Setup SD
  SD_MMC.setPins(PIN_SD_CLK, PIN_SD_CMD, PIN_SD_D0);
  if (!SD_MMC.begin("/sdcard", true, true)) {
    Serial.println("Card Mount Failed");
    return;
  }

  Serial.println("starting I2S input and write streams...");
  EncodedAudioStream i2sStream_write(&audioFile, new WAVEncoder()); // Access audioFile as a stream
  StreamCopy copier(i2sStream_write, i2sStream_in); // copies sound from i2s to audio file stream

  // configure I2S input stream
  auto config_in = i2sStream_in.defaultConfig(RX_MODE);
  config_in.i2s_format = I2S_STD_FORMAT; // if quality is bad change to I2S_LSB_FORMAT #23
  config_in.sample_rate = sample_rate;
  config_in.channels = 1;
  config_in.bits_per_sample = 32;
  config_in.pin_ws = I2S_MIC_WS_IO;
  config_in.pin_bck = I2S_MIC_SCK_IO;
  config_in.pin_data = I2S_MIC_SDO_IO;
  i2sStream_in.begin(config_in);

  // configure I2S write stream
  auto config_write = i2sStream_write.defaultConfig();
  config_write.sample_rate = sample_rate;
  config_write.channels = 1;
  config_write.bits_per_sample = 32;
  i2sStream_write.begin(config_write);
  i2sStream_write.setVolume(5);
  Serial.println("I2S input and write streams started.");

  SD_MMC.remove(filename);
  audioFile = SD_MMC.open(filename, FILE_WRITE); // Open a new file on the SD card for writing to
  if (!audioFile) {
    Serial.println("File open failed.");
    return;
  }
  copier.begin(i2sStream_write, i2sStream_in);
  Serial.println("RECORDING");
  while(runtime<30000) { // loop end after 30 sec of runtime
    runtime = millis();
    copier.copy(); // Copy sound from I2S to the audio file
  }
  audioFile.close(); 
  Serial.println("FILE CLOSED");
}

void loop() {
}
