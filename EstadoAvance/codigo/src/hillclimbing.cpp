// hillclimbing.cpp
//
// Vecindarios (ambos se exploran completos en cada iteracion: Best Improvement):
//
//  N1 "Mover":  el tramo j sale del turno A y se inserta en el turno B (B puede
//               ser un turno nuevo vacio). Basta este movimiento para recorrer
//               todo el espacio de busqueda: desde cualquier asignacion se puede
//               llegar a cualquier otra moviendo tramo por tramo (p. ej. pasando
//               por la solucion de un conductor por tramo).
//
//  N2 "Cruce":  intercambio de sufijos (2-opt*) entre dos turnos A y B: se corta
//               A antes de la posicion i y B antes de la posicion k, y se
//               intercambian las colas: A' = A[0..i) + B[k..], B' = B[0..k) + A[i..].
//               Si B[0..k) queda vacio el turno B desaparece, lo que permite
//               fusionar turnos cortos (clave por el tiempo pagado minimo).
//
// Evaluacion incremental: un movimiento solo cambia dos turnos, por lo que
//   delta = costo(A') + costo(B') - costo(A) - costo(B)
// y el costo de cada turno esta cacheado en Solucion::eval.
//
// Restricciones: la cobertura (cada tramo en exactamente un turno) la garantiza la
// representacion; el no traslape temporal es duro (los movimientos que lo violan
// no se generan); las reglas laborales y la continuidad espacial son blandas y se
// penalizan en la funcion de evaluacion.
#include "hillclimbing.h"

#include <algorithm>
#include <iostream>

using namespace std;

namespace {

const double EPS = 1e-9;

enum TipoMov { NINGUNO, MOVER, CRUCE };

struct Movimiento {
    TipoMov tipo = NINGUNO;
    double delta = -EPS;  // solo se aceptan mejoras estrictas
    int a = -1, b = -1;   // turnos involucrados (b == -1: turno nuevo)
    int i = -1, k = -1;   // MOVER: i = posicion de j en A, k = posicion en B
                          // CRUCE: puntos de corte en A y B
    EvalTurno evA, evB;
};

// N1: mover un tramo de un turno a otro.
void explorarMover(const Instancia& I, const Solucion& s, Movimiento& mejor) {
    const int m = (int)s.turnos.size();
    vector<int> sinJ, conJ;
    for (int a = 0; a < m; ++a) {
        const auto& A = s.turnos[a];
        for (int i = 0; i < (int)A.size(); ++i) {
            int j = A[i];
            sinJ = A;
            sinJ.erase(sinJ.begin() + i);
            EvalTurno evA = evaluarTurno(I, sinJ);  // costo 0 si queda vacio
            double baseA = evA.costo - s.eval[a].costo;

            // Turno nuevo (solo tiene sentido si A no queda vacio).
            if (A.size() > 1) {
                EvalTurno evB = evaluarTurno(I, {j});
                double d = baseA + evB.costo;
                if (d < mejor.delta) mejor = {MOVER, d, a, -1, i, 0, evA, evB};
            }
            for (int b = 0; b < m; ++b) {
                if (b == a) continue;
                int pos;
                if (!cabeSinTraslape(I, s.turnos[b], j, pos)) continue;
                conJ = s.turnos[b];
                conJ.insert(conJ.begin() + pos, j);
                EvalTurno evB = evaluarTurno(I, conJ);
                double d = baseA + evB.costo - s.eval[b].costo;
                if (d < mejor.delta) mejor = {MOVER, d, a, b, i, pos, evA, evB};
            }
        }
    }
}

// N2: intercambio de sufijos entre dos turnos.
void explorarCruce(const Instancia& I, const Solucion& s, Movimiento& mejor) {
    const auto& T = I.tramos;
    const int m = (int)s.turnos.size();
    vector<int> nA, nB;
    for (int a = 0; a < m; ++a) {
        const auto& A = s.turnos[a];
        for (int b = 0; b < m; ++b) {
            if (b == a) continue;
            const auto& B = s.turnos[b];
            // Cortes i = 1..|A| (A conserva al menos un tramo). Para cada i, el
            // corte en B es el primer tramo que comienza despues de A[i-1]; asi
            // A' no tiene traslape por construccion.
            int k = 0;
            for (int i = 1; i <= (int)A.size(); ++i) {
                int finCabezaA = T[A[i - 1]].fin;
                while (k < (int)B.size() && T[B[k]].inicio < finCabezaA) ++k;
                if (i == (int)A.size() && k == (int)B.size()) continue;  // sin cambio
                // B' = B[0..k) + A[i..] no debe traslaparse.
                if (k > 0 && i < (int)A.size() && T[B[k - 1]].fin > T[A[i]].inicio) continue;

                nA.assign(A.begin(), A.begin() + i);
                nA.insert(nA.end(), B.begin() + k, B.end());
                nB.assign(B.begin(), B.begin() + k);
                nB.insert(nB.end(), A.begin() + i, A.end());
                EvalTurno evA = evaluarTurno(I, nA);
                EvalTurno evB = evaluarTurno(I, nB);
                double d = evA.costo + evB.costo - s.eval[a].costo - s.eval[b].costo;
                if (d < mejor.delta) mejor = {CRUCE, d, a, b, i, k, evA, evB};
            }
        }
    }
}

void aplicar(Solucion& s, const Movimiento& mv) {
    auto& A = s.turnos[mv.a];
    if (mv.tipo == MOVER) {
        int j = A[mv.i];
        A.erase(A.begin() + mv.i);
        if (mv.b == -1) {
            s.turnos.push_back({j});
            s.eval.push_back(mv.evB);
        } else {
            auto& B = s.turnos[mv.b];
            B.insert(B.begin() + mv.k, j);
            s.eval[mv.b] = mv.evB;
        }
        s.eval[mv.a] = mv.evA;
    } else {
        auto& B = s.turnos[mv.b];
        vector<int> nA(A.begin(), A.begin() + mv.i);
        nA.insert(nA.end(), B.begin() + mv.k, B.end());
        vector<int> nB(B.begin(), B.begin() + mv.k);
        nB.insert(nB.end(), A.begin() + mv.i, A.end());
        A = std::move(nA);
        B = std::move(nB);
        s.eval[mv.a] = mv.evA;
        s.eval[mv.b] = mv.evB;
    }
    s.costo += mv.delta;
    s.compactar();
}

}  // namespace

