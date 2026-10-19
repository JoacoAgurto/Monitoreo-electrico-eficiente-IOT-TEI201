# Historial de Iteraciones

## Propósito

Esta carpeta documenta la evolución del prototipo a través de las diferentes versiones desarrolladas durante el curso.

---

## Contenido de Cada Iteración

Cada carpeta debe incluir:

1. **README.md** - Descripción de la versión
2. **Fotos** - Imágenes del prototipo en ese estado
3. **Esquema** - Circuito de esa versión (si cambió)
4. **Código** - Versión del software (si cambió)
5. **Notas** - Problemas identificados y mejoras planificadas

---

## Tabla Comparativa de Iteraciones

| Aspecto | v1 | v2 | v3 |
|---------|----|----|----| 
| **Fecha** | [19/10/26] | [fecha] | [fecha] |
| **Sensores** | [ADS1115 / Arduino esp32 cp210] | [lista] | [lista] |
| **Actuadores** | [] | [lista] | [lista] |
| **Conectividad** | [local / WIFI] | [tipo] | [tipo] |
| **Encapsulado** | Concepto visual | Prototipo | Finalizado |
| **Testing** | Solo equipo | 2 usuarios | 5+ usuarios |
| **Funcionalidad** | 95% | 80% | 100% |

---

## Evolución del Diseño

### Versión 1 (Iteración 1) - Concepto Inicial
**Fecha:** [19/10/2026]  
**Estado:** Prototipo alpha - Prueba de concepto

**Características:**
- El sistema es a base de conexiones de arduino / sensores montados en un protoboard, capaces de leer corriente eléctrica y mandar esos datos a una página web.
- El sistema es muy dependiente por el hecho de que los cables de conexión al sistema sean separados y no juntos.
- Para mejor usabilidad, es mejor implementar una nube para así ver el sistema de manera remota.

### Versión 2 (Iteración 2) - Primera Mejora
**Fecha:** [XX/XX/2025]  
**Estado:** Prototipo funcional mejorado

**Mejoras implementadas:**
- [Mejora 1]
- [Mejora 2]

**Problemas pendientes:**
- [Problema 1]
- [Problema 2]

### Versión 3 (Iteración 3) - Versión Final
**Fecha:** [XX/XX/2025]  
**Estado:** Prototipo de alta resolución

**Mejoras finales:**
- [Mejora 1]
- [Mejora 2]

**Resultado:**
- Sistema robusto y probado
- Listo para piloto en terreno

---

## Justificación de Cambios

### Hardware
**De v1 a v2:**
- Cambio: [Descripción]
- Razón: [Justificación técnica]
- Impacto: [Mejora cuantitativa]

**De v2 a v3:**
- Cambio: [Descripción]
- Razón: [Justificación técnica]
- Impacto: [Mejora cuantitativa]

### Software
**De v1 a v2:**
- [Cambios principales]

**De v2 a v3:**
- [Cambios principales]

### Diseño 3D
**De v1 a v2:**
- [Cambios en encapsulado]

**De v2 a v3:**
- [Cambios finales]

---

## Incorporación de Feedback

### Feedback del Avance #1 → Cambios en v2
1. [Comentario profesor/ayudante]:
   - Utilizar un servidor global para garantizar un acceso al sistema de manera remota, facilitando su uso para el usuario en vez de estar en cercanía con el sistema montado en el protoboard. La importancia de esto para el usuario es que es capaz de saber la cantidad de energía que está consumiendo en su hogar cuando NO está en ella.
   - Acción tomada: Usar Firebase

### Feedback del Avance #2 → Cambios en v3
1. [Comentario profesor/ayudante]
   - Acción tomada: [Descripción]

### Feedback de Usuarios → Cambios en v3
1. [Comentario común de usuarios]
   - Acción tomada: [Descripción]

---

## Métricas de Mejora

| Métrica | v1 | v2 | v3 | Mejora Total |
|---------|----|----|----| -------------|
| Tiempo respuesta (ms) | 500 | 200 | 100 | ↓80% |
| Consumo (mA) | 250 | 180 | 120 | ↓52% |
| Tasa de errores (%) | 15 | 5 | 1 | ↓93% |
| Satisfacción usuario (1-5) | 3.0 | 3.8 | 4.5 | ↑50% |

---

## Lecciones Aprendidas

### Técnicas
- [Lección 1]
- [Lección 2]

### De Proceso
- [Lección 1]
- [Lección 2]

### De Trabajo en Equipo
- [Lección 1]
- [Lección 2]
