#include <iostream>
#include <fstream>
#include <string>
#include <iomanip>
#include <time.h>

using namespace std;

//CONSTANTES
const int MODO_DEBUG = false;

const int CENTINELA = 0;

const int CASILLA_INICIAL = 1;
const int NUN_CASILLAS = 63;

const int TURNOS_POSADA = 1;
const int TURNOS_CARCEL = 2;
const int TURNOS_POZO = 3;

const int RETROCESO_LABERINTO = 12;

const int NUM_JUGADORES = 2;
const int MAX_JUGADORES = 4;

const int MAX_PARTIDAS = 10;

const int NUM_FILAS_A_DIBUJAR = 3;

//ARRAYS
typedef enum { NORMAL, OCA, PUENTE1, PUENTE2, POZO, POSADA, LABERINTO, DADO1, DADO2, CARCEL, CALAVERA } tCasilla;
typedef tCasilla tTablero[NUN_CASILLAS];

struct tEstadoJugador {
    int casillas;
    int penalizaciones;
};

typedef  tEstadoJugador tEstadoJugadores[NUM_JUGADORES];

struct tEstadoPartida {
    tEstadoJugadores jugador;
    tTablero tablero;
    int turno;
};

typedef tEstadoPartida tArrayPartidas[MAX_PARTIDAS];

struct tListaPartidas {
    int contador;
    tArrayPartidas arrayPartidas;
};

//PROTOTIPOS
//V1---------------------------------------------------------------------
int tirarDado();
int tirarDadoManual();
int quienEmpieza();
bool esMeta(int casilla);

//V2---------------------------------------------------------------------
//SUSTITUTO DE LA V1
bool esCasillaPremio(const tTablero tablero, int casilla);

//CARGAR
void iniciaTablero(tTablero tablero);
string casillaAstring(tCasilla casilla);
tCasilla stringACasilla(string nombre);
string casillaAstringGuardar(tCasilla casilla);

//SALTA A CASILLA
int saltaACasilla(const tTablero tablero, int casillaActual);
void buscaCasillaAvanzando(const tTablero tablero, tCasilla tipo, int& posicion);
void buscaCasillaRetrocediendo(const tTablero tablero, tCasilla tipo, int& posicion);

//SUBPROGRAMAS PARA PINTAR EL TABLERO O MOSTRAR INFORMACION
void pintaNumCasilla(int fila, int casillasPorFila);
void pintaBorde(int casillasPorFila);
void pintaTipoCasilla(const tTablero tablero, int fila, int casillasPorFila);
void pintaJugadores(const tEstadoJugadores casillasJ, int fila, int casillasPorFila);

//V3---------------------------------------------------------------------
//PINTAR TABLERO
void pintaTablero(const tEstadoPartida& partida);

//LOGICA DE TIRADAS
void iniciaJugadores(tEstadoJugadores jugadores);

//EFECTO TIRADA
void efectoTirada(const tTablero tablero, tEstadoJugador& estadoJug);

//PARTIDO
int partida(tEstadoPartida& estado);

//TIRADA
void tirada(const tTablero tablero, tEstadoJugador& estadoJug);

//CARGAR PARTIDA
bool cargaPartidas(tListaPartidas& partidas);
void cargaTablero(tTablero tablero, ifstream& archivo);
void cargaJugadores(tEstadoJugadores& jugadores, ifstream& archivo);

//ELIMINAR PARTIDA
void eliminarPartida(tListaPartidas& partidas, int indice);

//INSERTAR NUEVA PARTIDA
bool insertarNuevaPartida(tListaPartidas& partidas, const tEstadoPartida& partidaOca);

//GUARDAR PARTIDA
void guardarPartida(const tListaPartidas& partidas);
void guardaTablero(const tTablero tablero, ofstream& archivo);
void guardarJugadores(const tEstadoJugadores jugadores, ofstream& archivo);

//----------------------------------------------------------

