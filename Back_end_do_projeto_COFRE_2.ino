#include <Keypad.h>
#include <LiquidCrystal_I2C.h>
#include <Stepper.h>

// === DEFINIÇÕES DE PINOS ===
#define IN1 4       
#define IN3 5
#define IN2 34 
#define IN4 18 
#define BUZZER_PIN 2

#define ROW1 32
#define ROW2 14
#define ROW3 27
#define ROW4 26

#define COL1 25
#define COL2 33
#define COL3 23
#define COL4 13

// === LCD ===
LiquidCrystal_I2C lcd(0x27, 16, 2);

// === TECLADO ===
const byte ROWS = 4;
const byte COLS = 4;
char keys[ROWS][COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};

byte rowPins[ROWS] = {ROW1, ROW2, ROW3, ROW4};
byte colPins[COLS] = {COL1, COL2, COL3, COL4};

Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

// === MOTOR 28BYJ-48 ===
const int stepsPerRevolution = 2048; // Para uma volta completa com redutor
Stepper motor(stepsPerRevolution, IN1, IN3, IN2, IN4); // Ordem correta de pinos

// === VARIÁVEIS ===
String senhaatual = "1234"; // senha padrão inicial
String senhadigitada;
int bloqueios = 0;
int senhaserradas = 3;
bool estadoporta = false;

void setup() {
  lcd.init();
  lcd.backlight();

  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);

  motor.setSpeed(20);  // Velocidade em RPM

  lcd.clear();
  lcd.print("Cofre fechado");
  lcd.setCursor(0, 1);
  lcd.print("Aperte p/ nova");
}

void loop() {
  char tecla = keypad.getKey();
  if (tecla) {
    if (tecla == 'A') {
      cadastrarNovaSenha();
    } else if (tecla == 'B') {
      fecharporta();
    } else if (tecla == '#') {
      if (senhadigitada == senhaatual) {
        abrirporta();
        senhaserradas = 3;
      } else {
        errosdesenha();
      }
      senhadigitada = "";
    } else if (tecla == '*') {
      if (senhadigitada.length() > 0) {
        senhadigitada.remove(senhadigitada.length() - 1);
        updateDisplay();
      }
    } else if (tecla >= '0' && tecla <= '9') {
      senhadigitada += tecla;
      updateDisplay();
    }
  }
}

void cadastrarNovaSenha() {
  lcd.clear();
  lcd.print("Nova senha");
  lcd.setCursor(0, 1);
  lcd.print("Digite-a:");

  String novaSenha = "";

  while (true) {
    char tecla = keypad.getKey();
    if (tecla) {
      if (tecla == '#') {
        if (novaSenha.length() >= 4) {
          senhaatual = novaSenha;
          lcd.clear();
          lcd.print("Senha salva!");
          delay(2000);
          resetDisplay();
          return;
        } else {
          lcd.clear();
          lcd.print("Min: 4 digitos");
          delay(2000);
          resetDisplay();
          return;
        }
      } else if (tecla == '*') {
        if (novaSenha.length() > 0) {
          novaSenha.remove(novaSenha.length() - 1);
        }
      } else if (tecla >= '0' && tecla <= '9') {
        novaSenha += tecla;
      }
      lcd.setCursor(0, 1);
      lcd.print("                ");
      lcd.setCursor(0, 1);
      for (int i = 0; i < novaSenha.length(); i++) lcd.print('*');
    }
  }
}

void abrirporta() {
  if (!estadoporta) { // só abre se estiver fechada
    lcd.clear();
    lcd.print("Abrindo...");

    motor.step(stepsPerRevolution);  // 1 volta = destrancar
    estadoporta = true;

    lcd.clear();
    lcd.print("Porta Aberta");
    delay(1500);
    lcd.clear();
    lcd.print("Aperte B p/ fechar");
  } else {
    lcd.clear();
    lcd.print("Ja esta aberta");
    delay(1500);
    resetDisplay();
  }
}


void fecharporta() {
  if (estadoporta) { // só fecha se estiver aberta
    lcd.clear();
    lcd.print("Fechando...");

    motor.step(-stepsPerRevolution); // 1 volta reversa = trancar
    estadoporta = false;

    lcd.clear();
    lcd.print("Cofre Fechado");
    delay(1500);
    resetDisplay();
  } else {
    lcd.clear();
    lcd.print("Ja esta fechado");
    delay(1500);
    resetDisplay();
  }
}


void errosdesenha() {
  senhaserradas--;
  if (senhaserradas <= 0) {
    triggerAlarm();
  } else {
    lcd.clear();
    lcd.print("Senha Incorreta!");
    lcd.setCursor(0, 1);
    lcd.print("Tentativas: " + String(senhaserradas));
    delay(2000);
    resetDisplay();
  }
}

void triggerAlarm() {
  bloqueios++;
  lcd.clear();
  lcd.print("SISTEMA BLOQ.");
  lcd.setCursor(0, 1);
  lcd.print("ALARME ATIVADO!");

  for (int i = 0; i < 3000; i++) {
    digitalWrite(BUZZER_PIN, HIGH);
    delayMicroseconds(125);
    digitalWrite(BUZZER_PIN, LOW);
    delayMicroseconds(125);
  }

  senhaserradas = 3;
  resetDisplay();
}

void resetDisplay() {
  lcd.clear();
  lcd.print("Digite a senha:");
}

void updateDisplay() {
  lcd.setCursor(0, 1);
  lcd.print("                ");
  lcd.setCursor(0, 1);
  for (int i = 0; i < senhadigitada.length(); i++) {
    lcd.print('*');
  }
}
