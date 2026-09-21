# Librerías

## Librerías Estándar (Arduino IDE)

Instalar desde el Library Manager de Arduino IDE:

1. **[Nombre Librería 1]** - v[X.X.X]
   - Propósito: [Descripción]
   - Instalación: `Sketch > Include Library > Manage Libraries > Buscar "[nombre]"`

2. **[Nombre Librería 2]** - v[X.X.X]
   - Propósito: [Descripción]
   - Instalación: [Instrucciones]

---

## Librerías Personalizadas/Modificadas

Si hay librerías modificadas o no disponibles en Arduino IDE, incluir en esta carpeta:

### Estructura
```
librerias/
├── MiLibreria/
│   ├── MiLibreria.h
│   ├── MiLibreria.cpp
│   ├── keywords.txt
│   └── README.md
```

### Instalación Manual
```bash
# Copiar carpeta a Arduino libraries
cp -r MiLibreria ~/Documents/Arduino/libraries/
```

---

## Dependencias Completas

Listar TODAS las librerías necesarias con versiones exactas:

| Librería | Versión | Fuente | Obligatoria |
|----------|---------|--------|-------------|
| WiFi | Built-in | ESP32 Core | ✅ |
| [Lib 1] | x.x.x | Arduino Library Manager | ✅ |
| [Lib 2] | x.x.x | GitHub | ❌ |
