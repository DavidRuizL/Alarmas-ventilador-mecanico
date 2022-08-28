
//Variables globales
volatile bool bandRX=false;
volatile bool bandTimer1= false;
volatile bool bandADC1= false; //Bandera de lecura del ADC 0
unsigned int contTimer1=0; // Contador asociado al número de interrupciones del Timer 0
unsigned int ContDuty=2399;// el Duty comienza en un valor del 15%
unsigned int ValADC1; // Valor ADC temperatura
unsigned int V;//Valor de temperatura
unsigned int T;//Valor de voltaje


void serialEvent(){
  bandRX=true;
}

ISR(TIMER1_COMPA_vect){ //Rutina de atención a la interrupción 
  OCR1B=ContDuty;
  bandTimer1 = true; // Bandera del PWM  
}

void setup() {  
  // Configuración UART
  Serial.begin(115200,SERIAL_8N1);
  while(!(Serial)){};
  //Serial.println("Inicio Refrigeración");
  //Configuración pines digitales
//  pinMode(11, OUTPUT);// LED alarmas
//  digitalWrite(11, LOW);// Cuando se enciende el ventilador las alarmas estan apagadas
//  pinMode(13, OUTPUT);//Sonido
//  digitalWrite(13, LOW);

  //REFRIGERANCIÓN -> Configuración timer
  //1. Asociación del PWM modo directo, en relación con el pin OC1B
  bitSet(TCCR1A, COM1B1);
  bitClear(TCCR1A, COM1B0);
  bitClear(TCCR1A, COM1A0);
  bitClear(TCCR1A, COM1A1);
  //2. Modo de funcionamiento fast PWM 16 bits
  bitSet(TCCR1A, WGM10);
  bitSet(TCCR1A, WGM11);
  bitSet(TCCR1B, WGM12);
  bitSet(TCCR1B, WGM13);
  //3. Selección entrada de reloj, prescaler y máximo conteo
  bitClear(TCCR1B,CS12);
  bitClear(TCCR1B,CS11);
  bitSet(TCCR1B,CS10);
  OCR1A=15999; // máximo conteo para la frecuencia de 1Khz para 16 bits
  //4. Duty%,
  OCR1B= ContDuty; // Valor de conteo para el duty por defecto el 15%
  //5.Habilitación de la interrupción
  bitSet(TIMSK1,OCIE1A);
  //6. Definición del pin de salida
  pinMode(12, OUTPUT);// Ventilación
}

void loop() {
  //Procesamiento información serial
  if (bandRX == true){
    bandRX=false;
    ProcedSerial();
  }
  // Procesamiento timer PWM
  if(bandTimer1 == true){
    bandTimer1= false;
    contTimer1++;
    if(contTimer1>=1000){//para leer la entrada ADC cada segundo
      bandADC1=true;
      contTimer1=0;
    }
  }
  //Procesamiento ADC de temperatura
  if(bandADC1==true){
    bandADC1=false;
    ValADC1=analogRead(A0);//Lee el valor de ADC0
    V=((unsigned long)ValADC1*50000)/1023; //Convierte ACD a voltaje
    T=((unsigned long)V*10-574)/487; //Convierte a temperatura *10
    if ((0<=T) && (T<300)){
      ContDuty=(16000*0.15)-1;
    }
    else if ((300<=T) && (T<500)){
      ContDuty=(16000*0.30)-1;
      }
    else if ((500<=T) && (T<700)){
      ContDuty=(16000*0.60)-1;
    }
    else{
      ContDuty=16000-1;
    }
  }
}
void ProcedSerial(){
  unsigned char dato;
  if(Serial.available() != -1){
    dato= Serial.read();
    if (dato==13){
      mostrarVal();
    }
  }
}
void mostrarVal(){
//  Serial.print("Temp.: ");
//  Serial.print(T/10);
//  Serial.print(".");
//  Serial.print(T%10);
//  Serial.println("C");
  Serial.println(T);
  Serial.print(V);
}
