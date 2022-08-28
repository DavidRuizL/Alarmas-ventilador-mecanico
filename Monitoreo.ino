//Inclusion de librerias
#include <LiquidCrystal.h>
LiquidCrystal lcd(53, 51, 50, 49, 48, 47);

//Variables PRESION
volatile bool bandTimer0 = false;
volatile bool bandADC0 = false;
volatile bool bandLCD = false;
volatile bool bandTI= false;
volatile bool bandTE= false;
volatile bool bandON= false;
volatile bool bandbuzz= false;
volatile bool bandbuzz1= false;
unsigned int Vp;
unsigned int ValADCp;
unsigned int P;
unsigned int N;
unsigned int Pmx;
unsigned int Pmx1;
unsigned int PIP;
unsigned int Pmn;
unsigned int Pmn1;
unsigned int PEEP;
unsigned int TR;//  Periodo de la senal
unsigned int FR1;
unsigned int FR;//  Frecuencia de la senal CORREGIDO
unsigned int TI;//  Periodo inspiratorio
unsigned int TE;//  Periodo expiratorio
unsigned int contTE;
unsigned int contTI;
unsigned int contdes;
unsigned int contbuzz;


//VARIABLES PANTALLA

unsigned int D_H=1; // decenas horas
unsigned int U_H=2; // unidades horas
unsigned int D_M=0; // decenas minutos
unsigned int U_M=0; // unidades minutos
unsigned int PIP_SW2=250;
unsigned int FR_SW2=140;
String select_flecha="->";
volatile bool bandButton_3 =false;//Bandera de identificación de interrupción boton 3
volatile bool bandButton_2 =false;//Bandera de identificación de interrupción boton 2
volatile bool bandButton_18=false;//Bandera de identificación de interrupción boton 18
volatile bool bandButton_2v =false;
volatile bool bandButton_18v=false;
volatile bool abajo=true;


//INTERRUPCIONES ADC
ISR (TIMER0_COMPA_vect) {
  bandTimer0 = true;
}



//INTERRUPCIONES LCD
void ButtonPress_3(){// si se presiona el bonton de SELECCIONAR
  bandButton_3=true;  
}
void ButtonPress_2(){ 
  if(bandButton_3==false)
  bandButton_2=!bandButton_2; //bandera para bajar la flecha
  if(bandButton_3==true){
    bandButton_2v=true; //bandera para disminuir la magnitud
  }
}
void ButtonPress_18(){
  bandButton_18=!bandButton_18;//bandera para subir la flecha
  if(bandButton_3==true){
    bandButton_18v==true; //bandera para disminuir magnitud 
  }
}



void setup() {

  //Configuracion del lcd
  lcd.begin(20,4);
  pinMode(6,INPUT); //Para el switch Iniciar/Standby -->abierto=standby cerrado=Iniciar 
  pinMode(7,INPUT); //Para el switch Monitoreo/Alarma -->abierto=alarmas cerrado=monitoreo
  pinMode(3,INPUT); // Entrada digital--> Para botón de ACEPTAR
  digitalWrite(3,HIGH); //Internamente como resistencia pull-up
  pinMode(2,INPUT); // Entrada digital --> Para botón de DISMINUIR/BAJAR
  digitalWrite(2,HIGH); //Internamente como resistencia pull-up
  pinMode(18,INPUT); // Entrada digital --> para botón de AUMENTAR/SUBIR
  digitalWrite(18,HIGH); //Internamente como resistencia pull-up

    //Configuración interrupciones
  // ACEPTAR
  attachInterrupt(digitalPinToInterrupt(3),ButtonPress_3,RISING);
  //DISMINUIR/BAJAR
  attachInterrupt(digitalPinToInterrupt(2),ButtonPress_2,RISING);
  //AUMENTAR/SUBIR
  attachInterrupt(digitalPinToInterrupt(18),ButtonPress_18,RISING);

  
  //Configuracion salida
  pinMode(10, OUTPUT);
  pinMode(11, OUTPUT);
  digitalWrite(10, LOW);
  digitalWrite(11, LOW);
  //Configuracion timer
  //1. Modo de funcionamiento
  bitClear(TCCR0B, WGM02);
  bitSet(TCCR0A, WGM01);
  bitClear(TCCR0A, WGM00);
  //2. Seleccion de reloj
  bitClear(TCCR0B, CS02);
  bitSet(TCCR0B, CS01);
  bitSet(TCCR0B, CS00);
  bitClear(TCCR0A, COM1B1);
  bitClear(TCCR0A, COM1B0);
  bitClear(TCCR0A, COM1A0);
  bitClear(TCCR0A, COM1A1);
  OCR0A= 249;//1KHZ CORREGIDO
  //Habilitacion de interrupcion
  bitSet(TIMSK0, OCIE0A);

}



