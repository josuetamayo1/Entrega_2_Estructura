# Pruebas

## Receptor

| Prueba | Mensaje enviado | Resultado esperado | Estado |
|---|---|---|---|
| Mensaje válido | `ABC123,18:30:05,12` | Se encola `ABC123,18:30:05,12` | ✅ |
| Formato inválido | `basura` | Se descarta | ✅ |
| Celda no numérica | `ABC123,18:30:05,xx` | Se descarta | ✅ |
| Mensajes pegados | Dos mensajes en un solo `send` | Se reciben los dos por separado | ✅ |
| DLL desde Python | `pruebas/prueba_dll.py` + `cliente_prueba` | Python imprime placa, hora y celda | ✅ |

## Integración

| Prueba | Componentes | Estado |
|---|---|---|
| Visualizador con la DLL en un PC | Visualizador + DLL + cliente de prueba | Pendiente |
| Generador con la DLL en un PC | Generador + `prueba_dll.py` | Pendiente |
| Sistema completo en un PC | Generador + DLL + visualizador | Pendiente |
| Sistema completo en dos PCs | Generador en el PC 1; DLL + visualizador en el PC 2 | Pendiente |
| Reconexión | Cerrar y volver a abrir el visualizador con el generador corriendo | Pendiente |
