#include <SPI.h>
#include <MFRC522.h>
#include <Ethernet.h>
#include <string.h>

#define RST 2
#define SCK 9
#define TIME_DOOR_OPENED 50

boolean isDb = false;

byte mac[] = {0x90,0xA2,0xDA,0x0D,0x0E,0xDA};
IPAddress server_ip(192,168,1,1);
char server[] = "localhost";
IPAddress ip(192,168,1,2);

EthernetClient client;

uint8_t successRead;

byte cardIdDb[4];
byte cardReaded[4];
char *uid;
constexpr uint8_t RST_PIN = RST;
constexpr uint8_t SS_PIN = SCK;
constexpr uint8_t RELAY = 6;

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

const char* hexToStr(byte* hex,size_t len) {
  byte* octet = (byte*) hex;
  int i=0;
  char hexa[len];
  char hexstring[20] = "";

  while(i<len) {
    sprintf(hexa,"%02x",octet[i]);
    strcat(hexstring,hexa);
    //printf("%02x",octet[i++]);
    //fflush(stdout);
    i++;
  }
  char *str = malloc(sizeof(char)*len+1);

  hexstring[19] = '\0';
  //str = strdup(hexstring);
  return hexstring;

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
  uid = hexToStr(cardReaded,4);
 
	Serial.println("");
	mfrc522.PICC_HaltA();
	return 1;
}


void enableEth() {
  Serial.println("Enabling Ethernet Chip");
  digitalWrite(SS_PIN,HIGH);
  digitalWrite(10,LOW);
}

void enableRFID() {
  Serial.println("Enabling RFID Chip");
  digitalWrite(SS_PIN,LOW);
  digitalWrite(10,HIGH);
}

void parseResponse(char c) {
  
}
void handleUid() {
  
  enableEth();
  boolean received = false;
  delay(200);
 
 if(client.connect(server_ip,8080)) {
    Serial.print("SENDING REQUEST FOR UID ");
    Serial.print(uid);
    Serial.println("");

    String req = "GET /RestTest/webapi/nfcaccess/get/";
    req.concat(uid);
    req.concat(" HTTP/1.1");
    client.println(req);
    client.println("Host: 192.168.1.1");
    
    client.println("Connection: keep-open");
    client.println("");
 } 
  char c = -1;
  int httprep=0;
  int index=0;
  String httpHead;
  String response;
  while(client.available()) {
    c = client.read();
    if(index == 1) {
      httpHead.concat(c);
    }
    if(c == ' ') index++;
    response.concat(c);
    received = true;
  }
  Serial.println(httpHead);
  httprep = httpHead.toInt();
  Serial.print(response);
  if(httprep == 200) {
    Serial.println();
    Serial.println("ACCESS GRANTED");
    digitalWrite(RELAY,LOW);
    delay(TIME_DOOR_OPENED);
    digitalWrite(RELAY,HIGH);
  }
  if(received) {
    client.stop();
  }
  
  Serial.println("");
  
  enableRFID();
  
  
  
}
void setup()
{
  pinMode(RELAY,OUTPUT);
  digitalWrite(RELAY,HIGH);
  Serial.begin(9600);
  Serial.println("STARTING CONFIGURATION...");

  Serial.println("Config Pins");
  pinMode(SS_PIN,OUTPUT);
  pinMode(10,OUTPUT);

  //--------------SETTING UP ETHERNET
  enableEth();
  
	Ethernet.begin(mac, ip);
  Serial.println(Ethernet.localIP());
  
  delay(1000);
  Serial.println("Connecting...");
  if(client.connect(server_ip, 8080)) {
    Serial.println("CONNECTED TO REST");

    client.println("GET /RestTest/webapi/nfcaccess/get/0c30e399 HTTP/1.1");
    client.println("Host: 192.168.1.1");
    client.println("Connection: keep-open");
    client.println("");
    
  } else {
    Serial.println("connection failed");
  }
  client.stop();
  
  Serial.print("Ethernet LOCAL IP: ");
  Serial.println(Ethernet.localIP());
  
  //---------------END ETHERNET SET UP
  enableRFID();
  
  
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
    handleUid();
    
  }
  
  
}

