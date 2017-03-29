const int buttonPin = 1;
const int clock = 2;
const int data = 3;
const uint8_t red = 12;
const uint8_t green = 13;
const uint8_t yellow = 11; 
uint8_t digits[] = { 0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7d, 0x07, 0x7f, 0x6f }; 
void setup() 
{
  pinMode(buttonPin, INPUT);
  pinMode(clock, OUTPUT);
  pinMode(data, OUTPUT);
  pinMode(red, OUTPUT);
  pinMode(green, OUTPUT);
  pinMode(yellow, OUTPUT);
  setupInterrupt();
  start();
  writeValue(0x8f);
  stop();
  write(0x00, 0x00, 0x00, 0x00);
}

byte tcnt2;
unsigned long time = 0;
void setupInterrupt()
{
  TIMSK2 &= ~(1<<TOIE2);    
  TCCR2A &= ~((1<<WGM21) | (1<<WGM20));   
  TCCR2B &= ~(1<<WGM22);     
  ASSR &= ~(1<<AS2);  
  TIMSK2 &= ~(1<<OCIE2A);    
  TCCR2B |= (1<<CS22)  | (1<<CS20); 
  TCCR2B &= ~(1<<CS21); 
  tcnt2 = 131;     
  TCNT2 = tcnt2;   
  TIMSK2 |= (1<<TOIE2);   
}
ISR(TIMER2_OVF_vect) {   
  TCNT2 = tcnt2; 
  if(time > 0)
  {
    time--;
  }
}  

void loop() 
{
  handleButton();
  displayTime();
}
void handleButton()
{
  const unsigned long sprintDuration = 50000; // change time
  static uint8_t previousButtonState = LOW;

  uint8_t buttonState = digitalRead(buttonPin);
  if(buttonState == LOW && previousButtonState == HIGH)
  {
    time = sprintDuration;
    digitalWrite(yellow, HIGH);
    digitalWrite(green, LOW);
    delay(500);
  }

  previousButtonState = buttonState;
}
void displayTime()
{
  unsigned long t = (unsigned long)(time/1000);  
  uint8_t minutes = (byte)((t / 60) % 60);
  uint8_t seconds = (byte)(t % 60);
  if(t > 0)
  {
    digitalWrite(red, HIGH);
    digitalWrite(green, LOW);
    digitalWrite(yellow, LOW);
  }
  else
  {
    digitalWrite(red, LOW);
    digitalWrite(green, HIGH);
    digitalWrite(yellow, LOW);
  }
   write(digits[minutes / 10], digits[minutes % 10] | ((seconds & 0x01) << 7) , digits[seconds / 10], digits[seconds % 10]);
}
void write(uint8_t first, uint8_t second, uint8_t third, uint8_t fourth)
{
  start();
  writeValue(0x40);
  stop();  
  start(); 
  writeValue(0xc0);
  writeValue(first);
  writeValue(second);
  writeValue(third);
  writeValue(fourth);

  stop();
}
void start(void)
{
  digitalWrite(clock,HIGH);
  digitalWrite(data,HIGH);
  delayMicroseconds(5);
  
  digitalWrite(data,LOW); 
  digitalWrite(clock,LOW); 
  delayMicroseconds(5);
} 
void stop(void)
{
  digitalWrite(clock,LOW);
  digitalWrite(data,LOW);
  delayMicroseconds(5);
  
  digitalWrite(clock,HIGH);
  digitalWrite(data,HIGH); 
  delayMicroseconds(5);
}
bool writeValue(uint8_t value)
{
  for(uint8_t i = 0; i < 8; i++)
  {
    digitalWrite(clock, LOW);
    delayMicroseconds(5);   
    digitalWrite(data, (value & (1 << i)) >> i);
    delayMicroseconds(5);
    digitalWrite(clock, HIGH);
    delayMicroseconds(5);
  } 
  digitalWrite(clock,LOW);
  delayMicroseconds(5);   
  pinMode(data,INPUT);
  digitalWrite(clock,HIGH);     
  delayMicroseconds(5);   
  bool ack = digitalRead(data) == 0;
  pinMode(data,OUTPUT); 
  return ack;
}
