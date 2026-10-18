# Sistema de Parqueadero

Simulación de un parqueadero de 30 celdas con tres componentes que se comunican entre sí:

- Un **generador** (C++) envía placas por red cada 2 a 5 segundos.
- Un **receptor** (C++, librería dinámica) las recibe por socket.
- Un **visualizador** (Python) usa esa librería y muestra el estado del parqueadero.

## Páginas

| Página | Contenido |
|---|---|
| [Arquitectura](Arquitectura.md) | Cómo se conectan los componentes y en qué PC corre cada uno |
| [Protocolo](Protocolo.md) | Formato de los mensajes y reglas de entrada y salida |
| [Receptor DLL](Receptor-DLL.md) | Servidor de sockets, hilo, cola y funciones exportadas |
| [Generador](Generador.md) | Clases del generador y su ciclo de envío |
| [Visualizador](Visualizador.md) | Interfaz en Python y uso de la DLL con ctypes |
| [Pruebas](Pruebas.md) | Cómo se probó cada parte y el sistema completo |

Las instrucciones para compilar y ejecutar están en el [README](../README.md).

## Equipo

| Integrante | Componente |
|---|---|
| Cristopher Corrales | Receptor (DLL) |
| Johan López | Generador de placas |
| Josué Tamayo | Visualizador |
