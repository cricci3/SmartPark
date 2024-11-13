#define trigpin 9 
#define echopin 10
#define R 5

#define G 4

#define B 3

void setup(){ //serial monitor and pin setup. 
  Serial.begin(9600);
  pinMode(trigpin,OUTPUT); //set trigpin as output
  pinMode(echopin,INPUT);//set echopin as input
  pinMode(R,OUTPUT);// set R,G and B as outputs
  pinMode(G,OUTPUT);
  pinMode(B,OUTPUT); 
  // put your setup code here, to run once:

}

void loop() {
  
  //the trigpin sends out a signal, which bounces off an obstacle and comes back, the 
  //echopin recieves this signal and gives out +5v setting the arduino pin on which it is connected to high.
  //distance= time*speed, but this distnce is divided by 2 because signal sent out returns
  //so distance= (the time it takes for the signal to leave and return)/2.
  //i.e if the time is 6s the distance = (6s/2) = 3m or cm.
  
  int duration, distance;//declare distance and duration as integers
  digitalWrite(trigpin,HIGH);// trigin send out signal
  delayMicroseconds(1000);//coninously for 1000ms
  digitalWrite(trigpin, LOW);// then goes low
  
  duration=pulseIn(echopin,HIGH); // duration is the pulseIn to the echopin
  distance=(duration/2)/29.1; //  the 29.1 is used to convert the distnce to cm, the value varies for other units.
  
  if(distance > 0 && distance <= 20){//distcance is greater than 0 and less than 20cm
    digitalWrite(G,LOW);//green led is off
    digitalWrite(B,LOW);//blue led is off
    delayMicroseconds(500);//delay
    digitalWrite(R,HIGH);//red led is on
    delayMicroseconds(500);
  }
  else if(distance > 20 && distance <= 80){//distcance is greater than 20 and less than 80cm
    digitalWrite(R,LOW);//red led is off
    digitalWrite(G,LOW);//green led is off
    delayMicroseconds(500);
    digitalWrite(B,HIGH);//blue led is on
  }
  else if(distance > 80 && distance <= 120 ){//distcance is greater than 80 and less than 120cm
    digitalWrite(R,LOW);//red led is off
    digitalWrite(B,LOW);//blue led is off
    delayMicroseconds(500);
    digitalWrite(G,HIGH);//green led is on
  }
  Serial.print("cm");
  Serial.println(distance);//print values on serial monitor
  delayMicroseconds(100);
}
  
  
  // put your main code here, to run repeatedly: