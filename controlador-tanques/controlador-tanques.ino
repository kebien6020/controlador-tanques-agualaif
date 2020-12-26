#include <Nextion.h>

// Pines
constexpr int en1 = 23;
constexpr int en2 = 25;
constexpr int en3 = 27;
constexpr int sa1 = 29;
constexpr int sa2 = 31;
constexpr int sa3 = 33;
constexpr int bomba = 35;
constexpr int salidas[] = {en1, en2, en3, sa1, sa2, sa3, bomba};

constexpr int sensor1 = 39;
constexpr int sensor2 = 41;
constexpr int entradas[] = {sensor1, sensor2};

// Componentes de la interfaz
auto bReset0 = NexButton(0, 4, "b2");
auto bReset1 = NexButton(1, 4, "b2");
auto bReset2 = NexButton(2, 2, "b2");
auto bReset3 = NexButton(3, 4, "b2");
auto bReset4 = NexButton(4, 5, "b2");
auto bReset5 = NexButton(5, 3, "b2");
auto bReset6 = NexButton(6, 6, "b2");
NexButton* todosReset[] = {&bReset0, &bReset1, &bReset2, &bReset3, &bReset4, &bReset5, &bReset6};

auto bLlenar1 = NexButton(1, 2, "llenar1");
auto bLlenar2 = NexButton(1, 3, "llenar2");

auto bRecir1 = NexButton(3, 2, "recir1");
auto bRecir2 = NexButton(3, 3, "recir2");
auto bRecir3 = NexButton(3, 5, "recir3");

auto bTiempo5 = NexButton(4, 2, "tiempo5");
auto bTiempo10 = NexButton(4, 3, "tiempo10");
auto bTiempo20 = NexButton(4, 4, "tiempo20");

auto tMinutos = NexText(5, 4, "minutos");
auto tSegundos = NexText(5, 5, "segundos");

auto tTanque = NexText(6, 3, "tanque");
auto tTiempo = NexText(6, 5, "tiempo");

// Componentes que generan eventos
NexTouch* nex_listen_list[] = {
  &bReset0,
  &bReset1,
  &bReset2,
  &bReset3,
  &bReset4,
  &bReset5,
  &bReset6,
  &bLlenar1,
  &bLlenar2,
  &bRecir1,
  &bRecir2,
  &bRecir3,
  &bTiempo5,
  &bTiempo10,
  &bTiempo20,
  NULL
};

// Variables de los temporizadores
unsigned long long temporizadorEncenderBomba = -1;
unsigned long long temporizadorRecirculado = -1;
unsigned long long temporizadorActualizarPantalla = -1;
int tanqueSeleccionado = -1;
int minutosRecirculados = -1;

// Estados
constexpr int NADA = 0;
constexpr int LLENANDO1 = 1;
constexpr int LLENANDO2 = 2;
constexpr int RECIRCULANDO1 = 3;
constexpr int RECIRCULANDO2 = 4;
constexpr int RECIRCULANDO3 = 5;


int estado = NADA;

// Callbacks de botones
void resetCallback(void*) {
  for (int pin : salidas) {
    digitalWrite(pin, HIGH);
  }
  temporizadorEncenderBomba = -1;
  tanqueSeleccionado = -1;
  temporizadorRecirculado = -1;
  temporizadorActualizarPantalla = -1;
  minutosRecirculados = -1;
  estado = NADA;
}

void llenar1Callback(void*) {
  digitalWrite(en1, LOW);
  digitalWrite(sa3, LOW);
  temporizadorEncenderBomba = millis() + 3000;
  estado = LLENANDO1;
}

void llenar2Callback(void*) {
  digitalWrite(en2, LOW);
  digitalWrite(sa3, LOW);
  temporizadorEncenderBomba = millis() + 3000;
  estado = LLENANDO2;
}

void recir1Callback(void*) { tanqueSeleccionado = 1; }
void recir2Callback(void*) { tanqueSeleccionado = 2; }
void recir3Callback(void*) { tanqueSeleccionado = 3; }

void recircular(int tanque, int minutos) {
  switch (tanque) {  
  case 1:
    digitalWrite(en1, LOW);
    digitalWrite(sa1, LOW);
    estado = RECIRCULANDO1;
    break;
  case 2:
    digitalWrite(en2, LOW);
    digitalWrite(sa2, LOW);
    estado = RECIRCULANDO2;
    break;
  case 3:
    digitalWrite(en3, LOW);
    digitalWrite(sa3, LOW);
    estado = RECIRCULANDO3;
    break;
  default:
    return;
  }

  temporizadorRecirculado = millis() + static_cast<unsigned long long>(minutos) * 60ull * 1000ull;
  temporizadorEncenderBomba = millis() + 3000;
  
  temporizadorActualizarPantalla = millis();
  minutosRecirculados = minutos;
}