int main() {

    tListaPartidas partidas;
    ifstream archivoNuevaPartida;
    char opcionPartida;
    int opcionPartidaExistente = -1;
    int ganador = -1;
    string nombreArchivoNuevo;

    srand(time(NULL));

    if (!cargaPartidas(partidas)) {
        cout << "No se ha podido abrir correctamente el archivo!" << endl;
    }

    else {

        cout << "Fichero abierto correctamente" << endl << endl;
        cout << "Deseas cargar un fichero nuevo o existente? (n = nuevo, e = existente): ";
        cin >> opcionPartida;

        if (opcionPartida == 'N' || opcionPartida == 'n') {

            cout << "Vas a jugar una partida nueva." << endl;
            cout << "Dame el nombre del fichero que contiene el tablero de la oca: ";

            cin >> nombreArchivoNuevo;

            archivoNuevaPartida.open(nombreArchivoNuevo);

            if (insertarNuevaPartida(partidas, partidas.arrayPartidas[partidas.contador])){
                cargaTablero(partidas.arrayPartidas[partidas.contador - 1].tablero, archivoNuevaPartida);
                iniciaJugadores(partidas.arrayPartidas[partidas.contador - 1].jugador);
                partidas.arrayPartidas[partidas.contador - 1].turno = quienEmpieza();
                ganador = partida(partidas.arrayPartidas[partidas.contador - 1]);
            }
        }

        else if (opcionPartida == 'E' || opcionPartida == 'e') {

            if (partidas.contador == 1) {
                cout << "El unico identificador disponible es el 1" << endl;
            }

            else {
                cout << "Los identificadores disponibles son: ";

                for (int i = 0; i < partidas.contador; i++) {
                    cout << i + 1 << " ";
                }
            }

            cout << "Que partida quiere continuar? " << endl;
            cin >> opcionPartidaExistente;
            opcionPartidaExistente--;

            ganador = partida(partidas.arrayPartidas[opcionPartidaExistente]);
        }

        else {
            cout << "Opcion introducida no valida!" << endl;
        }

        //SI SALE DEL BUCLE DE PARTIDA O HAY GANADOR O ES ABANDONAR PARTIDA
        if (ganador != -1 ) {
            cout << endl << "------ GANA EL JUGADOR " << ganador + 1 << "------" << endl;
            
            //SI ES UNA PARTIDA EXISTENTE, QUE SE ELIMINE LA PARTIDA
            if ((opcionPartida == 'E' || opcionPartida == 'e')) {
                eliminarPartida(partidas, opcionPartidaExistente);
                guardarPartida(partidas);
            }
        }

        else {
            cout << "PARTIDA SIN ACABAR" << endl;
            guardarPartida(partidas);
        }
    }

    return 0;
}

//----------------------------------------------------------
//CARGAR PARTIDA, TABLERO Y JUGADORES
//----------------------------------------------------------

bool cargaPartidas(tListaPartidas& partidas) {

    ifstream archivo;
    string nombreArchivo;
    char opcion;
    bool abierto;
    int opcionPartida = -1;

    cout << "Dame el nombre del archivo que contiene las partidas: ";
    cin >> nombreArchivo;

    archivo.open(nombreArchivo);
    abierto = archivo.is_open();

    if (abierto) {

        archivo >> partidas.contador;

        for (int i = 0; i < partidas.contador; i++) {
            cargaTablero(partidas.arrayPartidas[i].tablero, archivo);
            archivo >> partidas.arrayPartidas[i].turno;
            partidas.arrayPartidas[i].turno--;
            cargaJugadores(partidas.arrayPartidas[i].jugador, archivo);
        }

        archivo.close();
    }

    return abierto;
}

//----------------------------------------------------------

void cargaTablero(tTablero tablero, ifstream& archivo) {

    int posicion;
    string casillaNombre;
    tCasilla casilla;

    iniciaTablero(tablero);

    archivo >> posicion;

    while (posicion != CENTINELA) {
        archivo >> casillaNombre;
        casilla = stringACasilla(casillaNombre);
        tablero[posicion - 1] = casilla;
        archivo >> posicion;
    }
}

//----------------------------------------------------------

void cargaJugadores(tEstadoJugadores& jugadores, ifstream& archivo) {

    for (int i = 0; i < NUM_JUGADORES; i++) {
        archivo >> jugadores[i].casillas >> jugadores[i].penalizaciones;
        jugadores[i].casillas--;
    }
}

//----------------------------------------------------------
//ELIMINAR PARTIDA
//----------------------------------------------------------

void eliminarPartida(tListaPartidas& partidas, int indice) {

    for (int i = indice; i < partidas.contador; i++) {
        partidas.arrayPartidas[i] = partidas.arrayPartidas[i + 1];
    }

    partidas.contador--;

    cout << "La partida " << indice + 1 << "  ha terminado. Se  ha elimina de la lista de partidas." << endl;
}

//----------------------------------------------------------
//INSERTAR NUEVA PARTIDA
//----------------------------------------------------------

