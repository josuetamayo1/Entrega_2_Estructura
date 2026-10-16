"""
Prueba de la DLL del receptor desde Python.

1. Copia receptor.dll en la misma carpeta de este archivo.
2. Ejecuta:  python prueba_dll.py
3. Abre el cliente de prueba y envia mensajes(placa, hora, celda).
4. Ctrl + C para terminar.
"""
import ctypes
import os
import time

# Cargar la DLL desde la carpeta de este script
ruta_dll = os.path.join(os.path.dirname(os.path.abspath(__file__)), "receptor.dll")
lib = ctypes.CDLL(ruta_dll)

# Declarar los tipos de cada funcion(contrato del equipo)
lib.iniciar_receptor.argtypes = [ctypes.c_int]
lib.iniciar_receptor.restype = ctypes.c_int

lib.obtener_mensaje.argtypes = [ctypes.c_char_p, ctypes.c_int]
lib.obtener_mensaje.restype = ctypes.c_int

lib.detener_receptor.argtypes = []
lib.detener_receptor.restype = None

PUERTO = 8080

if lib.iniciar_receptor(PUERTO) != 1:
print(f"No se pudo iniciar el receptor en el puerto {PUERTO} (¿ya esta en uso?)")
raise SystemExit(1)

print(f"Receptor iniciado en el puerto {PUERTO}. Esperando mensajes... (Ctrl + C para salir)")

buffer = ctypes.create_string_buffer(256)  # memoria donde la DLL escribe el mensaje

try :
    while True :
        while lib.obtener_mensaje(buffer, len(buffer)) == 1 :
            placa, hora, celda = buffer.value.decode().split(",")
            print(f"Placa: {placa} | Hora: {hora} | Celda: {celda}")
            time.sleep(0.2)
            except KeyboardInterrupt :
pass
finally :
    lib.detener_receptor()  # siempre detener el hilo antes de salir
    print("Receptor detenido")