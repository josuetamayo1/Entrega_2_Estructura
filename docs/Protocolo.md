# Protocolo de comunicación

El generador y el receptor se comunican por **TCP en el puerto 8080**. El generador solo envía; el receptor no responde.

## Formato del mensaje

```
placa,hora,celda\n
```

| Campo | Formato | Ejemplo |
|---|---|---|
| placa | 3 letras mayúsculas y 3 dígitos | `ABC123` |
| hora | `HH:MM:SS` | `18:30:05` |
| celda | Entero de 1 a 30 | `12` |

Ejemplo completo: `ABC123,18:30:05,12\n`

## Por qué termina en `\n`

TCP no envía mensajes separados, sino un **flujo de bytes**. Un solo `recv` puede traer un mensaje completo, dos pegados o medio mensaje. El salto de línea marca dónde termina cada uno, y el receptor acumula lo recibido hasta encontrarlo.

## Reglas

- **Entrada:** la placa no está en el parqueadero. Ocupa la celda indicada, que debe estar libre.
- **Salida:** la placa ya está en el parqueadero. Se envía con la misma celda que ocupaba y esa celda queda libre.
- El generador decide aleatoriamente entre entrada y salida. Si el parqueadero está lleno, siempre envía una salida; si está vacío, siempre una entrada.

## Validación

| Dónde | Qué valida |
|---|---|
| Receptor (DLL) | Que haya exactamente 3 campos y que la celda sea un número. Si no, descarta el mensaje. |
| Visualizador | Placa no vacía, hora válida, celda entre 1 y 30, y que una entrada no llegue a una celda ocupada. Los errores se muestran en el historial. |
