# Iteración 1 - Concepto Inicial

## Información General

**Fecha:** [19/10/2026]  
**Versión:** v1.0  
**Estado:** Prototipo alpha y Prueba de concepto  
**Avance asociado:** Avance #1 con prototipo accesible y manipulable

---

## Descripción

Esta primera iteración contiene la primera versión del prototipo del sistema. Su enfoque es validar su funcionamiento y factibilidad técnica. El protoboard contiene todo el hardware necesario para recopilar datos de consumo de energía de manera local / remota, mediante conexión WIFI.

**Objetivo de esta iteración:**
- Probar conexión de sensores básicos
- Validar comunicación con microcontrolador
- Demostrar concepto de actuación
- Circuito básico y soldado en protoboard

---

## Componentes Utilizados

### Hardware
- Microcontrolador: Micro SD
- Sensores:
  - ADS1115
- Actuadores:
  - [Actuador 1]
- Otros: [Lista]

### Software
- IDE: Arduino esp32 cp210
- Librerías: [Lista]
- Versión código: v1.0

---

## Fotos del Prototipo

Incluir en esta carpeta:
- `v1_circuito_protoboard.jpg`
- `v1_vista_general.jpg`
- `v1_detalle_conexiones.jpg`

---

## Resultados de Testing Inicial

### Funcionalidad Lograda
✅ Leer corriente eléctrica 
✅ Recopilar los datos y subirlos en la página web
❌ No reconoce entre corriente real y ruido

### Problemas Identificados

1. **Problema 1:**
El problema principal es que el sistema no funciona se está midiendo en cable junto, sólo cuando es cables separados. Esto ocurre ya que al medir por cable junto, ambos cable anulan las ondas de electricidad medibles, y el sistema no leerá nada.
   - Solución propuesta: [Plan]

---

## Aprendizajes

### Técnicos
La tarjeta de memoria dejaría de ser útil si cambiamos el servidor de local, a global. Esto permite que los datos recopilados se puedan ver de forma remota y no sea necesario revisar la tarjeta cuando todo estará guardado en una nube.

### De Diseño
- Para los broches plásticos en el modelaje de encapsulado, se aprendió que es mucho mejor utilizar broche estándar de plástico inyectado, puesto que la técnica anterior será muy sensible (siendo impreso por 3D) y podrían romperse.

---

## Plan para Iteración 2

### Mejoras Planificadas

**Hardware:**
- [ ] 

**Software:**
- [ ] Utilizar nube como repositorio de datos (Firebase)

**Diseño:**
- [ ] Iniciar diseño de encapsulado para tenerlo en formato físico

---

## Archivos en esta Carpeta

- `v1_codigo.ino` - Código de esta versión
- `v1_esquema.png` - Esquema del circuito
- `v1_fotos/` - Fotografías del prototipo
- `v1_notas.txt` - Notas adicionales del equipo
