# VisualAutomata

VisualAutomata es una aplicación de escritorio en C++17 y Qt para crear y
simular autómatas finitos deterministas (AFD/DFA) y no deterministas
(AFN/NFA). Incluye el modelo, la simulación, un editor gráfico, importación y
exportación de archivos y pruebas automatizadas.

## Funcionalidades

- Crear AFD o AFN, agregar y mover estados, renombrarlos, marcar el inicial y
  alternar estados finales.
- Agregar y eliminar transiciones; las transiciones se pueden eliminar desde
  la barra o seleccionándolas y pulsando `Delete`. Para cambiar una transición,
  elimínala y créala de nuevo.
- Simular una cadena completa, avanzar paso a paso o probar varias cadenas en
  paralelo.
- Abrir y guardar autómatas en JSON y JFLAP (`.jff`).
- Avisar antes de descartar cambios sin guardar.

Un autómata nuevo comienza vacío. El primer estado que agregues queda marcado
como inicial. Los símbolos de transición son caracteres de un byte; deja el
campo vacío para crear una transición epsilon en un AFN.

## Estructura

```text
src/
  adapters/     Importadores y exportadores JSON/JFLAP
  controller/   Factory y punto de entrada de la aplicación
  model/        Estados, transiciones, AFD y AFN
  simulation/   Estrategias y ejecución de pruebas por lotes
  view/         Editor Qt, escena, panel de simulación y ventana principal
tests/          Siete suites Qt Test registradas con CTest
```

## Requisitos

- CMake 3.16 o posterior.
- Compilador compatible con C++17.
- Qt 6 o Qt 5 con los módulos Core, Widgets, Gui, Xml y Test.
- Qt Creator es opcional; se puede compilar desde la terminal.

## Instalación y ejecución con Qt Creator

1. Instala Qt y Qt Creator. En el instalador de Qt selecciona un kit de escritorio
   compatible con tu compilador (por ejemplo, **Qt 6.x MinGW 64-bit**) y los
   módulos Core, Widgets, Gui, Xml y Test.
2. Abre Qt Creator y selecciona **File → Open File or Project…**.
3. Abre el archivo `CMakeLists.txt` de la carpeta del repositorio y configura el
   proyecto con el kit de Qt instalado.
4. Selecciona el objetivo **VisualAutomata**, compila con `Ctrl+B` y ejecútalo
   con el botón verde **Run** o `Ctrl+R`.

La ventana de VisualAutomata se abre aparte de Qt Creator. En la aplicación:

1. Selecciona **Agregar estado** y haz clic en el lienzo para crear estados.
2. Selecciona un estado para marcarlo inicial, alternar si es final o
   renombrarlo. Arrástralo para moverlo.
3. Selecciona **Agregar transición**, haz clic en el origen y luego en el
   destino; escribe el símbolo cuando aparezca el diálogo.
4. Escribe una cadena en el panel derecho y pulsa **Ejecutar** o **Paso a paso**.
5. Usa el menú **Archivo** para abrir, guardar o exportar el autómata.

## Compilación y pruebas desde la terminal

Desde la raíz del repositorio:

```powershell
cmake -S . -B build
cmake --build build --parallel
ctest --test-dir build --output-on-failure
```

En Windows, si CTest informa que faltan DLL (`0xc0000135`), añade a `PATH` las
carpetas `bin` de Qt y del compilador MinGW que correspondan a tu instalación.
Por ejemplo, para Qt instalado en `D:\QT`:

```powershell
$env:PATH = 'D:\QT\6.11.2\mingw_64\bin;D:\QT\Tools\mingw1310_64\bin;' + $env:PATH
ctest --test-dir build --output-on-failure
```

Los tests cubren modelo, AFD, AFN, simulación, adapters, factory y carga/
eliminación de transiciones en la escena gráfica. El test de vista usa la
plataforma Qt `offscreen`, por lo que no requiere una pantalla.
