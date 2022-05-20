#include <Keypad.h>
#include <Wire.h> 
#include <LiquidCrystal.h>

LiquidCrystal lcd(13, 12, 11, 10, 9, 8);

String first = "";
String second = "";
int maxDigitLength = 3;
char expr;
double total = 0;
int digitPos = 0;

const byte ROWS = 4;
const byte COLS = 4;

char keys[ROWS][COLS] = {
  {'1','2','3','+'},
  {'4','5','6','-'},
  {'7','8','9','*'},
  {'C','0','=','/'}
};
byte rowPins[ROWS] = {7,6,5,4}; //connect to the row pinouts of the keypad
byte colPins[COLS] = {3,2,A4,A5}; //connect to the column pinouts of the keypad

enum {CALCULADORA, CONVERSOR_DEC_BIN, CONVERSOR_BIN_DEC, CONVERSOR_DEC_OCT, CONVERSOR_OCT_DEC} ESTADO = CALCULADORA;
enum {FIRST, SECOND, ANSWERED} AWAITING = FIRST;
enum {DECIMAL, BINARIO, OCTAL} BASE = DECIMAL;

//initialize an instance of class NewKeypad
Keypad customKeypad = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS); 

void setup(){
  lcd.begin(16, 2);               // start lcd
  //lcd.setCursor(0,1);
  lcd.clear();
  lcd.setCursor(0, 0);
  //Serial.begin(9600);
  lcd.print("Calc");
  lcd.setCursor(5, 0);
  lcd.print("Dec");
  lcd.setCursor(0, 0);
}


void loop(){
  keypad();
}

String calculate(){
  long result = 0;
  first = convert(first);
  second = convert(second);
  switch(expr){
    case '+':
    result = first.toInt() + second.toInt();
    break;
    case '-':
    result = first.toInt() - second.toInt();
    break;
    case '*':
    result = first.toInt() * second.toInt();
    break;
    case '/':
    result = first.toInt() / second.toInt();
    break;
  }
  return reconvert(String(result));
}

String reconvert(String n){ // decimal para x
  int base = 0;
  String result = "";
  int num = n.toInt();
  switch(BASE){
    case BINARIO:
    base = 2;
    break;
    case OCTAL:
    base = 8;
    break;	
  }
  if(BASE == DECIMAL || num < base){
    return n;
  }
  while(num >= base){
    result.concat(num % base);
    num = num / base;
    if(num < base)result.concat(num);
  }
  result = invert(result);
  return result;
}

String convert(String str){ // x para decimal
  int base = 0;
  int result = 0;
  switch(BASE){
    case BINARIO:
    base = 2;
    break;
    case OCTAL:
    base = 8;
    break;	
  }
  if(BASE == DECIMAL){
    return str;
  }
  int n = 0;
  for(int i = 0, p = str.length() - 1; i < str.length(); i++, p--){
    n = String(str.charAt(i)).toInt();
    result += n*pow(base, p);
  }
  return String(result);
}

int pow(int base, int expo){
  int result = base;
  if(expo == 1)return base;
  else if(expo == 0) return 1;
    while(expo > 1){
    result = result*base;
    expo--;
  }
  return result;
}

String invert(String str){
  if(str.length() == 1) return str;
  String result = "";
  for(int i = str.length() - 1; i >= 0; i--){
    result += str.charAt(i);
  }
  return result;
}

void error(){
  clear();
  printLcd("ERROR");
  AWAITING = ANSWERED;
}

bool toDecimal = false;
void changeMode(){
  clear();
  lcd.setCursor(0,0);
  if(ESTADO == CALCULADORA){
    BASE = BINARIO;
    ESTADO = CONVERSOR_DEC_BIN;
    toDecimal = false;
    AWAITING = FIRST;
    lcd.print("Conv");
    lcd.setCursor(5,0);
    lcd.print("Dec => Bin");
  }else{
    switch(ESTADO){
      case CONVERSOR_DEC_BIN:
      toDecimal = true;
      ESTADO = CONVERSOR_BIN_DEC;
      lcd.setCursor(5,0);
      lcd.print("Bin => Dec");
      break;
      case CONVERSOR_BIN_DEC:
      toDecimal = false;
      BASE = OCTAL;
      ESTADO = CONVERSOR_DEC_OCT;
      lcd.setCursor(5,0);
      lcd.print("Dec => Oct");
      break;
      case CONVERSOR_DEC_OCT:
      toDecimal = true;
      ESTADO = CONVERSOR_OCT_DEC;
      lcd.setCursor(5,0);
      lcd.print("Oct => Dec");
      break;
      default:
      ESTADO = CALCULADORA;
      BASE = DECIMAL;
      lcd.print("Calc");
      lcd.setCursor(5,0);
      lcd.print("Dec       ");
      break;
    }
  }
}

void changeBase(){
  AWAITING = FIRST;
  if(BASE == DECIMAL){
    BASE = BINARIO;
    lcd.setCursor(5, 0);
    lcd.print("Bin");
  }else if(BASE == BINARIO){
    BASE = OCTAL;
    lcd.setCursor(5, 0);
    lcd.print("Oct");
  }else{
    BASE = DECIMAL;
    lcd.setCursor(5, 0);
    lcd.print("Dec");
  }
}

void printLcd(String s){
  lcd.setCursor(digitPos,1);
  lcd.print(s);
  digitPos += s.length();
}

void clear(){
  for(int i = 0; i < digitPos; i++){
    lcd.setCursor(i,1);
    lcd.print(" ");
  }
  first = "";
  second = "";
  digitPos = 0;
}

void keypad(){
  char key = customKeypad.getKey();
  switch(key){
    case '0' ... '9':
    if(AWAITING == FIRST && first.length() <= maxDigitLength){
      first += key;
      printLcd(String(key));
    }else if(AWAITING == SECOND && second.length() <= maxDigitLength){
      second += key;
      printLcd(String(key));
    }
    break;
    case '+':
    case '-':
    case '*':
    case '/':
    if(AWAITING != FIRST){
      error();
    }else{
      printLcd(String(key));
      expr = key;
      AWAITING = SECOND;
    }
    break;
    case '=':
    if(first == ""){
      changeMode();
    }else if(AWAITING != ANSWERED){
      if(ESTADO == CALCULADORA){
        printLcd("=");
        printLcd(calculate());
      }else{
        printLcd("=>");
        if(toDecimal){
          printLcd(convert(first));
        }else{
          printLcd(reconvert(first));
        }
      }
      AWAITING = ANSWERED;
    }
    break;
    case 'C':
    if(first == "" && ESTADO == CALCULADORA && AWAITING != ANSWERED){
      changeBase();
      clear();
    }else{
      AWAITING = FIRST;
      clear();
    }
    break;
  }
}