long long hillClimbingBI(const Instancia& I, Solucion& s, const ConfigHC& cfg, double deadline,
                         double (*reloj)()) {
    long long movs = 0;
    while (reloj() < deadline) {
        Movimiento mejor;
        if (cfg.usarMover) explorarMover(I, s, mejor);
        if (cfg.usarCruce) explorarCruce(I, s, mejor);
        if (mejor.tipo == NINGUNO) break;  // optimo local
        aplicar(s, mejor);
        ++movs;
    }
    return movs;
}

ResultadoHC hcbiConReinicios(const Instancia& I, mt19937& rng, const ConfigHC& cfg,
                             double (*reloj)()) {
    ResultadoHC res;
    double inicio = reloj();
    double deadline = inicio + cfg.tiempoMax;
    bool hayMejor = false;

    while (res.reinicios < cfg.maxReinicios && reloj() < deadline) {
        Solucion s = solucionInicialAleatoria(I, rng);
        double costoInicial = s.costo;
        res.iteraciones += hillClimbingBI(I, s, cfg, deadline, reloj);
        res.reinicios++;
        s.evaluarCompleta(I);  // elimina posible deriva numerica del delta
        if (cfg.verbose)
            cerr << "Reinicio " << res.reinicios << ": inicial=" << costoInicial
                 << " final=" << s.costo << " conductores=" << s.turnos.size()
                 << " t=" << reloj() - inicio << "s\n";
        if (!hayMejor || s.costo < res.mejor.costo - EPS) {
            res.mejor = s;
            res.tiempoMejor = reloj() - inicio;
            hayMejor = true;
        }
    }
    return res;
}
