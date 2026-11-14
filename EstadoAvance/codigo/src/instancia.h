// instancia.h
// Estructuras de datos de una instancia del BDSP y su lectura desde archivo.
#pragma once
#include <string>
#include <vector>

// Un tramo: unidad indivisible de trabajo entre dos puntos de relevo.
// Todos los tiempos se guardan en minutos desde las 00:00 del dia
// (las horas >= 24 representan tramos posteriores a la medianoche).
struct Tramo {
    std::string linea;  // ID de linea (sin espacios finales)
    int lineaId;        // linea mapeada a entero (comparaciones rapidas)
    int origen;         // punto de partida (mapeado a entero)
    int destino;        // punto de termino (mapeado a entero)
    int inicio;         // s_j
    int fin;            // e_j = s_j + d_j
    int duracion;       // d_j
    int bus;            // ID de bus
    int nocturno;       // nu_j: minutos de conduccion entre 22:00 y 05:00
};

// Parametros de la variante (Seccion 2.3 del informe), en el orden del archivo.
struct Parametros {
    int maxStretch;          // T^max: maximo trabajo continuo sin descanso (360)
    int descansoMin;         // R^min: duracion minima del descanso (90)
    int umbralDescanso;      // R^pag: sobre este umbral el descanso se paga (300)
    int pagoMinimo;          // P^min: tiempo pagado minimo por turno (432)
    int maxSobretiempo;      // O^max: maximo sobretiempo (120)
    int maxTurno;            // S^max: duracion maxima del turno / spreadover (780)
    double factorNocturno;   // f_N: correccion nocturna (60/52)
    double bonoNocturno;     // beta_N: recargo nocturno (0.20)
    double bonoSobretiempo;  // beta_O: recargo por sobretiempo (0.50)
    double penCambioLinea;   // p_L: penalizacion por cambio de linea (1)
};

struct Instancia {
    std::string nombre;
    std::vector<Tramo> tramos;      // en el orden del archivo (ID = indice + 1)
    std::vector<std::string> lugares;
    Parametros p;

    int n() const { return (int)tramos.size(); }
    // Lee el archivo; devuelve false (con mensaje en stderr) si hay error de formato.
    bool leer(const std::string& ruta);
};

// Minutos del intervalo [s, e) que caen en horario nocturno (22:00-05:00),
// considerando tambien los dias siguientes (horas >= 24).
int minutosNocturnos(int s, int e);
