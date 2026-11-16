# Roland JV-1010 Visual Editor & Librarian (JV-1010 Editor Pro)

**Languages / Idiomas:** [English](#english) · [Español](#español) · [Português](#português) · [Français](#français)

---

<a id="english"></a>
## English

### Project Goal
**JV-1010 Editor Pro** is an open-source, high-density **Visual MIDI/SysEx Editor, Librarian, and Autonomous VST3 / Standalone Application** for the **Roland JV-1010** synthesizer (also compatible with JV-1080, JV-2080, and XP-series `.syx`/`.mid` patch imports). Its goal is to give complete real-time visual control over every hidden parameter of the JV-1010—which on the hardware front panel is limited to a 3-digit LED display—using a photorealistic hardware-inspired **Dear ImGui (OpenGL3)** interface powered by **C++20** and **JUCE 8**.

### Key Features
- **Autonomous Standalone & VST3 Plugin**: Works directly with physical MIDI ports in both Standalone and VST3 modes, or through your DAW MIDI bus, with configurable MIDI Thru/Merge and paced SysEx transmission.
- **All-in-One Patch Editor**:
  - Edit all 4 Tones (`Wave Group`, `Wave #`, `Gain`, `FXM`, `Delay`, `Key/Vel Ranges` with musical note readouts, `Pan`, `Output Assign`, `Mix/Chorus/Reverb Sends`, `Link All Active Tones`, `Copy Tone`).
  - **12-Slot Control Matrix** (`C1-1..C3-4`) in a compact horizontal grid.
  - **Simultaneous Stacked Envelopes** (**Amplifier TVA**, **Filter TVF**, and **Pitch**) with interactive draggable `EnvelopeGraph`s, `VelocityCurvePreview` (`1..7`), and `V-On/V-Off/Time KF` parameters.
  - **Patch Common & Dual LFOs**: `Patch Level` fader, `Portamento` (Switch/Mode/Type/Time/Start), `Peak + Hold` (`EFX/C1/C2/C3`), `Structures 1+2 & 3+4` topology diagrams, and side-by-side `LFO 1 & 2` comparison table.
- **Patch Effects (`40 EFX + Chorus + Reverb/Delay`)**: Dynamic parameter mapping for all 40 Roland EFX algorithms, plus quick `Master EFX / Chorus / Reverb` switches and one-click `Route Active Tones -> EFX`.
- **16-Part Performance Mixer**: Channel strips with `FaderV` level sliders, Pan, Sends, Coarse/Fine Tune, Key Ranges, Voice Reserve (`64` voices total), and direct Part Patch editing.
- **Drums / Rhythm Kit Editor**: 64-note Rhythm Kit inspector (`Tone`, `Wave`, `Mute Group`, `Pitch`, `Filter`, `TVA`, `Env`).
- **System & Microtonal Scale Tuning**: Master Tune, Receive/Control channels, Phrase Preview config, and 12-note Scale Tuning (with `Equal`, `Just`, `Arabian` presets and optional `GS 42H` SysEx).
- **Librarian, Expansion Detection & Hardware Sync**:
  - Browse `USER`, `PR-A..E`, `XP-A (Session)`, and `XP-B` (`SR-JV80-01..19` auto-detection & manual selector, including expansion Drum Kits).
  - Automatic mode switching (`PATCH` / `PERFORM`) and paced `RQ1` sync when selecting Patches, Performances, or Rhythm Kits.
  - Fast User Bank name recovery (`128` Patches, `32` Performances, `2` Rhythm Sets) and full User Bank dump.
  - `.syx` / `.mid` import (with Cross-JV address remapping and automatic transmission to the synth) & export, `Compare A/B`, `Patch Morph / Randomize`, and a **Virtual Keyboard** with `Chord`, `Arp`, `Phrase Preview`, and `Mod Wheel`.

### Build (Windows / MSVC)
```powershell
$env:_CL_="/Z7"
cmake -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --target JV1010Tests JV1010EditorPro_VST3 JV1010EditorPro_Standalone --config Release
.\build\Release\JV1010Tests.exe
```

---

<a id="español"></a>
## Español

### Objetivo del Proyecto
**JV-1010 Editor Pro** es un **Editor Visual MIDI/SysEx, Librero y aplicación Standalone / plugin VST3 autónomo** de código abierto para el sintetizador **Roland JV-1010** (compatible además con importación de patches `.syx`/`.mid` de JV-1080, JV-2080 y serie XP). Su objetivo es ofrecer control visual completo y en tiempo real sobre todos los parámetros internos del JV-1010 mediante una interfaz fotorrealista estilo hardware en **Dear ImGui (OpenGL3)** sobre **C++20** y **JUCE 8**.

### Características Principales
- **Standalone y VST3 Autónomo**: Conexión directa a puertos MIDI físicos tanto en Standalone como en VST3, o mediante el bus MIDI del DAW, con MIDI Thru/Merge y cola SysEx con pacing seguro para el hardware.
- **Patch Editor Todo-en-Uno**:
  - Edición completa de los 4 Tones (`Wave Group`, `Wave #`, `Gain`, `FXM`, `Delay`, rangos de tecla/velocidad con nombres de notas musicales, `Pan`, `Output Assign`, envíos `Mix/Chorus/Reverb`, `Link All Active Tones`, `Copy Tone`).
  - **Control Matrix de 12 ranuras** (`C1-1..C3-4`) en grilla horizontal.
  - **3 Envolventes simultáneas** (**Amplifier TVA**, **Filter TVF** y **Pitch**) con gráficas interactivas arrastrables (`EnvelopeGraph`), curvas visuales de velocidad (`VelocityCurvePreview 1..7`) y parámetros `V-On/V-Off/Time KF`.
  - **Patch Common y LFO 1/2**: `Patch Level`, `Portamento` completo, bloque `Peak + Hold`, diagramas de `Structures 1+2 & 3+4` y tabla comparativa `LFO 1 vs LFO 2`.
- **Patch Effects (`40 algoritmos EFX + Chorus + Reverb/Delay`)**: Parámetros dinámicos para los 40 algoritmos EFX, interruptores rápidos `Master EFX / Chorus / Reverb` y botón `Route Active Tones -> EFX`.
- **Performance Mixer (16 Partes)**: Canales con faders verticales, Pan, Sends, afinación, rangos de teclado, Voice Reserve (`64` voces) y edición directa del Patch de cada parte.
- **Drums Editor**: Inspector de 64 notas para Rhythm Kits (`Wave`, `Mute Group`, `Pitch`, `Filter`, `TVA`).
- **System Editor y Scale Tuning**: Afinación maestra, canales MIDI, Phrase Preview y afinación microtonal de 12 notas (con presets `Equal`, `Just`, `Arabian` y envío `GS 42H` opcional).
- **Librero, Expansión `XP-B` y Sincronización**:
  - Navegador para `USER`, `PR-A..E`, `XP-A (Session)` y `XP-B` (detección de placas `SR-JV80-01..19` y sus Drum Kits).
  - Cambio automático de modo (`PATCH` / `PERFORM`) y lectura `RQ1` automática al seleccionar Patches, Performances o Kits.
  - Recuperación rápida de nombres del banco `USER` (`128` Patches, `32` Performances, `2` Rhythm Sets) y volcado completo.
  - Importación `.syx` / `.mid` (con remapeo Cross-JV y envío automático inmediato al sintetizador) y exportación, `Compare A/B`, `Morph / Randomize` y **Teclado Virtual** con `Chord`, `Arp`, `Phrase Preview` y `Mod Wheel`.

---

<a id="português"></a>
## Português

### Objetivo do Projeto
**JV-1010 Editor Pro** é um **Editor Visual MIDI/SysEx, Bibliotecário e aplicativo Standalone / plugin VST3 autônomo** de código aberto para o sintetizador **Roland JV-1010** (compatível também com importação `.syx`/`.mid` das séries JV-1080, JV-2080 e XP). O objetivo é oferecer controle visual completo em tempo real de todos os parâmetros ocultos do JV-1010 usando uma interface estilo hardware em **Dear ImGui (OpenGL3)** com **C++20** e **JUCE 8**.

### Recursos Principais
- **Standalone e VST3 Autônomo**: Acesso direto a portas MIDI físicas ou roteamento pelo DAW, com MIDI Thru/Merge e fila SysEx temporizada.
- **Patch Editor Completo (Tudo-em-Um)**: Edição dos 4 Tones, Matriz de Controle de 12 slots (`C1-1..C3-4`), 3 envelopes empilhados interativos (`TVA`, `TVF`, `Pitch`), `Velocity Curves (1..7)`, `Portamento`, `Peak + Hold`, diagramas `Structures 1+2 & 3+4` e tabela comparativa `LFO 1 & 2`.
- **Patch Effects (`40 EFX + Chorus + Reverb/Delay`)**: Controles dedicados para os 40 algoritmos EFX, chaves `Master EFX / Chorus / Reverb` e roteamento rápido `Route Active Tones -> EFX`.
- **Performance Mixer (16 Partes) & Drums Editor**: Mixer completo de 16 canais, reserva de vozes e inspetor de 64 notas para Rhythm Kits.
- **System & Scale Tuning**: Configuração global, Phrase Preview e microafinação de 12 notas (`GS 42H` opcional).
- **Bibliotecário & Sincronização**: Suporte a `USER`, `PR-A..E`, `XP-A` e placas de expansão `XP-B (SR-JV80-01..19)`, recuperação de nomes `USER` via `RQ1`, importação `.syx`/`.mid` com envio automático para o sintetizador, `Compare A/B`, `Morph / Randomize` e teclado virtual com `Chord` e `Arp`.

---

<a id="français"></a>
## Français

### Objectif du Projet
**JV-1010 Editor Pro** est un **Éditeur Visuel MIDI/SysEx, Bibliothécaire et application Standalone / plugin VST3 autonome** open-source pour le synthétiseur **Roland JV-1010** (également compatible avec l'import `.syx`/`.mid` des séries JV-1080, JV-2080 et XP). Il permet de contrôler visuellement et en temps réel tous les paramètres internes du JV-1010 grâce à une interface inspirée du matériel Roland en **Dear ImGui (OpenGL3)** avec **C++20** et **JUCE 8**.

### Fonctionnalités Clés
- **Standalone & VST3 Autonome** : Connexion directe aux ports MIDI physiques ou via le bus MIDI du DAW, avec MIDI Thru/Merge et file SysEx cadencée.
- **Éditeur de Patch Tout-en-Un** : Édition des 4 Tones, Matrice de Contrôle à 12 emplacements (`C1-1..C3-4`), 3 enveloppes interactives simultanées (`TVA`, `TVF`, `Pitch`), courbes de vélocité (`1..7`), `Portamento`, `Peak + Hold`, diagrammes `Structures 1+2 & 3+4` et tableau comparatif `LFO 1 & 2`.
- **Effets de Patch (`40 EFX + Chorus + Reverb/Delay`)** : Paramètres dynamiques pour les 40 algorithmes EFX, interrupteurs `Master EFX / Chorus / Reverb` et routage rapide `Route Active Tones -> EFX`.
- **Mixeur Performance (16 Parties) & Éditeur Drums** : Faders de niveau, panoramique, départs d'effets, réserve de voix et inspecteur 64 notes pour les kits rythmiques.
- **Système & Accordage Microtonal** : Paramètres globaux, Phrase Preview et Scale Tuning à 12 notes (`GS 42H` optionnel).
- **Bibliothécaire & Synchronisation Matérielle** : Banques `USER`, `PR-A..E`, `XP-A` et cartes d'extension `XP-B (SR-JV80-01..19)`, récupération rapide des noms `USER` via `RQ1`, import `.syx`/`.mid` avec envoi automatique au synthétiseur, `Compare A/B`, `Morph / Randomize` et clavier virtuel (`Chord`, `Arp`).
