// solucion.cpp
#include "solucion.h"

#include <algorithm>
#include <iomanip>
#include <numeric>

using namespace std;

void Solucion::evaluarCompleta(const Instancia& I) {
    eval.resize(turnos.size());
    costo = 0;
    for (size_t k = 0; k < turnos.size(); ++k) {
        eval[k] = evaluarTurno(I, turnos[k]);
        costo += eval[k].costo;
    }
}

void Solucion::compactar() {
    size_t w = 0;
    for (size_t k = 0; k < turnos.size(); ++k) {
        if (turnos[k].empty()) continue;
        if (w != k) {
            turnos[w] = std::move(turnos[k]);
            eval[w] = eval[k];
        }
        ++w;
    }
    turnos.resize(w);
    eval.resize(w);
}

double Solucion::violacionTotal() const {
    double v = 0;
    for (auto& e : eval) v += e.violacion;
    return v;
}

double Solucion::objetivo(const Instancia& I) const {
    double f = 0;
    for (auto& e : eval) f += e.pagado + I.p.penCambioLinea * e.cambiosLinea;
    return f;
}

vector<int> Solucion::asignacion(const Instancia& I) const {
    vector<int> a(I.n(), 0);
    for (size_t k = 0; k < turnos.size(); ++k)
        for (int j : turnos[k]) a[j] = (int)k + 1;
    return a;
}

Solucion solucionInicialAleatoria(const Instancia& I, mt19937& rng) {
    Solucion s;
    vector<int> orden(I.n());
    iota(orden.begin(), orden.end(), 0);
    shuffle(orden.begin(), orden.end(), rng);

    vector<int> candidatos, posiciones;
    for (int j : orden) {
        candidatos.clear();
        posiciones.clear();
        for (size_t k = 0; k < s.turnos.size(); ++k) {
            int pos;
            if (!cabeSinTraslape(I, s.turnos[k], j, pos)) continue;
            vector<int> nuevo = s.turnos[k];
            nuevo.insert(nuevo.begin() + pos, j);
            if (evaluarTurno(I, nuevo).violacion == 0) {
                candidatos.push_back((int)k);
                posiciones.push_back(pos);
            }
        }
        if (candidatos.empty()) {
            s.turnos.push_back({j});
        } else {
            int c = uniform_int_distribution<int>(0, (int)candidatos.size() - 1)(rng);
            auto& t = s.turnos[candidatos[c]];
            t.insert(t.begin() + posiciones[c], j);
        }
    }
    s.evaluarCompleta(I);
    return s;
}

void escribirSalida(ostream& out, const Instancia& I, const Solucion& s, double tiempo,
                    unsigned semilla, long long iteraciones) {
    out << fixed << setprecision(2) << s.objetivo(I) << " " << setprecision(3) << tiempo << " "
        << semilla << " " << iteraciones << "\n\n";
    vector<int> a = s.asignacion(I);
    for (int j = 0; j < I.n(); ++j) out << j + 1 << " " << a[j] << "\n";
}

static string hhmm(int m) {
    char b[8];
    snprintf(b, sizeof b, "%02d:%02d", m / 60, m % 60);
    return b;
}

void escribirDetalle(ostream& out, const Instancia& I, const Solucion& s) {
    out << fixed << setprecision(1);
    for (size_t k = 0; k < s.turnos.size(); ++k) {
        const auto& t = s.turnos[k];
        const auto& e = s.eval[k];
        out << "Conductor " << k + 1 << ": " << hhmm(I.tramos[t.front()].inicio) << "-"
            << hhmm(I.tramos[t.back()].fin) << " tramos=" << t.size() << " spread=" << e.spread
            << " descanso=" << e.descanso << " trabajado=" << e.trabajado
            << " sobretiempo=" << e.sobretiempo << " nocturno=" << e.nocturno
            << " cambiosLinea=" << e.cambiosLinea << " pagado=" << e.pagado;
        if (e.violacion > 0) out << "  VIOLACION=" << e.violacion;
        out << "\n    [";
        for (size_t a = 0; a < t.size(); ++a) out << (a ? " " : "") << t[a] + 1;
        out << "]\n";
    }
}
