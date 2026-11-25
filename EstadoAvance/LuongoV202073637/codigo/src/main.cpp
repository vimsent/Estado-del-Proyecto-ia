// main.cpp
// Uso:
//   ./bdsp <instancia> [semilla] [tiempo_max_seg] [opciones]
// Opciones:
//   --inicial      solo genera y evalua la solucion inicial (sin HC)
//   --sin-mover    desactiva el vecindario N1
//   --sin-cruce    desactiva el vecindario N2
//   --reinicios R  limite de reinicios
//   --salida F     archivo de salida (por defecto salidas/<instancia>_<semilla>.out)
//   -v             muestra progreso y detalle de los turnos por stderr
#include <chrono>
#include <cstring>
#include <fstream>
#include <iostream>
#include <random>
#include <sys/stat.h>

#include "hillclimbing.h"
#include "instancia.h"
#include "solucion.h"

using namespace std;

static double reloj() {
    using namespace chrono;
    static const auto t0 = steady_clock::now();
    return duration<double>(steady_clock::now() - t0).count();
}

int main(int argc, char** argv) {
    if (argc < 2) {
        cerr << "Uso: " << argv[0]
             << " <instancia> [semilla] [tiempo_max_seg] [--inicial] [--sin-mover] "
                "[--sin-cruce] [--reinicios R] [--salida F] [-v]\n";
        return 1;
    }
    reloj();
    string ruta = argv[1];
    unsigned semilla = 1;
    ConfigHC cfg;
    bool soloInicial = false;
    string salida;

    int posicional = 0;
    for (int a = 2; a < argc; ++a) {
        if (!strcmp(argv[a], "--inicial")) soloInicial = true;
        else if (!strcmp(argv[a], "--sin-mover")) cfg.usarMover = false;
        else if (!strcmp(argv[a], "--sin-cruce")) cfg.usarCruce = false;
        else if (!strcmp(argv[a], "--reinicios") && a + 1 < argc) cfg.maxReinicios = atoi(argv[++a]);
        else if (!strcmp(argv[a], "--salida") && a + 1 < argc) salida = argv[++a];
        else if (!strcmp(argv[a], "-v")) cfg.verbose = true;
        else if (posicional == 0) { semilla = (unsigned)stoul(argv[a]); ++posicional; }
        else if (posicional == 1) { cfg.tiempoMax = stod(argv[a]); ++posicional; }
        else { cerr << "Argumento desconocido: " << argv[a] << "\n"; return 1; }
    }

    Instancia I;
    if (!I.leer(ruta)) return 1;
    cerr << "Instancia " << I.nombre << ": " << I.n() << " tramos, " << I.lugares.size()
         << " lugares de relevo\n";

    mt19937 rng(semilla);
    Solucion sol;
    long long iteraciones = 0;

    if (soloInicial) {
        sol = solucionInicialAleatoria(I, rng);
    } else {
        ResultadoHC r = hcbiConReinicios(I, rng, cfg, reloj);
        sol = r.mejor;
        iteraciones = r.iteraciones;
        cerr << "Reinicios: " << r.reinicios << ", movimientos: " << r.iteraciones
             << ", mejor encontrada en t=" << r.tiempoMejor << "s\n";
    }
    double tiempo = reloj();

    cerr << "Conductores: " << sol.turnos.size() << ", FO: " << sol.objetivo(I)
         << (sol.esFactible() ? " (factible)" : " (INFACTIBLE)") << "\n";
    if (cfg.verbose) escribirDetalle(cerr, I, sol);

    if (salida.empty()) {
        mkdir("salidas", 0755);
        string base = I.nombre.substr(0, I.nombre.find_last_of('.'));
        salida = "salidas/" + base + "_" + to_string(semilla) + ".out";
    }
    ofstream out(salida);
    escribirSalida(out, I, sol, tiempo, semilla, iteraciones);
    escribirSalida(cout, I, sol, tiempo, semilla, iteraciones);
    cerr << "Salida escrita en " << salida << "\n";
    return 0;
}
