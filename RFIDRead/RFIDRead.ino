#include <SPI.h>
#include <MFRC522.h>
#include <Ethernet.h>

#define RST 2
#define SCK 9

boolean isDb = false;

byte mac[] = {0x90,0xA2,0xDA,0x0D,0x0E,0xDA};
IPAddress server_ip(192,168,1,1);
char server[] = "localhost";
IPAddress ip(192,168,1,2);

EthernetClient client;

uint8_t successRead;

byte cardIdDb[4];
byte cardReaded[4];

constexpr uint8_t RST_PIN = RST;
constexpr uint8_t SS_PIN = SCK;

MFRC522 mfrc522(SS_PIN,RST_PIN);

void showDetails() {
	byte v = mfrc522.PCD_ReadRegister(mfrc522.VersionReg);
	Serial.print("MFC522 Software version 0x");
	Serial.print(v,HEX);
  Serial.println("");
	if((v==0x00) || (v==0xFF)) {
		Serial.println("WARNING: Communication failure");
		Serial.println("System halter: Check connections");
		Serial.println("PRESS RESET");
		while(true);
	}
}

uint8_t getUID() {
	if(!mfrc522.PICC_IsNewCardPresent()) {
		return 0;
	}
	if(!mfrc522.PICC_ReadCardSerial()) {
		return 0;
	}

	Serial.println("Scanned UID : ");
	uint8_t i;
	for(i=0;i<4;i++) { //PENSER A FAIRE LA DIFF ENTRE 4 ET 7 BITS
		cardReaded[i] = mfrc522.uid.uidByte[i];
		Serial.print(cardReaded[i],HEX);
	}
	Serial.println("");
	mfrc522.PICC_HaltA();
	return 1;
}

void setup()
{
  Serial.begin(9600);
  Serial.println("STARTING CONFIGURATION...");

  Serial.println("Config Pins");
  pinMode(SS_PIN,OUTPUT);
  pinMode(10,OUTPUT);

  //--------------SETTING UP ETHERNET
  Serial.println("Enabling Ethernet Chip");
  digitalWrite(SS_PIN,HIGH);
  digitalWrite(10,LOW);
  
	Ethernet.begin(mac, ip);
  Serial.println(Ethernet.localIP());
  
  delay(1000);
  Serial.println("Connecting...");
  if(client.connect(server_ip, 8080)) {
    Serial.println("CONNECTED TO REST");

    client.println("GET /RestTest/webapi/nfcaccess/get/c30e399 HTTP/1.1");
    client.println("Host: 192.168.1.1");
    //client.println("Connection: close");
    client.println("");
    while(client.available()) {
      char c = client.read();
      Serial.print(c);
    }
    
  } else {
    Serial.println("connection failed");
  }
  
  Serial.print("Ethernet LOCAL IP: ");
  Serial.println(Ethernet.localIP());
  //---------------END ETHERNET SET UP
  
  Serial.println("Enabling RFID Chip");
  digitalWrite(SS_PIN,LOW);
  digitalWrite(10,HIGH);
  
  SPI.begin();
	mfrc522.PCD_Init();

	mfrc522.PCD_SetAntennaGain(mfrc522.RxGain_max);

	Serial.println("Launching hardware");
	showDetails();

	Serial.println("EVERYTHING READY. WAITING FOR TARGET...");
  
}

void loop()
{
	do {
		successRead = getUID();
	} while(!successRead);
  if(successRead) {
    delay(1000);
  }

	
}