bool insertarNuevaPartida(tListaPartidas& partidas, const tEstadoPartida& partidaOca) {

    bool insertada = false;

    if (partidas.contador < MAX_PARTIDAS) {
        partidas.arrayPartidas[partidas.contador] = partidaOca;
        partidas.contador++;
        insertada = true;
    }

    else {
        cout << "NO SE HA PODIDO INSERTAR LA PARTIDA NUEVA" << endl;
    }

    return insertada;
}

//----------------------------------------------------------
//GUARDAR PARTIDA, TABLERO Y JUGADORES
//----------------------------------------------------------

void guardarPartida(const tListaPartidas& partidas) {

    ofstream archivo;
    string nombreArchivo;
    bool abierto = false;

    cout << "Dame el nombre del archivo en el que quieras guardar la partidas: ";
    cin >> nombreArchivo;

    archivo.open(nombreArchivo);

    if (archivo.is_open()) {

        archivo << partidas.contador << endl;

        for (int i = 0; i < partidas.contador; i++) {
            guardaTablero(partidas.arrayPartidas[i].tablero, archivo);
            archivo << partidas.arrayPartidas[i].turno + 1 << endl;
            guardarJugadores(partidas.arrayPartidas[i].jugador, archivo);
        }

        cout << "La partida se ha guardado correctamente en el fichero " << nombreArchivo << endl;

        archivo.close();
        abierto = true;
    }

    else {
        cout << "EL ARCHIVO NO SE HA PODIDO ABRIR CORRECTAMENTE " << endl;
    }
}

//----------------------------------------------------------

void guardaTablero(const tTablero tablero, ofstream& archivo) {

    tCasilla casilla = NORMAL;
    string casillaNombre;

    for (int i = 0; i < NUN_CASILLAS; i++) {
        if (tablero[i] != NORMAL) {
            archivo << i + 1 << " ";
            archivo << casillaAstringGuardar(tablero[i]) << endl;
        }
    }

    archivo << "0" << endl;
}

//----------------------------------------------------------

void guardarJugadores(const tEstadoJugadores jugadores, ofstream& archivo) {

    for (int i = 0; i < NUM_JUGADORES; i++) {
        archivo << jugadores[i].casillas + 1 << " " << jugadores[i].penalizaciones << endl;
    }
}

//---------------------------------------------------------------------------
//PARTIDA
//---------------------------------------------------------------------------

int partida(tEstadoPartida& estado) {

    tListaPartidas partidaGuardar;

    bool finPartida = false;
    bool abandonarPartida = false;
    char opcionPartida;
    int ganador;

    pintaTablero(estado);

    cout << "Comienza el juego" << endl << endl;
    cout << "**** EMPIEZA EL JUGADOR " << estado.turno + 1 << " ****" << endl;

    cout << "CASILLA ACTUAL: " << estado.jugador[estado.turno].casillas + 1 << endl;

    while (!finPartida && !abandonarPartida) {

        if (estado.jugador[estado.turno].penalizaciones == 0) {
            tirada(estado.tablero, estado.jugador[estado.turno]);
        }

        else if (estado.jugador[estado.turno].penalizaciones > 0) {
            cout << "NO PUEDE JUGAR, LE QUEDAN AUN " << estado.jugador[estado.turno].penalizaciones << " TURNOS SIN JUGAR" << endl;
            estado.jugador[estado.turno].penalizaciones--;
        }

        pintaTablero(estado);

        if (!esMeta(estado.jugador[estado.turno].casillas) && !esCasillaPremio(estado.tablero, estado.jugador[estado.turno].casillas)) {

            //CAMBIO DE JUGADOR
            if (estado.turno < NUM_JUGADORES - 1) {
                estado.turno++;
            }

            else {
                estado.turno = 0;
            }

            cout << "DESEA ABANDONAR LA PARTIDA? (S/N): ";
            cin >> opcionPartida;
            
            if (opcionPartida == 's' || opcionPartida == 'S') {
                ganador = -1;
                abandonarPartida = true;
            }
            
            else {
                cout << "TURNO PARA EL JUGADOR: " << estado.turno + 1 << endl;
                cout << "CASILLA ACTUAL: " << estado.jugador[estado.turno].casillas + 1 << endl;
            }
        }

        else if (esMeta(estado.jugador[estado.turno].casillas)) {
            ganador = estado.turno;
            cout << endl << "**** FIN DEL JUEGO ****" << endl;
            finPartida = true;
        }
    }

    return ganador;
}

