# BDSP – Hill-Climbing Best Improvement + Restart

Vicente Luongo – ROL 202073637-5 – INF295 Inteligencia Artificial 2026-2

## Compilación

```
make          # genera el ejecutable ./bdsp
make clean    # elimina objetos, ejecutable y carpeta salidas/
```

Requiere g++ con soporte C++17 (Linux).

## Ejecución

```
./bdsp <instancia> [semilla] [tiempo_max_seg] [opciones]
```

| Argumento | Descripción | Por defecto |
|---|---|---|
| `instancia` | archivo en el formato del curso | — |
| `semilla` | semilla del generador aleatorio | 1 |
| `tiempo_max_seg` | tiempo total de búsqueda (todos los reinicios) | 60 |
| `--inicial` | solo genera y evalúa la solución inicial aleatoria | |
| `--sin-mover` / `--sin-cruce` | desactiva el vecindario N1 / N2 | |
| `--reinicios R` | número máximo de reinicios | sin límite |
| `--salida F` | archivo de salida | `salidas/<instancia>_<semilla>.out` |
| `-v` | progreso por reinicio y detalle de cada turno (stderr) | |

Ejemplo:

```
./bdsp instancias/V_TCCC0130.txt 7 30 -v
```

## Salida

Se escribe en stdout y en el archivo de salida:

```
<Valor Func. Objetivo> <Tiempo Ejecución [s]> <Semilla> <Número de iteraciones>

<ID Tramo> <ID Conductor>
...
```

Los IDs de los tramos siguen el orden de la entrada (desde 1) y los de los conductores van desde 1. El número de iteraciones es la cantidad total de movimientos aplicados, sumando todos los reinicios.

## Estructura

| Archivo | Contenido |
|---|---|
| `src/instancia.*` | lectura de la instancia, conversión a minutos y minutos nocturnos |
| `src/evaluacion.*` | función de evaluación de un turno (tiempo pagado y penalizaciones) |
| `src/solucion.*` | representación, solución inicial aleatoria y escritura de la salida |
| `src/hillclimbing.*` | HC-BI con los vecindarios Mover y Cruce, y los reinicios |
| `src/main.cpp` | argumentos, medición del tiempo y flujo principal |
| `instancias/` | instancias del curso y el ejemplo de 6 tramos de las diapositivas (`ejemplo_curso.txt`) |

## Uso de LLMs

Para esta entrega utilicé **Claude Code** (Anthropic, modelo Claude Opus 5.5) como
asistente de programación. El trabajo se organizó de la siguiente manera:

**Aportado por mí (autor):**
- La especificación del problema: interpretación de las reglas del proyecto y
  del modelo matemático del Informe 1 (función objetivo, restricciones,
  definición de los vecindarios N1 y N2).
- Las decisiones de diseño: representación de la solución, estrategia de
  reinicios, criterio de término por tiempo, estructura de archivos y formato
  de salida.
- La definición de los casos de prueba y la validación de los resultados
  (ver más abajo).
- La revisión del código generado y la corrección de los errores detectados.

**Aportado por el asistente:**
- La redacción del código de `src/`, el `Makefile` y este `README`, a partir de
  la especificación anterior y de iteraciones sucesivas de revisión.

**Verificación realizada por mí:**
- Ejecución del ejemplo de 6 tramos de las diapositivas, contrastando el
  resultado con el valor esperado (FO = 864).
- Ejecución sobre las 10 instancias del curso, revisando la factibilidad de las
  soluciones y la coherencia de los valores de FO entre semillas.
- Verificación manual del cálculo del pago de un turno con trabajo nocturno,
  comparando la fórmula del informe con los valores que reporta el programa
  (recargos, tramos nocturnos y penalizaciones).

**Nivel modificación:**
Revisé el código completo y ajusté las partes que dependen de decisiones de modelado, no solo de sintaxis. En concreto:

- Fijé la relación entre PESO_VIOLACION y el pago mínimo de un turno. Determiné que el peso debía superar 432 (el pago mínimo de un turno) para que el algoritmo nunca prefiera ahorrar un conductor a costa de aceptar una violación; el valor 1000 y el comentario que lo justifica en src/evaluacion.h son decisiones mías.

- Definí los flags de línea de comandos (--inicial, --sin-mover, --sin-cruce, --reinicios, -v) para poder hacer los experimentos de ablación (N1 solo, N2 solo, ambos, HC puro vs. con reinicios) sin recompilar entre corridas.

- Amplié el modo -v para que imprima, al final, el desglose por turno (horario, spread, descanso, sobretiempo, minutos nocturnos, pago y violaciones). Es lo que uso para explicar el comportamiento del algoritmo en la presentación y para depurar.

- Verifiqué y corregí el cálculo del pago con trabajo nocturno y el criterio de término por tiempo, contrastando a mano contra la fórmula del Informe 1.

- Definí el protocolo experimental: correr cada instancia con 10 semillas y reportar promedio y mejor valor, en lugar de una sola corrida.

- La documentación de parámetros y constantes que acompaña al código la redacté a partir de lo que fui comprobando al ejecutar el programa con distintas semillas, tiempos y combinaciones de vecindarios.