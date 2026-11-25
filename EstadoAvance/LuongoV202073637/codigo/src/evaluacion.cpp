// evaluacion.cpp
// Implementa la funcion objetivo del Modelo 2 del informe, evaluada turno a turno:
//   pagado_k = max(W_k, P^min) + beta_O * O_k + beta_N * f_N * nu_k
//   W_k      = spread_k - min(descanso_k, R^pag) + (f_N - 1) * nu_k
//   O_k      = max(0, W_k - P^min)
// mas p_L por cada cambio de linea. Las reglas laborales se manejan como
// restricciones blandas (penalizadas) y el no traslape como restriccion dura.
#include "evaluacion.h"

#include <algorithm>
#include <climits>

using namespace std;

EvalTurno evaluarTurno(const Instancia& I, const vector<int>& turno) {
    EvalTurno r;
    if (turno.empty()) return r;
    const Parametros& P = I.p;
    const auto& T = I.tramos;
    const int L = (int)turno.size();

    const int S = T[turno.front()].inicio;
    const int E = T[turno.back()].fin;
    r.spread = E - S;

    for (int a = 0; a < L; ++a) {
        r.nocturno += T[turno[a]].nocturno;
        if (a > 0) {
            const Tramo& prev = T[turno[a - 1]];
            const Tramo& cur = T[turno[a]];
            if (prev.lineaId != cur.lineaId) r.cambiosLinea += 1;
            if (prev.destino != cur.origen) r.saltos++;  // continuidad espacial
        }
    }
    r.violacion += (double)r.saltos * MINUTOS_POR_SALTO;

    // Regla 6: duracion maxima del turno (spreadover).
    if (r.spread > P.maxTurno) r.violacion += r.spread - P.maxTurno;

    // Regla 1-2: si el turno dura mas que T^max necesita un descanso >= R^min
    // que deje ambos stretches <= T^max. Entre las pausas validas se elige la
    // que maximiza la parte no pagada (minimiza el costo). Si ninguna es valida,
    // se penaliza la que viola menos.
    if (r.spread > P.maxStretch) {
        int mejorDescanso = -1;
        double menorViolacion = r.spread - P.maxStretch;  // caso sin pausas
        for (int a = 0; a + 1 < L; ++a) {
            int fin_i = T[turno[a]].fin, ini_j = T[turno[a + 1]].inicio;
            int g = ini_j - fin_i;
            double v = max(0, P.descansoMin - g) + max(0, (fin_i - S) - P.maxStretch) +
                       max(0, (E - ini_j) - P.maxStretch);
            if (v == 0) {
                mejorDescanso = max(mejorDescanso, min(g, P.umbralDescanso));
            } else {
                menorViolacion = min(menorViolacion, v);
            }
        }
        if (mejorDescanso >= 0)
            r.descanso = mejorDescanso;
        else
            r.violacion += menorViolacion;
    }

    // Tiempo trabajado, sobretiempo y pago (reglas 3, 4 y 5).
    r.trabajado = r.spread - r.descanso + (P.factorNocturno - 1.0) * r.nocturno;
    r.sobretiempo = max(0.0, r.trabajado - P.pagoMinimo);
    if (r.sobretiempo > P.maxSobretiempo) r.violacion += r.sobretiempo - P.maxSobretiempo;

    r.pagado = max(r.trabajado, (double)P.pagoMinimo) + P.bonoSobretiempo * r.sobretiempo +
               P.bonoNocturno * P.factorNocturno * r.nocturno;

    r.costo = r.pagado + P.penCambioLinea * r.cambiosLinea + PESO_VIOLACION * r.violacion;
    return r;
}

bool cabeSinTraslape(const Instancia& I, const vector<int>& turno, int j, int& pos) {
    const auto& T = I.tramos;
    // Busqueda binaria de la posicion por hora de inicio.
    pos = (int)(lower_bound(turno.begin(), turno.end(), j,
                            [&](int a, int b) { return T[a].inicio < T[b].inicio; }) -
                turno.begin());
    if (pos > 0 && T[turno[pos - 1]].fin > T[j].inicio) return false;
    if (pos < (int)turno.size() && T[j].fin > T[turno[pos]].inicio) return false;
    return true;
}