//---------------------------------------------------------------------------
//LOGICA DE TIRADAS
//---------------------------------------------------------------------------

void iniciaJugadores(tEstadoJugadores jugadores) {
   
    for (int i = 0; i < NUM_JUGADORES; i++) {
        jugadores[i].casillas = 0;
        jugadores[i].penalizaciones = 0;
    }
}

//---------------------------------------------------------------------------
//EFECTO TIRADA
//---------------------------------------------------------------------------

void efectoTirada(const tTablero tablero, tEstadoJugador& estadoJug) {

    if (tablero[estadoJug.casillas] != NORMAL) {

        switch (tablero[estadoJug.casillas]) {

        case OCA:
            estadoJug.casillas = saltaACasilla(tablero, estadoJug.casillas);
            cout << "DE OCA A OCA Y TIRO POR QUE ME TOCA" << endl;
            cout << "SALTAS A LA CASILLA: " << estadoJug.casillas + 1 << endl;
            if (!esMeta(estadoJug.casillas)) {
                cout << "Y VUELVES A TIRAR" << endl;
            }
            break;

        case DADO1:
            estadoJug.casillas = saltaACasilla(tablero, estadoJug.casillas);
            cout << "DE DADO A DADO Y TIRO POR QUE ME HA TOCADO" << endl;
            cout << "SALTAS A LA CASILLA: " << estadoJug.casillas + 1 << endl;
            if (!esMeta(estadoJug.casillas)) {
                cout << "Y VUELVES A TIRAR" << endl;
            }
            break;

        case DADO2:
            estadoJug.casillas = saltaACasilla(tablero, estadoJug.casillas);
            cout << "DE DADO A DADO Y TIRO POR QUE ME HA TOCADO" << endl;
            cout << "SALTAS A LA CASILLA: " << estadoJug.casillas + 1 << endl;
            if (!esMeta(estadoJug.casillas)) {
                cout << "Y VUELVES A TIRAR" << endl;
            }
            break;

        case PUENTE1:
            estadoJug.casillas = saltaACasilla(tablero, estadoJug.casillas);
            cout << "DE PUENTE A PUENTE Y TIRO POR QUE ME LLEVA LA CORRIENTE" << endl;
            cout << "SALTAS A LA CASILLA: " << estadoJug.casillas + 1 << endl;
            if (!esMeta(estadoJug.casillas)) {
                cout << "Y VUELVES A TIRAR" << endl;
            }
            break;

        case PUENTE2:
            estadoJug.casillas = saltaACasilla(tablero, estadoJug.casillas);
            cout << "DE PUENTE A PUENTE Y TIRO POR QUE ME LLEVA LA CORRIENTE" << endl;
            cout << "SALTAS A LA CASILLA: " << estadoJug.casillas + 1 << endl;
            if (!esMeta(estadoJug.casillas)) {
                cout << "Y VUELVES A TIRAR" << endl;
            }
            break;

        case LABERINTO:
            estadoJug.casillas = saltaACasilla(tablero, estadoJug.casillas);
            cout << "HAS CAIDO EN EL LABERINTO" << endl;
            cout << "SALTAS A LA CASILLA: " << estadoJug.casillas + 1 << endl;
            break;

        case POSADA:
            cout << "HAS CAIDO EN LA POSADA." << endl;
            cout << "PIERDES " << TURNOS_POSADA << " TURNOS" << endl;
            estadoJug.penalizaciones = TURNOS_POSADA;
            break;

        case POZO:
            cout << "HAS CAIDO EN EL POZO." << endl;
            cout << "PIERDES " << TURNOS_POZO << " TURNOS" << endl;
            estadoJug.penalizaciones = TURNOS_POZO;
            break;

        case CARCEL:
            cout << "HAS CAIDO EN LA CARCEL." << endl;
            cout << "PIERDES " << TURNOS_CARCEL << " TURNOS" << endl;
            estadoJug.penalizaciones = TURNOS_CARCEL;
            break;

        case CALAVERA:
            estadoJug.casillas = saltaACasilla(tablero, estadoJug.casillas);
            cout << "HAS CAIDO EN LA MUERTE." << endl;
            cout << "RETROCEDES A LA CASILLA " << estadoJug.casillas + 1 << endl;
            break;
        }
    }
}

//---------------------------------------------------------------------------
//TIRADA
//---------------------------------------------------------------------------

