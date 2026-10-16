from dataclasses import dataclass


NUM_CELDAS = 30


@dataclass
class Evento:
    tipo: str       
    placa: str
    hora: str
    celda: str
    detalle: str


class Parqueadero:
    def __init__(self, num_celdas: int = NUM_CELDAS):
        self.num_celdas = num_celdas

        self.celdas: dict[int, str | None] = {i: None for i in range(1, num_celdas + 1)}
       
        self.por_placa: dict[str, int] = {}
        self.historial: list[Evento] = []

    def procesar(self, mensaje: str) -> Evento:
 
        partes = mensaje.strip().split(",")
        if len(partes) != 3:
            return self._error(mensaje, "", "", "Formato inválido (se esperaba placa,hora,celda)")

        placa, hora, celda_txt = (p.strip() for p in partes)

        if not placa:
            return self._error(mensaje, placa, hora, "Placa vacía")
        if not self._hora_ok(hora):
            return self._error(mensaje, placa, hora, "Hora inválida")

        try:
            celda = int(celda_txt)
        except ValueError:
            return self._error(mensaje, placa, hora, f"Celda no numérica: {celda_txt}")

        if celda < 1 or celda > self.num_celdas:
            return self._error(mensaje, placa, hora, f"Celda fuera de rango: {celda}")


        if placa in self.por_placa:
            celda_real = self.por_placa.pop(placa)
            self.celdas[celda_real] = None
            evento = Evento("SALIDA", placa, hora, str(celda_real), "Liberó la celda")
            self.historial.append(evento)
            return evento

        
        if self.celdas[celda] is not None:
            ocupante = self.celdas[celda]
            return self._error(
                mensaje, placa, hora,
                f"Celda {celda} ya ocupada por {ocupante}",
            )

        self.celdas[celda] = placa
        self.por_placa[placa] = celda
        evento = Evento("ENTRADA", placa, hora, str(celda), "Ocupó la celda")
        self.historial.append(evento)
        return evento

    def limpiar_historial(self) -> None:
        
        self.historial.clear()

    def _error(self, _mensaje: str, placa: str, hora: str, detalle: str) -> Evento:
        evento = Evento("ERROR", placa or "—", hora or "—", "—", detalle)
        self.historial.append(evento)
        return evento

    @staticmethod
    def _hora_ok(hora: str) -> bool:
        partes = hora.split(":")
        if len(partes) != 3:
            return False
        try:
            h, m, s = (int(x) for x in partes)
        except ValueError:
            return False
        return 0 <= h <= 23 and 0 <= m <= 59 and 0 <= s <= 59