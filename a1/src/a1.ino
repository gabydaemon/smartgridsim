SYSTEM_THREAD(ENABLED);
SerialLogHandler logHandler(LOG_LEVEL_ALL); 

int togglePower(String command);
int resetAll(String empty);

char myID[5]="A1";

void setup() {
    pinMode(D4, OUTPUT);
    pinMode(D5, OUTPUT);
    pinMode(D6, OUTPUT);
    pinMode(D7, OUTPUT);
    //Starting serial via USB
    Serial.begin();

    Mesh.subscribe("PowerNet", commsHandler);
    //registering our functions to cloud!
    Particle.function("toglPwr", togglePower);
    Particle.function("reset", resetAll);

    digitalWrite(D4, HIGH);
    
}
//A3 reads power supplied from X2
//A4 reads power supplied from X1
//A5 reads power supplied form SELF 

void loop() {
  Serial.printf("\n\n----------------------------------\n");
  Serial.print("City:Argon1\n");
//read if power is on
    if (analogRead(A5) >= 3500) {
        Serial.print("Power is ON\nSelf-generated power levels: ");
        Serial.print(analogRead(A5));
        Serial.printf("\n----------------------------------\n");
    //read all analogs to see if power is being recieved
    } else if ((analogRead(A5) < 3000) && (analogRead(A3) < 3000 ) && (analogRead(A4)) < 3000) {  
        Serial.print("Power is OFF\nOutsourced power levels /A4: ");
        Serial.print(analogRead(A4));        
        Serial.print("Power is OFF\nOutsourced power levels /A3: ");
        Serial.print(analogRead(A3));  
        Serial.print("\n\n----------------------------------\n");
        noPower();
    //read all analogs and determine if power is being supplied by others
    } else if (analogRead(A5) < 3000 && (analogRead(A4) > 3000 || analogRead(A3) >3000)){
        Serial.print("\nPower supplied by a gracious neighbor");
        Serial.print("\nOutsourced power levels: ");
        Serial.print(analogRead(A4));
    } else {
        Serial.println("\n\nOops! Please pay $5 BTC for me to work again! \n");
        Serial.print("\nOutsourced power levels /A4: ");
        Serial.print(analogRead(A4));
        Serial.print("\nOutsourced power levels /A3: ");
        Serial.print(analogRead(A3));
        Serial.print("\nSelf-generated power levels /A5: ");
        Serial.print(analogRead(A5));
    }
    delay(2000); 
}

//signal that POWER OFF message was recieved and power is being rerouted
void blink(){
    delay(500);
    digitalWrite(D7, LOW);
    delay(500);
    digitalWrite(D7, HIGH);
    delay(500);
    digitalWrite(D7, LOW);
    delay(500);
    digitalWrite(D7, HIGH);
    delay(500);
    digitalWrite(D7, LOW);
    delay(500);
    digitalWrite(D7, HIGH);
    delay(500);
    digitalWrite(D7, LOW);
}

void reroute(char who[5]){

    //signal that POWER OFF message was recieved and power is being rerouted
    blink();
    //route power
    delay(1000);
    //do not route if already supplying power to someone else
    if (strstr(who, "X1")){
        digitalWrite(D5,HIGH);
        Serial.print("\n\n______________________________");
        Serial.printf("\nREROUTING POWER TO %s", who);
        //Mesh.publish("PowerNet", "A1 - SUPPLYING POWER");
        Serial.print("\n______________________________");
    } else if (strstr(who, "X2")){
        digitalWrite(D6, HIGH);
        Serial.print("\n\n______________________________");
        Serial.printf("\nREROUTING POWER TO %s", who);
        //Mesh.publish("PowerNet", "A1 - SUPPLYING POWER");
        Serial.print("\n______________________________");
    } else {
      Serial.print("these are not the droids you are looking for...");
    }
}



void noPower(){
    Mesh.publish("PowerNet", "A1 - POWER OFF");
    Serial.print("\nAlert sent to mesh network.");
    do {
        Serial.print("\nWaiting for power...\n");
        delay(2300);
        } while((analogRead(A5) > 3900 || analogRead(A5) < 2000) && analogRead(A4) < 3500); 
}
    
void commsHandler(const char *event, const char *data){
    Serial.print("\n\n data:");
    Serial.print(data);
    Serial.print("\n\n");
    char x1[5] = "X1";
    char x2[5] = "X2";
    //char a1[5] = "A1";
    char lowPwr[12] = "POWER OFF";
    char rts[5]= "RTS";
    char cts[5]= "CTS";

        //handle recieved low power messages
    if (strstr(data, lowPwr)){
        if (strstr(data, x1)){
            char *target[5] = {x1};
            Serial.printf("\n%s Has No Power!! \n", *target);
            delay((random(0,41)/random(10,16))*10000);
            Mesh.publish("PowerNet", "A1 - X1 RTS");
        } else if (strstr(data, x2)){
            char *target[5] = {x2};
            Serial.printf("\n%s Has No Power!! \n", *target);
            delay((random(0,41)/random(10,16))*10000);
            Mesh.publish("PowerNet", "A1 - X2 RTS");
        } else { 
        }
    //handle CTS messages
    } else if (strstr(data, cts)){
        if (strstr(data, myID) && strstr(data,x1)){
          reroute(x1);
        } else if (strstr(data, myID) && strstr(data,x2)){
          reroute(x2);
        }
    //handle RTS messages
    } else if (strstr(data, rts)){
        // parse who sent RTS
         if (strstr(data, x1) && strstr(data, myID)){
             //if no one has already sent power
            if (analogRead(A4)<2000 && analogRead(A3) < 3000 && analogRead(A5) < 3000){
                //CTS
                 Mesh.publish("PowerNet", "A1 - X1 CTS");
             }
        } else if (strstr(data, x2) && strstr(data, myID)){
            if (analogRead(A4)<2000 && analogRead(A3) < 3000 && analogRead(A5) < 3000){
                 Mesh.publish("PowerNet", "A1 - X2 CTS");
             }
        } else { 
        } 
    //handle reset messages
    } else if (strstr(data, "RESET")){
        digitalWrite(D5,LOW);
        digitalWrite(D6,LOW);
    } else {}
    //slow down flow
    delay(3000);
}

//cloud commands

int resetAll(String empty){
    Mesh.publish("PowerNet", "RESET");
    digitalWrite(D4, HIGH);
    return 0;
}

int togglePower(String command){
     if (command=="on") {
        digitalWrite(D4,HIGH);
        return 1;
    }
    else if (command=="off") {
        digitalWrite(D4,LOW);
        return 0;
    }
    else {
        return -1;
    }
}
