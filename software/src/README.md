# Código Fuente

## Archivos Principales

### `main.ino`
Archivo principal del proyecto Arduino/ESP32

**Estructura:**
- Setup: Inicialización del sistema
- Loop: Ciclo principal de ejecución

### Otros archivos (si aplica)
- `config.h` - Configuraciones y constantes
- `sensores.h` / `sensores.cpp` - Funciones de sensores
- `actuadores.h` / `actuadores.cpp` - Control de actuadores
- `comunicacion.h` / `comunicacion.cpp` - WiFi/Bluetooth

---

## Estándares de Código

### Comentarios
```cpp
// Comentarios en español
// Explicar QUÉ hace y POR QUÉ (no solo el CÓMO)

/**
 * Función: leerTemperatura
 * Descripción: Lee temperatura del sensor DHT22
 * Retorna: float - Temperatura en grados Celsius
 */
float leerTemperatura() {
  // código
}
```

### Nombres de Variables
- Variables: `camelCase`
- Constantes: `UPPER_SNAKE_CASE`
- Funciones: `camelCase()`

---

## Testing y Debugging

### Modo Debug
Descomentar para activar modo debug:
```cpp
#define DEBUG true
```

### Monitor Serial
- Baudrate: 115200
- Mensajes informativos con prefijos:
  - `[INFO]` - Información general
  - `[ERROR]` - Errores
  - `[DEBUG]` - Debug
