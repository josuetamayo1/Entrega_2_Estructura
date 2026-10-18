# Arquitectura

```mermaid
flowchart LR
    subgraph PC1["PC 1"]
        G["Generador<br/>(C++ · cliente TCP)"]
    end
    subgraph PC2["PC 2"]
        R["receptor.dll<br/>(C++ · servidor TCP)"]
        V["Visualizador<br/>(Python · Tkinter)"]
    end
    G -- "TCP · puerto 8080" --> R
    V -- "ctypes" --> R
```

## Componentes

| Componente | Rol | Proceso | Comunicación |
|---|---|---|---|
| Generador | Cliente | `generador.exe` en el PC 1 | Envía mensajes por un socket TCP |
| Receptor | Servidor | Corre **dentro** del proceso de Python en el PC 2 | Escucha el socket y entrega los mensajes por funciones de la DLL |
| Visualizador | Interfaz | `python main.py` en el PC 2 | Llama a la DLL con `ctypes` |

## Por qué el receptor es una DLL

El enunciado pide que el envío y la recepción de placas se hagan en C++ y que el visualizador incorpore una librería dinámica para capturar la información del generador. Por eso el servidor de sockets vive dentro de `receptor.dll`:

1. El visualizador carga la DLL y llama a `iniciar_receptor(8080)`.
2. La DLL abre el socket y lanza un **hilo** que recibe los mensajes en segundo plano. La función regresa de inmediato, así la ventana de Python no se congela.
3. Cada mensaje válido se guarda en una **cola** protegida con un **mutex**.
4. El visualizador llama a `obtener_mensaje()` cada 400 ms y procesa lo que haya en la cola.

## Flujo de un mensaje

```mermaid
sequenceDiagram
    participant G as Generador (C++)
    participant H as Hilo de la DLL
    participant C as Cola (mutex)
    participant V as Visualizador (Python)
    V->>H: iniciar_receptor(8080)
    G->>H: connect()
    loop cada 2 a 5 s
        G->>H: "ABC123,18:30:05,12\n"
        H->>H: separar por \n y validar
        H->>C: encolar mensaje
    end
    loop cada 400 ms
        V->>C: obtener_mensaje()
        C-->>V: "ABC123,18:30:05,12"
        V->>V: entrada o salida y actualizar celda
    end
    V->>H: detener_receptor()
```