void tirada(const tTablero tablero, tEstadoJugador& estadoJug) {

    int dado;

    if (MODO_DEBUG == true) {
        dado = tirarDadoManual();
    }

    else {
        dado = tirarDado();
    }

    estadoJug.casillas = estadoJug.casillas + dado;

    cout << "PASAS A LA CASILLA " << estadoJug.casillas + 1 << endl;

    if (!esMeta(estadoJug.casillas)) {
        efectoTirada(tablero, estadoJug);
    }
}

//---------------------------------------------------------------------------
//VERSION 2
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
//CARGAR TABLERO
//---------------------------------------------------------------------------

void iniciaTablero(tTablero tablero) {

    for (int i = 0; i < NUN_CASILLAS; i++) {
        tablero[i] = NORMAL;
    }

    tablero[NUN_CASILLAS - 1] = OCA;
}

//---------------------------------------------------------------------------

string casillaAstring(tCasilla casilla) {

    string cadena;

    switch (casilla) {

    case NORMAL:
        cadena = " ";
        break;

    case OCA:
        cadena = "OCA";
        break;

    case DADO1:
    case DADO2:
        cadena = "DADO";
        break;

    case PUENTE1:
    case PUENTE2:
        cadena = "PNTE";
        break;

    case POSADA:
        cadena = "PSDA";
        break;

    case CALAVERA:
        cadena = "MUER";
        break;

    case LABERINTO:
        cadena = "LBRN";
        break;

    case POZO:
        cadena = "POZO";
        break;

    case CARCEL:
        cadena = "CRCL";
        break;
    }

    return cadena;
}

//---------------------------------------------------------------------------

string casillaAstringGuardar(tCasilla casilla) {

    string cadena;

    switch (casilla) {

    case OCA:
        cadena = "OCA";
        break;

    case DADO1:
        cadena = "DADO1";
        break;

    case DADO2:
        cadena = "DADO2";
        break;

    case PUENTE1:
        cadena = "PUENTE1";
        break;

    case PUENTE2:
        cadena = "PUENTE2";
        break;

    case POSADA:
        cadena = "POSADA";
        break;

    case CALAVERA:
        cadena = "CALAVERA";
        break;

    case LABERINTO:
        cadena = "LABERINTO";
        break;

    case POZO:
        cadena = "POZO";
        break;

    case CARCEL:
        cadena = "CARCEL";
        break;
    }

    return cadena;
}

//---------------------------------------------------------------------------

tCasilla stringACasilla(string nombre) {

    tCasilla casilla = NORMAL;

    if (nombre == "OCA") {
        casilla = OCA;
    }

    else if (nombre == "DADO1") {
        casilla = DADO1;
    }

    else if (nombre == "DADO2") {
        casilla = DADO2;
    }

    else if (nombre == "PUENTE1") {
        casilla = PUENTE1;
    }

    else if (nombre == "PUENTE2") {
        casilla = PUENTE2;
    }

    else if (nombre == "POSADA") {
        casilla = POSADA;
    }

    else if (nombre == "CALAVERA") {
        casilla = CALAVERA;
    }

    else if (nombre == "LABERINTO") {
        casilla = LABERINTO;
    }

    else if (nombre == "POZO") {
        casilla = POZO;
    }

    else if (nombre == "CARCEL") {
        casilla = CARCEL;
    }

    return casilla;
}

//---------------------------------------------------------------------------
//SALTA A CASILLA
//---------------------------------------------------------------------------

int saltaACasilla(const tTablero tablero, int casillaActual) {

    if (tablero[casillaActual] == OCA) {
        buscaCasillaAvanzando(tablero, OCA, casillaActual);
    }

    else if (tablero[casillaActual] == PUENTE1) {
        buscaCasillaAvanzando(tablero, PUENTE2, casillaActual);
    }

    else if (tablero[casillaActual] == DADO1) {
        buscaCasillaAvanzando(tablero, DADO2, casillaActual);
    }

    else if (tablero[casillaActual] == PUENTE2) {
        buscaCasillaRetrocediendo(tablero, PUENTE1, casillaActual);
    }

    else if (tablero[casillaActual] == DADO2) {
        buscaCasillaRetrocediendo(tablero, DADO1, casillaActual);
    }

    else if (tablero[casillaActual] == LABERINTO) {
        casillaActual = casillaActual - RETROCESO_LABERINTO;
    }

    else if (tablero[casillaActual] == CALAVERA) {
        casillaActual = CASILLA_INICIAL - 1;
    }

    return casillaActual;
}

