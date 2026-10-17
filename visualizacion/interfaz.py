


import tkinter as tk
from tkinter import ttk

from logica import NUM_CELDAS, Parqueadero
import receptor

COLOR_LIBRE = "#2e7d32"
COLOR_OCUPADA = "#e65100"
COLOR_RESALTE = "#f9a825"  
COLOR_TEXTO = "white"
COLOR_TEXTO_RESALTE = "#212121"


COLOR_ENTRADA = "#2e7d32"
COLOR_SALIDA = "#1565c0"
COLOR_ERROR = "#c62828"

DURACION_RESALTE_MS = 600


FILAS = 5
COLUMNAS = 6


class App(tk.Tk):
    def __init__(self):
        super().__init__()
        self.title("Parqueadero — Visualizador")
        self.geometry("1200x700")
        self.minsize(1050, 600)

        if FILAS * COLUMNAS != NUM_CELDAS:
            raise ValueError(
                f"La grilla es {FILAS}x{COLUMNAS}={FILAS * COLUMNAS}, "
                f"pero NUM_CELDAS={NUM_CELDAS}. Deben coincidir."
            )

        self._aplicar_tema()

        self.parqueadero = Parqueadero(NUM_CELDAS)
        self.corriendo = False
        self._after_id = None
        self.celdas_ui: dict[int, tuple[tk.Frame, tk.Label, tk.Label]] = {}
        self._resaltes: dict[int, str] = {}

        self._armar()
        self._pintar_todas()
        self.protocol("WM_DELETE_WINDOW", self._al_cerrar)

    def _aplicar_tema(self):
        estilo = ttk.Style(self)
        try:
            estilo.theme_use("clam")
        except tk.TclError:
            pass
        estilo.configure("Treeview", rowheight=24, font=("Consolas", 10))
        estilo.configure("Treeview.Heading", font=("Segoe UI", 10, "bold"))
        estilo.configure("TButton", padding=6)

    def _armar(self):
        raiz = ttk.Frame(self, padding=14)
        raiz.pack(fill="both", expand=True)

        encabezado = ttk.Frame(raiz)
        encabezado.pack(fill="x")

        ttk.Label(
            encabezado, text="Parqueadero", font=("Segoe UI", 18, "bold")
        ).pack(side="left")

        self.contadores = ttk.Label(
            encabezado, text="", font=("Segoe UI", 12, "bold")
        )
        self.contadores.pack(side="right")

        cuerpo = ttk.Frame(raiz)
        cuerpo.pack(fill="both", expand=True, pady=(10, 0))

        marco_celdas = ttk.LabelFrame(cuerpo, text="Celdas", padding=10)
        marco_celdas.pack(side="left", fill="both", expand=True)

        grilla = ttk.Frame(marco_celdas)
        grilla.pack(pady=(4, 8))

        n = 1
        for fila in range(FILAS):
            for col in range(COLUMNAS):
                frame = tk.Frame(
                    grilla, width=100, height=70, bg=COLOR_LIBRE, bd=0
                )
                frame.grid(row=fila, column=col, padx=4, pady=4)
                frame.pack_propagate(False)

                lbl_num = tk.Label(
                    frame,
                    text=f"Celda {n}",
                    bg=COLOR_LIBRE,
                    fg=COLOR_TEXTO,
                    font=("Segoe UI", 8),
                )
                lbl_num.pack(pady=(6, 0))

                lbl_placa = tk.Label(
                    frame,
                    text="LIBRE",
                    bg=COLOR_LIBRE,
                    fg=COLOR_TEXTO,
                    font=("Consolas", 12, "bold"),
                )
                lbl_placa.pack(expand=True)

                self.celdas_ui[n] = (frame, lbl_num, lbl_placa)
                n += 1

        leyenda = ttk.Frame(marco_celdas)
        leyenda.pack()
        self._item_leyenda(leyenda, COLOR_LIBRE, "Libre")
        self._item_leyenda(leyenda, COLOR_OCUPADA, "Ocupada")
        self._item_leyenda(leyenda, COLOR_RESALTE, "Cambio reciente")

        lado = ttk.Frame(cuerpo)
        lado.pack(side="right", fill="both", padx=(14, 0))

        botones = ttk.Frame(lado)
        botones.pack(fill="x")

        self.btn = ttk.Button(botones, text="Iniciar", command=self._toggle)
        self.btn.pack(side="left")

        ttk.Button(
            botones, text="Limpiar historial", command=self._limpiar
        ).pack(side="left", padx=8)

        modo = "MOCK" if receptor.USAR_MOCK else "DLL"
        self.estado = ttk.Label(lado, text=f"Detenido  |  modo: {modo}")
        self.estado.pack(anchor="w", pady=(10, 4))

        ttk.Label(
            lado, text="Historial", font=("Segoe UI", 10, "bold")
        ).pack(anchor="w")

        marco_tabla = ttk.Frame(lado)
        marco_tabla.pack(fill="both", expand=True)

        columnas = ("hora", "tipo", "placa", "celda", "detalle")
        self.tabla = ttk.Treeview(
            marco_tabla,
            columns=columnas,
            show="headings",
            selectmode="browse",
        )

        encabezados = {
            "hora": ("Hora", 75),
            "tipo": ("Tipo", 80),
            "placa": ("Placa", 80),
            "celda": ("Celda", 55),
            "detalle": ("Detalle", 260),
        }
        for col, (texto, ancho) in encabezados.items():
            self.tabla.heading(col, text=texto)
            self.tabla.column(
                col, width=ancho, anchor="w", stretch=(col == "detalle")
            )

        self.tabla.tag_configure("ENTRADA", foreground=COLOR_ENTRADA)
        self.tabla.tag_configure("SALIDA", foreground=COLOR_SALIDA)
        self.tabla.tag_configure("ERROR", foreground=COLOR_ERROR)

        scroll_y = ttk.Scrollbar(
            marco_tabla, orient="vertical", command=self.tabla.yview
        )
        scroll_x = ttk.Scrollbar(
            marco_tabla, orient="horizontal", command=self.tabla.xview
        )
        self.tabla.configure(
            yscrollcommand=scroll_y.set, xscrollcommand=scroll_x.set
        )

        self.tabla.grid(row=0, column=0, sticky="nsew")
        scroll_y.grid(row=0, column=1, sticky="ns")
        scroll_x.grid(row=1, column=0, sticky="ew")
        marco_tabla.rowconfigure(0, weight=1)
        marco_tabla.columnconfigure(0, weight=1)

    @staticmethod
    def _item_leyenda(padre, color, texto):
        cuadro = tk.Label(padre, bg=color, width=2)
        cuadro.pack(side="left", padx=(10, 4))
        ttk.Label(padre, text=texto).pack(side="left")

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
            eventos = []
            for _ in range(10):
                hay, texto = receptor.obtener_mensaje()
                if hay != 1:
                    break
                evento = self.parqueadero.procesar(texto)
                self._agregar_historial(evento)
                eventos.append(evento)

            if eventos:
                self._pintar_todas()
                for evento in eventos:
                    self._resaltar_evento(evento)
        except Exception as e:
            self.estado.config(text=f"Error al consultar: {e}")
        finally:
            if self.corriendo:
                self._after_id = self.after(400, self._consultar)

    def _pintar_celda(self, celda: int):
        placa = self.parqueadero.celdas[celda]
        frame, lbl_num, lbl_placa = self.celdas_ui[celda]
        if placa is None:
            color, texto = COLOR_LIBRE, "LIBRE"
        else:
            color, texto = COLOR_OCUPADA, placa
        frame.config(bg=color)
        lbl_num.config(bg=color, fg=COLOR_TEXTO)
        lbl_placa.config(bg=color, fg=COLOR_TEXTO, text=texto)

    def _pintar_todas(self):
        for celda in self.parqueadero.celdas:
            self._pintar_celda(celda)
        self._actualizar_contadores()

    def _actualizar_contadores(self):
        ocupadas = sum(
            1 for p in self.parqueadero.celdas.values() if p is not None
        )
        libres = len(self.parqueadero.celdas) - ocupadas
        self.contadores.config(
            text=f"Ocupadas: {ocupadas}   |   Libres: {libres}"
        )

    def _resaltar_evento(self, evento):
        if evento.tipo not in ("ENTRADA", "SALIDA"):
            return
        try:
            celda = int(evento.celda)
        except ValueError:
            return
        if celda not in self.celdas_ui:
            return

        previo = self._resaltes.pop(celda, None)
        if previo is not None:
            try:
                self.after_cancel(previo)
            except tk.TclError:
                pass

        frame, lbl_num, lbl_placa = self.celdas_ui[celda]
        frame.config(bg=COLOR_RESALTE)
        lbl_num.config(bg=COLOR_RESALTE, fg=COLOR_TEXTO_RESALTE)
        lbl_placa.config(bg=COLOR_RESALTE, fg=COLOR_TEXTO_RESALTE)

        self._resaltes[celda] = self.after(
            DURACION_RESALTE_MS, lambda c=celda: self._fin_resalte(c)
        )

    def _fin_resalte(self, celda: int):
        self._resaltes.pop(celda, None)
        self._pintar_celda(celda)

    def _agregar_historial(self, evento):
        item = self.tabla.insert(
            "",
            "end",
            values=(
                evento.hora,
                evento.tipo,
                evento.placa,
                evento.celda,
                evento.detalle,
            ),
            tags=(evento.tipo,),
        )
        self.tabla.see(item)

    def _limpiar(self):
        self.tabla.delete(*self.tabla.get_children())
        self.parqueadero.limpiar_historial()

    def _al_cerrar(self):
        self.corriendo = False
        if self._after_id is not None:
            try:
                self.after_cancel(self._after_id)
            except tk.TclError:
                pass
            self._after_id = None

        for id_resalte in self._resaltes.values():
            try:
                self.after_cancel(id_resalte)
            except tk.TclError:
                pass
        self._resaltes.clear()

        receptor.detener_receptor()
        self.destroy()


def main():
    app = App()
    app.mainloop()