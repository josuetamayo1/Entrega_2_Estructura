
import tkinter as tk
from tkinter import ttk

from logica import NUM_CELDAS, Parqueadero
import receptor


COLOR_LIBRE = "#2e7d32"
COLOR_OCUPADA = "#e65100"
COLOR_TEXTO = "white"

FILAS = 5
COLUMNAS = 6


class App(tk.Tk):
    def __init__(self):
        super().__init__()
        self.title("Parqueadero — Visualizador")
        self.geometry("1100x620")
        self.minsize(1000, 520)

        if FILAS * COLUMNAS != NUM_CELDAS:
            raise ValueError(
                f"La grilla es {FILAS}x{COLUMNAS}={FILAS * COLUMNAS}, "
                f"pero NUM_CELDAS={NUM_CELDAS}. Deben coincidir."
            )

        self.parqueadero = Parqueadero(NUM_CELDAS)
        self.corriendo = False
        self._after_id = None 
        self.labels: dict[int, tk.Label] = {}

        self._armar()
        self._pintar_todas()
        self.protocol("WM_DELETE_WINDOW", self._al_cerrar)

    def _armar(self):
        raiz = ttk.Frame(self, padding=12)
        raiz.pack(fill="both", expand=True)

        ttk.Label(raiz, text="Parqueadero", font=("Segoe UI", 16, "bold")).pack(anchor="w")

        cuerpo = ttk.Frame(raiz)
        cuerpo.pack(fill="both", expand=True, pady=(8, 0))

        marco_celdas = ttk.LabelFrame(cuerpo, text="Celdas", padding=8)
        marco_celdas.pack(side="left", fill="both", expand=True)

        grilla = ttk.Frame(marco_celdas)
        grilla.pack()

        n = 1
        for fila in range(FILAS):
            for col in range(COLUMNAS):
                lbl = tk.Label(
                    grilla,
                    text=f"{n}\nLIBRE",
                    width=10,
                    height=2,
                    bg=COLOR_LIBRE,
                    fg=COLOR_TEXTO,
                    font=("Consolas", 9, "bold"),
                    relief="raised",
                    bd=2,
                )
                lbl.grid(row=fila, column=col, padx=3, pady=3)
                self.labels[n] = lbl
                n += 1

        lado = ttk.Frame(cuerpo)
        lado.pack(side="right", fill="both", padx=(12, 0))

        botones = ttk.Frame(lado)
        botones.pack(fill="x")

        self.btn = ttk.Button(botones, text="Iniciar", command=self._toggle)
        self.btn.pack(side="left")

        ttk.Button(botones, text="Limpiar historial", command=self._limpiar).pack(
            side="left", padx=8
        )

        modo = "MOCK" if receptor.USAR_MOCK else "DLL"
        self.estado = ttk.Label(lado, text=f"Detenido  |  modo: {modo}")
        self.estado.pack(anchor="w", pady=(8, 4))

        ttk.Label(lado, text="Historial").pack(anchor="w")

        marco_lista = ttk.Frame(lado)
        marco_lista.pack(fill="both", expand=True)

        self.lista = tk.Listbox(marco_lista, width=52, font=("Consolas", 9))
        scroll_y = ttk.Scrollbar(marco_lista, orient="vertical", command=self.lista.yview)
        scroll_x = ttk.Scrollbar(marco_lista, orient="horizontal", command=self.lista.xview)
        self.lista.configure(yscrollcommand=scroll_y.set, xscrollcommand=scroll_x.set)

        self.lista.grid(row=0, column=0, sticky="nsew")
        scroll_y.grid(row=0, column=1, sticky="ns")
        scroll_x.grid(row=1, column=0, sticky="ew")
        marco_lista.rowconfigure(0, weight=1)
        marco_lista.columnconfigure(0, weight=1)

    def _toggle(self):
        if self.corriendo:
            self._detener()
        else:
            self._iniciar()

    def _iniciar(self):
        ok = receptor.iniciar_receptor(receptor.PUERTO)
        if ok != 1:
            self.estado.config(text="No se pudo iniciar el receptor")
            return

        self.corriendo = True
        self.btn.config(text="Detener")
        modo = "MOCK" if receptor.USAR_MOCK else "DLL"
        self.estado.config(text=f"Recibiendo  |  modo: {modo}")
        self._after_id = self.after(400, self._consultar)

    def _detener(self):
        self.corriendo = False
        if self._after_id is not None:
            try:
                self.after_cancel(self._after_id)
            except tk.TclError:
                pass
            self._after_id = None

        receptor.detener_receptor()
        self.btn.config(text="Iniciar")
        self.estado.config(text="Detenido")

    def _consultar(self):
     
        if not self.corriendo:
            self._after_id = None
            return

        try:
            hubo_eventos = False
            for _ in range(10):
                hay, texto = receptor.obtener_mensaje()
                if hay != 1:
                    break
                evento = self.parqueadero.procesar(texto)
                self._agregar_historial(evento)
                hubo_eventos = True
            if hubo_eventos:
                self._pintar_todas()  
        except Exception as e:
            self.estado.config(text=f"Error al consultar: {e}")
        finally:
            if self.corriendo:
                self._after_id = self.after(400, self._consultar)

    def _pintar_todas(self):
        for celda, placa in self.parqueadero.celdas.items():
            lbl = self.labels[celda]
            if placa is None:
                lbl.config(text=f"{celda}\nLIBRE", bg=COLOR_LIBRE)
            else:
                lbl.config(text=f"{celda}\n{placa}", bg=COLOR_OCUPADA)

    def _agregar_historial(self, evento):
        linea = (
            f"{evento.hora}  {evento.tipo:<8}  {evento.placa:<8}  "
            f"celda {evento.celda}  {evento.detalle}"
        )
        self.lista.insert("end", linea)
        self.lista.see("end")

    def _limpiar(self):
        self.lista.delete(0, "end")
        self.parqueadero.limpiar_historial()

    def _al_cerrar(self):
        self.corriendo = False
        if self._after_id is not None:
            try:
                self.after_cancel(self._after_id)
            except tk.TclError:
                pass
            self._after_id = None
        receptor.detener_receptor()
        self.destroy()


def main():
    app = App()
    app.mainloop()