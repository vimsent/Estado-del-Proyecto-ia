// instancia.cpp
// Lectura del formato de entrada del curso:
//   <n>
//   "linea","partida","termino",hora,minuto,duracion,bus   (n lineas)
//   T^max, R^min, R^pag, P^min, O^max, S^max, f_N (a/b), beta_N(%), beta_O(%), p_L
#include "instancia.h"

#include <algorithm>
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>

using namespace std;

int minutosNocturnos(int s, int e) {
    int total = 0;
    // Franja nocturna del dia d: [1440d - 120, 1440d + 300)  (22:00 a 05:00)
    for (int d = 0; d <= 3; ++d) {
        int a = max(s, 1440 * d - 120);
        int b = min(e, 1440 * d + 300);
        if (b > a) total += b - a;
    }
    return total;
}

// Elimina espacios, comillas y '\r' de los extremos.
static string limpiar(const string& t) {
    size_t a = t.find_first_not_of(" \t\r\"");
    if (a == string::npos) return "";
    size_t b = t.find_last_not_of(" \t\r\"");
    return t.substr(a, b - a + 1);
}

// Lee la siguiente linea no vacia del archivo.
static bool siguienteLinea(ifstream& in, string& linea) {
    while (getline(in, linea))
        if (!limpiar(linea).empty()) return true;
    return false;
}

static bool leerEntero(ifstream& in, int& x) {
    string l;
    if (!siguienteLinea(in, l)) return false;
    x = stoi(limpiar(l));
    return true;
}

// Acepta "60/52" o un decimal.
static bool leerReal(ifstream& in, double& x) {
    string l;
    if (!siguienteLinea(in, l)) return false;
    l = limpiar(l);
    size_t barra = l.find('/');
    if (barra != string::npos)
        x = stod(l.substr(0, barra)) / stod(l.substr(barra + 1));
    else
        x = stod(l);
    return true;
}

bool Instancia::leer(const string& ruta) {
    ifstream in(ruta);
    if (!in) {
        cerr << "No se pudo abrir " << ruta << "\n";
        return false;
    }
    size_t barra = ruta.find_last_of('/');
    nombre = ruta.substr(barra == string::npos ? 0 : barra + 1);

    try {
        int n;
        if (!leerEntero(in, n) || n <= 0) throw runtime_error("numero de tramos");

        map<string, int> idLugar, idLinea;
        auto mapear = [](map<string, int>& m, const string& k) {
            auto it = m.find(k);
            if (it != m.end()) return it->second;
            int id = (int)m.size();
            m[k] = id;
            return id;
        };

        tramos.resize(n);
        for (int j = 0; j < n; ++j) {
            string l;
            if (!siguienteLinea(in, l)) throw runtime_error("faltan tramos");
            vector<string> campos;
            string c;
            stringstream ss(l);
            while (getline(ss, c, ',')) campos.push_back(limpiar(c));
            if (campos.size() != 7) throw runtime_error("tramo " + to_string(j + 1) + " mal formado");

            Tramo& t = tramos[j];
            t.linea = campos[0];
            t.lineaId = mapear(idLinea, t.linea);
            t.origen = mapear(idLugar, campos[1]);
            t.destino = mapear(idLugar, campos[2]);
            t.inicio = stoi(campos[3]) * 60 + stoi(campos[4]);
            t.duracion = stoi(campos[5]);
            t.fin = t.inicio + t.duracion;
            t.bus = stoi(campos[6]);
            t.nocturno = minutosNocturnos(t.inicio, t.fin);
        }

        lugares.assign(idLugar.size(), "");
        for (auto& [k, v] : idLugar) lugares[v] = k;

        double bN, bO;
        if (!leerEntero(in, p.maxStretch) || !leerEntero(in, p.descansoMin) ||
            !leerEntero(in, p.umbralDescanso) || !leerEntero(in, p.pagoMinimo) ||
            !leerEntero(in, p.maxSobretiempo) || !leerEntero(in, p.maxTurno) ||
            !leerReal(in, p.factorNocturno) || !leerReal(in, bN) || !leerReal(in, bO) ||
            !leerReal(in, p.penCambioLinea))
            throw runtime_error("faltan parametros");
        p.bonoNocturno = bN / 100.0;
        p.bonoSobretiempo = bO / 100.0;
    } catch (const exception& e) {
        cerr << "Error de formato en " << ruta << ": " << e.what() << "\n";
        return false;
    }
    return true;
}
