# AGENTS.md — Guía Mandatoria para Agentes de IA

Este documento define las reglas obligatorias de arquitectura, compilación, UI y protocolo MIDI/SysEx para cualquier agente que trabaje en este repositorio (**Roland JV-1010 Visual Editor & Librarian**).

---

## 1. Stack y Estructura del Proyecto

- **Lenguaje y Frameworks**: C++20 + JUCE 8 (`third_party/JUCE`) + Dear ImGui (`third_party/imgui`, rama `docking`, configurado como submódulo git) sobre OpenGL3 (`Source/UI/ImGuiHost.cpp`).
- **CMake Targets**:
  - `JV1010Tests`: Suite de pruebas unitarias (`build/Release/JV1010Tests.exe`).
  - `JV1010EditorPro_VST3`: Plugin VST3 (`build/JV1010EditorPro_artefacts/Release/VST3/JV-1010 Editor Pro.vst3`).
  - `JV1010EditorPro_Standalone`: Aplicación Standalone (`build/JV1010EditorPro_artefacts/Release/Standalone/JV-1010 Editor Pro.exe`).

---

## 2. Reglas de Compilación y Pruebas (Windows / MSVC)

### OBLIGATORIO
1. **Siempre** definir `_CL_="/Z7"` antes de invocar `cmake` para evitar el error `C1902: Program database manager mismatch`:
   ```powershell
   $env:_CL_="/Z7"; & "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe" --build c:\Users\marti\StudioProjects\JVSYSEX\build --target JV1010Tests JV1010EditorPro_VST3 JV1010EditorPro_Standalone --config Release
   ```
2. **Siempre** ejecutar `c:\Users\marti\StudioProjects\JVSYSEX\build\Release\JV1010Tests.exe` después de cada cambio y verificar que pase (`All JV-1010 SysEx, State, Database & File I/O tests passed!`).
3. Si el enlazado de `JV1010EditorPro_Standalone` falla con `LNK1104`, es porque el usuario tiene `JV-1010 Editor Pro.exe` abierto: informarle para que lo cierre y luego re-enlazar.

### PROHIBIDO
- **NO** pasar `-j 8` (o paralelismo alto de MSBuild) en este entorno, ya que puede colgar el build.
- **NO** reportar pruebas o compilaciones como exitosas sin haberlas ejecutado tras la última edición.

---

## 3. Reglas de Interfaz de Usuario (Dear ImGui — Estética Hardware Roland)

### OBLIGATORIO
1. **Homogeneidad fotorrealista**: Todos los controles de parámetros deben usar los widgets hardware definidos en `Source/UI/ImGuiWidgets.h` / `.cpp`:
   - **Sliders horizontales**: usar `ui::FaderH(...)`.
   - **Sliders verticales / faders de mixer**: usar `ui::FaderV(...)`.
   - **Switches On/Off**: usar `ui::ToggleSwitch(...)` (palanca metálica con LED).
   - **Perillas**: usar `ui::KnobInt(...)`.
   - **Botones de modo/selección**: usar `ui::RolandButton(...)`.
   - **Encabezados de sección**: usar `ui::SectionHeader(...)`.
2. **IDs únicos en ImGui**: Si dos o más secciones visibles comparten nombres de parámetros (ej. `TVA`, `TVF`, `PITCH` en `PatchEditor.cpp`), envolver cada sección con `ImGui::PushID("...")` y `ImGui::PopID()`.
3. **Teclado virtual**: `ui::VirtualKeyboard` solo debe disparar `onNote` cuando `ImGui::IsItemActive() && ImGui::IsMouseDown(ImGuiMouseButton_Left)`, nunca al pasar el cursor por encima.
4. **Ventana redimensionable y maximizable**: Mantener `setResizable(true, true)`, límites amplios (`16384x16384`) y `DocumentWindow::allButtons` en `JV1010EditorAudioProcessorEditor::parentHierarchyChanged()`.
5. **Orden de pestañas y estado inicial**:
   - Orden en `ImGuiAppView.cpp`: `Patch Editor` (0), `Patch Effects` (1), `Drums Editor` (2), `Performance Editor` (3), `System Editor` (4).
   - Navegador (`SetLibrarianView`) arranca en `Patches`, banco `USER` (`bankFilter_ = 1`) y primer patch (`selectedIdx_ = 0`).
   - Al elegir `Patches`, `Perfs` o `Drums` en el navegador, cambiar automáticamente al editor correspondiente y actualizar `SoundMode` en el sintetizador (`1` para Patch, `0` para Perfs/Drums).

### PROHIBIDO
- **NO** usar `ImGui::SliderInt`, `ImGui::VSliderInt`, `ImGui::DragInt` ni `ImGui::Checkbox` para controles de parámetros de hardware en los editores.
- **NO** usar UI basada en componentes JUCE para los paneles internos; toda la UI vive en Dear ImGui dentro de `ImGuiHost`.
- **NO** permitir superposición de texto y gráficos en widgets personalizados (ej. `StructureDiagram` debe reservar su barra de título por encima del área de bloques).

---

## 4. Reglas de Protocolo MIDI / SysEx (Roland JV-1010)

### OBLIGATORIO
1. **Pacing de peticiones `RQ1` (`0x11`)**: En `MidiManager::run()`, respetar los tiempos de espera según el tamaño de respuesta `DT1` esperado (`>= 35 ms` para nombres de 12 bytes, `>= 50 ms` para bloques Common, `>= 70 ms` para Tones completos) para no saturar el buffer del JV-1010.
2. **Lectura de nombres User Bank**: `requestUserBankNamesFromHardware()` debe solicitar `0x0000000C` (12 bytes del nombre) por cada User Patch, Performance y Rhythm Set para una transferencia rápida y sin pérdidas.
3. **Cambio de modo y carga de Patch / Performance / Rhythm**:
   - En `selectPatch`, `selectPerformance` y `selectRhythmSet` (`PluginProcessor.cpp`): enviar `DT1(SoundMode)` **solo si el modo actual es distinto al requerido**.
   - Si se cambió de `SoundMode`, esperar **60 ms** antes de enviar `Bank Select (CC0/CC32) + Program Change`.
   - Después del `Program Change`, esperar **140 ms** (usando `selectionRequestToken_` para cancelar peticiones obsoletas) antes de encolar los `RQ1` del buffer temporal (`Common + 4 Tones` o `Common + 16 Parts`).
4. **Importación de archivos `.syx` / `.mid`**: Al importar vía `SyxMidFileIO::importFile` en `TopControlBar.cpp`, recolectar los paquetes `DT1` válidos en `ImportResult::hardwarePackets` y enviarlos automáticamente al sintetizador mediante `sendSysExQueue`.
5. **Expansión `XP-B`**: No sobrescribir los primeros 4 nombres de Patches de `XP-B` con `samplePatchNames` del catálogo; listar los `rhythmCount` Drum Kits de la expansión en la pestaña `Drums` usando `RhythmBank::WaveExp`.

### PROHIBIDO
- **NO** enviar `DT1(SoundMode)` incondicionalmente en cada clic de Patch/Performance justo antes de `Program Change` (provoca que el JV-1010 ignore el `Program Change` mientras reinicia el modo).
- **NO** enviar ráfagas masivas de `RQ1` a intervalos menores a 35–70 ms.
- **NO** asumir que los efectos de Patch están rotos si no se escuchan: recordar verificar `Master EFX/Chorus/Reverb` (`System Common`) y `OutputAssign = EFX (1)` en los Tones activos.
