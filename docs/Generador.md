# Generador de placas

**Responsable:** Jhoan · **Código:** `generador/`

Proceso cliente en C++ que simula los carros que entran y salen del parqueadero, y envía cada evento al receptor por TCP.

## Clases

| Clase | Responsabilidad |
|---|---|
| `Simulador` | Ciclo principal: decide entrada o salida, envía el evento y espera de 2 a 5 s. |
| `Parqueadero` | Lleva el estado local de las 30 celdas: ocupar, liberar, elegir celdas libres u ocupadas al azar. |
| `GeneradorPlacas` | Genera placas aleatorias con formato `ABC123`. |
| `Evento` | Une placa, hora actual y celda, y produce el texto `placa,hora,celda\n`. |
| `ClienteTCP` | Conexión con el receptor: conectar, reconectar cada 3 s y enviar asegurando que salgan todos los bytes. |
| `Config` | Lee la IP y el puerto desde `config.txt` (por defecto `127.0.0.1` y `8080`). |

## Ciclo del simulador

1. Decidir el tipo de evento: si el parqueadero está lleno, salida; si está vacío, entrada; si no, 50 % de probabilidad para cada uno.
2. **Entrada:** generar una placa que no esté adentro y elegir una celda libre.
3. **Salida:** elegir una celda ocupada y usar su placa y esa misma celda.
4. Enviar el evento. Si la conexión se perdió, reconectar y reenviar el mismo mensaje.
5. Actualizar el parqueadero local **solo después de enviar con éxito**, para no quedar desincronizado con el visualizador.
6. Esperar un tiempo aleatorio entre 2 y 5 segundos.

## Configuración

`config.txt` debe estar junto al `.exe`:

```
# Configuracion del Generador
ip=127.0.0.1
puerto=8080
```

Para la prueba en dos PCs, `ip` debe ser la IP del PC donde corre el visualizador.

## Diagrama de clases

*(Pendiente)*