void tiempo5Callback(void*)  { recircular(tanqueSeleccionado, 5); }
void tiempo10Callback(void*) { recircular(tanqueSeleccionado, 10); }
void tiempo20Callback(void*) { recircular(tanqueSeleccionado, 20); }

void setup() {
  // Inicializaciones
  Serial.begin(9600);
  {
    auto success = nexInit();
    while(Serial2.available()) { Serial.print((uint8_t)Serial2.read()); }
    if (success)
      dbSerialPrintln("Nex succesfully initialized");
  }

  // Eventos
  for (NexButton* bReset : todosReset) {
    bReset->attachPop(resetCallback);
  }
  bLlenar1.attachPop(llenar1Callback);
  bLlenar2.attachPop(llenar2Callback);
  bRecir1.attachPop(recir1Callback);
  bRecir2.attachPop(recir2Callback);
  bRecir3.attachPop(recir3Callback);
  bTiempo5.attachPop(tiempo5Callback);
  bTiempo10.attachPop(tiempo10Callback);
  bTiempo20.attachPop(tiempo20Callback);
  
  // pin setup
  for (int pin : salidas) {
    pinMode(pin, OUTPUT);
  }
  for (int pin : entradas) {
    pinMode(pin, INPUT_PULLUP);
  }
  
  // All off on init
  for (int pin : salidas) {
    digitalWrite(pin, HIGH);
  }

  dbSerialPrintln("Setup done");
}

void loop() {
  // Temporizadores
  auto ahora = millis();
  if (ahora >= temporizadorEncenderBomba && estado != NADA) {
    digitalWrite(bomba, LOW);
    temporizadorEncenderBomba = -1;
  }
  
  if (ahora >= temporizadorRecirculado && estado != NADA) {
    digitalWrite(bomba, HIGH);
    
    if (estado == RECIRCULANDO1) {
      digitalWrite(en1, HIGH);
      digitalWrite(sa1, HIGH);
    } else if (estado == RECIRCULANDO2) {
      digitalWrite(en2, HIGH);
      digitalWrite(sa2, HIGH);
    } else if (estado == RECIRCULANDO3) {
      digitalWrite(en3, HIGH);
      digitalWrite(sa3, HIGH);
    }
    temporizadorRecirculado = -1;
    temporizadorActualizarPantalla = -1;
    sendCommand("page 6");

    if (estado == RECIRCULANDO1) {
      tTanque.setText("1");
    } else if (estado == RECIRCULANDO2) {
      tTanque.setText("2");
    } else if (estado == RECIRCULANDO3) {
      tTanque.setText("3");
    }

    char minutosRecircText[20]; sprintf(minutosRecircText, "%d minutos", minutosRecirculados);
    tTiempo.setText(minutosRecircText);

    estado = NADA;
  }

  if (ahora >= temporizadorActualizarPantalla) {
    if (estado == RECIRCULANDO1 || estado == RECIRCULANDO2 || estado == RECIRCULANDO3) {
      auto tiempoRestante = temporizadorRecirculado - ahora;
      auto segundosRestantes = static_cast<int>(tiempoRestante / 1000ull) % 60;
      auto minutosRestantes = static_cast<int>(tiempoRestante / 1000ull) / 60;

      char segundosTexto[3]; sprintf(segundosTexto, "%02d", segundosRestantes);
      char minutosTexto[3]; sprintf(minutosTexto, "%02d", minutosRestantes);

      tSegundos.setText(segundosTexto);
      tMinutos.setText(minutosTexto);
      
      temporizadorActualizarPantalla = millis() + 1000;
    } else {
      temporizadorActualizarPantalla = -1;
    }
  }

  // Sensores
  auto flotador1 = digitalRead(sensor1);
  auto flotador2 = digitalRead(sensor2);

  if (flotador1 == LOW && estado == LLENANDO1) {
    
    digitalWrite(en1, HIGH);
    digitalWrite(sa3, HIGH);
    digitalWrite(bomba, HIGH);
    sendCommand("page 0");
    estado = NADA;
  }

  if (flotador2 == LOW && estado == LLENANDO2) {
    digitalWrite(en2, HIGH);
    digitalWrite(sa3, HIGH);
    digitalWrite(bomba, HIGH);
    sendCommand("page 0");
    estado = NADA;
  }

  
  
  nexLoop(nex_listen_list);
}