//---------------------------------------------------------------------------

void buscaCasillaAvanzando(const tTablero tablero, tCasilla tipo, int& posicion) {

    bool encontrado = false;

    while (!encontrado && posicion < NUN_CASILLAS) {

        posicion++;

        if (tipo == tablero[posicion]) {
            posicion = posicion;
            encontrado = true;
        }
    }
}

//---------------------------------------------------------------------------

void buscaCasillaRetrocediendo(const tTablero tablero, tCasilla tipo, int& posicion) {

    bool encontrado = false;

    while (!encontrado && posicion >= 0) {

        posicion--;

        if (tipo == tablero[posicion]) {
            posicion = posicion;
            encontrado = true;
        }
    }
}

//---------------------------------------------------------------------------
//REPRESENTACION DEL TABLERO (V3)
//---------------------------------------------------------------------------

void pintaTablero(const tEstadoPartida& partida) {

    int casillasPorFila = NUN_CASILLAS / NUM_FILAS_A_DIBUJAR;

    cout << endl;

    for (int fila = 0; fila < NUM_FILAS_A_DIBUJAR; fila++) {
        pintaBorde(casillasPorFila);
        pintaNumCasilla(fila, casillasPorFila);
        pintaTipoCasilla(partida.tablero, fila, casillasPorFila);
        pintaJugadores(partida.jugador, fila, casillasPorFila);
    }

    pintaBorde(casillasPorFila);

    cout << endl;
}

//---------------------------------------------------------------------------

void pintaBorde(int casillasPorFila) {

    for (int i = 1; i <= casillasPorFila; i++) {
        cout << "|====";
    }

    cout << "|" << endl;

}

//---------------------------------------------------------------------------

void pintaNumCasilla(int fila, int casillasPorFila) {

    for (int i = 1; i <= casillasPorFila; i++) {
        cout << "| " << setw(2) << setfill('0') << i + fila * casillasPorFila << " ";
    }

    cout << "|" << endl;
}

//---------------------------------------------------------------------------

void pintaTipoCasilla(const tTablero tablero, int fila, int casillasPorFila) {

    for (int i = 1; i <= casillasPorFila; i++) {
        cout << "|" << setw(4) << setfill(' ') << casillaAstring(tablero[i - 1 + fila * casillasPorFila]);
    }

    cout << "|" << endl;

}

//---------------------------------------------------------------------------

void pintaJugadores(const tEstadoJugadores casillasJ, int fila, int casillasPorFila) {

    int casilla;

    int blancos = MAX_JUGADORES - NUM_JUGADORES;

    string b = "";

    for (int i = 0; i < blancos; i++) b = b + " ";

    cout << "|";

    for (int i = 1; i <= casillasPorFila; i++) {

        casilla = i - 1 + fila * casillasPorFila;

        for (int jug = 0; jug < NUM_JUGADORES; jug++) {

            if (casillasJ[jug].casillas == casilla)
                cout << jug + 1;

            else
                cout << " ";
        }

        cout << b;
        cout << "|";
    }

    cout << endl;
}

//---------------------------------------------------------------------------
//SUSTITUTO DE LA V1
//---------------------------------------------------------------------------

bool esCasillaPremio(const tTablero tablero, int casilla) {

    bool premio = false;

    if (tablero[casilla] == OCA || tablero[casilla] == PUENTE1 || tablero[casilla] == PUENTE2 || tablero[casilla] == DADO1 || tablero[casilla] == DADO2) {
        premio = true;
    }

    return premio;
}

//---------------------------------------------------------------------------
//VERSION 1
//---------------------------------------------------------------------------

bool esMeta(int casilla) {

    bool meta = false;

    if (casilla >= NUN_CASILLAS - 1) {
        meta = true;
    }

    return meta;
}

//---------------------------------------------------------------------------

int tirarDado() {

    int numeroDado;

    numeroDado = 1 + rand() % (7 - 1);

    cout << "VALOR DEL DADO: " << numeroDado << endl;

    return numeroDado;
}

//---------------------------------------------------------------------------

int tirarDadoManual() {

    int dado;

    cout << "INTRODUCE EL VALOR DEL DADO: ";
    cin >> dado;
    cout << "VALOR DEL DADO: " << dado << endl;
    
    return dado;
}

//---------------------------------------------------------------------------

int quienEmpieza() {

    int jugador;

    jugador = rand() % (NUM_JUGADORES);

    return jugador;
}