void loop() {
  //Procesamiento informacion
  //Procesamiento Timer
  if (bandTimer0==true){
    bandTimer0=false;
    bandADC0=true;
    bandLCD=true;
    contdes++; // DIVISION DE MUESTRAS 
    
    // CONTADOR TI
    if (bandTI==true){
      contTI++;
    }
    if (bandTI==false){
      if (contTI!=0){
        TI=contTI;
      }
      contTI=0;
    }
    // CONTADOR TE
    if (bandTE==true){
      contTE++;
    }
    if (bandTE==false){
      if (contTE!=0) {
        TE=contTE;
        TR=((TI+TE)*10);//Periodo de la senal
        N= TR/1000;
        if (TR>1500){
          N=(TR/1000)-1;
          bandON=false;
          
        }
        FR1= 600000/(TR-(232*N));
        FR= (0.44*(FR1)+20);//frecuencia por minuto CORREGIDA
      }
      contTE=0;
    }
  }
  // PIP
  if (contdes == 1){
    Pmx=P;   
  }
  else if (contdes == 2){   
    if (Pmx<P) {
      Pmx1=Pmx;
      bandTI= true;
      bandON=true;     
      
    }    
    else if (Pmx>P) {
      PIP=Pmx1;
      bandTI= false;  
      
    }  
  }
  // PEEP
  if (contdes == 1){
    Pmn=P;   
  }
  if (contdes == 2){
    contdes = 0;
    if (contTE>600){
      PIP=0;
      PEEP=0;
      bandTE= false;
      bandTI= false;       
    }
    else if (Pmn>P) {
      Pmn1=Pmn;
      bandTE= true;
    }    
    else if (Pmn<P) {
      PEEP=Pmn1;
      bandTE= false;
    }
  }
  if (bandLCD == true) {
    bandLCD= false;
    ProcedLCD();
  }
  //Procesamiento ADC
  if (bandADC0==true){
    bandADC0=false;
    ValADCp= analogRead(A1);
    Vp = ((unsigned long)ValADCp*50000)/1023;
    P = ((((unsigned long)Vp)-3.7623)/5105.7)*101.97;//cmH2O con un decimal
  }

//ALARMAS
// PIP ALTO
  if (PIP>=PIP_SW2){
    bandbuzz=true;
  }
  else if(PIP<=PIP_SW2){
    bandbuzz=false;
    
  }

// FR ALTO
  if (FR>=FR_SW2){
    bandbuzz1=true;
  }
  else if(FR<=FR_SW2){
    bandbuzz1=false;
    
  }

  if ((bandbuzz1==false) && (bandbuzz==false)){
    digitalWrite(11,LOW);
  }
  else if ((bandbuzz1==true) || (bandbuzz==true)){
    digitalWrite(11,HIGH);
  }
}
void ProcedLCD() {
  


//condiciones cuando se enciende el ventilador
  //primera fila del LCD en blanco
  // **VENTANA DE MONITOREO - variables en cero____________________________________________________
  if (digitalRead(7)==HIGH){
    lcd.setCursor(0,1);
    //
    lcd.print("PIP:");
    lcd.print(PIP/1000);
    lcd.print(PIP/100);
    lcd.print((PIP % 100) / 10);
    lcd.print(".");
    lcd.print((PIP%10));
    
    lcd.print("  PEEP:");
    lcd.print(PEEP/100);
    lcd.print((PEEP % 100) / 10);
    lcd.print(".");
    lcd.print((PEEP%10));
//

    lcd.setCursor(0,2);

    //
    lcd.print("FR:");
    lcd.print(FR/100);// BPM
    lcd.print((FR % 100) / 10);
    lcd.print(".");
    lcd.print((FR%10));
//
    //lcd.setCursor(9,2);
    lcd.print("  I:E:");
    if (TE>TI) {
     lcd.print("1.0:");
     lcd.print(TE/TI);
     lcd.print(".");
     lcd.print((TE%TI)/10);
    }
    if (TE<TI) {
     lcd.print(TI/TE);
     lcd.print(".");
     lcd.print((TI%TE)/10);
     lcd.print(":1.0");
    }

    if (bandbuzz==true){
    lcd.setCursor(0,0); 
    lcd.print("Alarma: PIP elevada");
    
   }
   else if (bandbuzz1==true){
    lcd.setCursor(0,0); 
    lcd.print("Alarma: FR elevada");
    
   }
   else if ((bandbuzz=false)&&(bandbuzz1==false)){
    lcd.setCursor(0,0); 
    lcd.print("                   ");
   }
   lcd.setCursor(0,3);
   lcd.print("Hora:");// 00:00 am
   lcd.print(D_H);//hora
   lcd.print(U_H);//hora
   lcd.print(":");
   lcd.print(D_M);//minutos             **Reloj entra por serial, inicia en 00:00 am**
   lcd.print(U_M);
   lcd.print(" am");
 }

  
 //VENTANA DE AJUSTE_________________________________________________________________
 else if (digitalRead(7)==LOW) {
   lcd.setCursor(0,1);
   lcd.print(select_flecha);
   lcd.setCursor(2,1);
   lcd.print("PIP:");
   lcd.print(PIP_SW2); //por defecto 30
   lcd.setCursor(2,2);
   lcd.print("FR:"); // FR=30
   lcd.print(FR_SW2);
   lcd.setCursor(0,2);
   lcd.print("  ");
   lcd.setCursor(0,3);
   lcd.print("             ");
   lcd.setCursor(8,1);
   lcd.print("            ");
   lcd.setCursor(8,2);
   lcd.print("                  ");
// PARA LA BANDERA DE SELECCIÓN __________________________________________________
     if (bandButton_3==false){
      if((bandButton_2==true && bandButton_18==true)|| (bandButton_2==!bandButton_18)){ // para que suba o baje la flecha 
        abajo=false;
        while((bandButton_2==true && bandButton_18==true)|| (bandButton_2==!bandButton_18)){
          lcd.setCursor(0,1); //(COLUMNA, FILA)
          lcd.print("  ");
          lcd.setCursor(0,2); //(COLUMNA, FILA)
          lcd.print(select_flecha);
          if(digitalRead(7)==HIGH){
            break;
          }
        }
      }
      else if ((bandButton_2==false && bandButton_18==false)) {
        abajo=true;
        while ((bandButton_2==false && bandButton_18==false)){
          lcd.setCursor(0,1);
          lcd.print(select_flecha);
          lcd.setCursor(0,2); //(COLUMNA, FILA)
          lcd.print("  ");
          if(digitalRead(7)==HIGH){
            break;
          }
        }
        
      }
     }     
     else if(bandButton_3==true){
      //bandButton_3=false;
      digitalWrite(13,HIGH);
      if (bandButton_2v==true){//bandera para disminuir
        bandButton_2v=false;
        
        while(abajo==true){//para saber cual variable se cambia
          lcd.setCursor(6,2);//para la ubicación de la variable
          FR_SW2--;//para hacer la disminucion de la varible
          lcd.write(FR_SW2);
          if(abajo=false){
            break;
          }
        }
      }
     }   
  }
}
