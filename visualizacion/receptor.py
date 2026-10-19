import ctypes
import os
import random
from datetime import datetime

from logica import NUM_CELDAS


USAR_MOCK = False

PUERTO = 8080

_PLACAS = ["ABC123", "XYZ789", "QWE456", "RTY321", "MNO654", "JKL987"]

dll = None       
_activo = False  


def iniciar_receptor(puerto: int = PUERTO) -> int:

    global _activo, dll

    if USAR_MOCK:
        _activo = True
        return 1


    try:
        carpeta = os.path.dirname(os.path.abspath(__file__))
        if hasattr(os, "add_dll_directory"):
            os.add_dll_directory(carpeta)

        dll = ctypes.CDLL(os.path.join(carpeta, "receptor.dll"))
        dll.iniciar_receptor.argtypes = [ctypes.c_int]
        dll.iniciar_receptor.restype = ctypes.c_int
        dll.obtener_mensaje.argtypes = [ctypes.c_char_p, ctypes.c_int]
        dll.obtener_mensaje.restype = ctypes.c_int
        dll.detener_receptor.argtypes = []
        dll.detener_receptor.restype = None
    except (OSError, AttributeError):
      
        dll = None
        _activo = False
        return 0

    resultado = dll.iniciar_receptor(puerto)
    _activo = (resultado == 1) 
    return resultado


def obtener_mensaje() -> tuple[int, str]:

    if not _activo:
        return 0, ""

    if USAR_MOCK:
      
        if random.random() < 0.35:
            return 0, ""

        if random.random() < 0.08:
            return 1, random.choice([
                "sin comas",
                "ABC123,18:30",
                f"ABC123,18:30:05,{NUM_CELDAS + 1}",  
                ",,",
                "ABC123,mala,3",
            ])

        placa = random.choice(_PLACAS)
        hora = datetime.now().strftime("%H:%M:%S")
        celda = random.randint(1, NUM_CELDAS)
        return 1, f"{placa},{hora},{celda}"

    if dll is None:
        return 0, ""

    buf = ctypes.create_string_buffer(128)
    if dll.obtener_mensaje(buf, 128) == 1:
        return 1, buf.value.decode("utf-8", errors="replace")
    return 0, ""


def detener_receptor() -> None:
    """Equivale a: void detener_receptor();"""
    global _activo
    _activo = False
    if not USAR_MOCK and dll is not None:
        dll.detener_receptor()