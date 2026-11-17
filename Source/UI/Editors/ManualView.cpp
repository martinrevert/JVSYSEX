#include "ManualView.h"
#include "../ImGuiWidgets.h"
#include <imgui.h>
#include <algorithm>

namespace jv1010 {

namespace {

void drawCallout(const char* tag, const char* body, ImU32 accentCol) {
    ImGui::Spacing();
    float w = std::max(180.0f, ImGui::GetContentRegionAvail().x);
    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.08f, 0.09f, 0.11f, 1.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 4.0f);

    // Measure wrapped height first so child box fits snugly without inner scrolling
    ImVec2 textSz = ImGui::CalcTextSize(body, nullptr, false, std::max(120.0f, w - 26.0f));
    float boxH = ImGui::GetTextLineHeightWithSpacing() + textSz.y + 16.0f;

    ImGui::BeginChild(tag, ImVec2(w, boxH), true, ImGuiWindowFlags_NoScrollbar);
    ImVec2 pMin = ImGui::GetWindowPos();
    ImVec2 pMax(pMin.x + 4.0f, pMin.y + boxH);
    ImGui::GetWindowDrawList()->AddRectFilled(pMin, pMax, accentCol, 2.0f);

    ImGui::SetCursorPosX(12.0f);
    ImGui::TextColored(ImGui::ColorConvertU32ToFloat4(accentCol), "%s", tag);
    ImGui::SetCursorPosX(12.0f);
    ImGui::PushTextWrapPos(ImGui::GetCursorPosX() + std::max(120.0f, w - 24.0f));
    ImGui::TextUnformatted(body);
    ImGui::PopTextWrapPos();
    ImGui::EndChild();

    ImGui::PopStyleVar();
    ImGui::PopStyleColor();
    ImGui::Spacing();
}

void pWrap(const char* text) {
    ImGui::PushTextWrapPos(0.0f);
    ImGui::TextUnformatted(text);
    ImGui::PopTextWrapPos();
    ImGui::Spacing();
}

void bulletItem(const char* boldPrefix, const char* text) {
    ImGui::Bullet();
    ImGui::SameLine();
    ImGui::PushTextWrapPos(0.0f);
    ImGui::TextColored(ImVec4(0.95f, 0.72f, 0.25f, 1.0f), "%s", boldPrefix);
    ImGui::SameLine(0.0f, 4.0f);
    ImGui::TextUnformatted(text);
    ImGui::PopTextWrapPos();
}

void subTitle(const char* title) {
    ImGui::Spacing();
    ImGui::TextColored(ImVec4(0.38f, 0.74f, 0.98f, 1.0f), "%s", title);
    ImGui::Separator();
}

} // namespace

void ManualView::render() {
    // Top bar inside Manual tab: Title + Language toggle (ES / EN)
    ImGui::AlignTextToFramePadding();
    ImGui::TextColored(ImVec4(0.95f, 0.72f, 0.25f, 1.0f),
                       spanish_ ? "ROLAND JV-1010 - MANUAL INTEGRADO DE USUARIO Y SINTESIS"
                                : "ROLAND JV-1010 - INTEGRATED USER & SYNTHESIS MANUAL");
    ImGui::SameLine(std::max(380.0f, ImGui::GetContentRegionAvail().x - 165.0f));
    if (ui::RolandButton("Espanol", spanish_, ImVec2(76.0f, 24.0f), spanish_ ? 1 : 3)) {
        spanish_ = true;
    }
    ImGui::SameLine();
    if (ui::RolandButton("English", !spanish_, ImVec2(76.0f, 24.0f), !spanish_ ? 1 : 3)) {
        spanish_ = false;
    }
    ImGui::Separator();

    static const char* kSectionsEs[] = {
        "01. Principiantes & Hardware",
        "02. Barra Superior (Top Bar)",
        "03. Navegador & Librero",
        "04. Patch Editor: Tone & Env",
        "05. Patch Editor: Common & LFO",
        "06. Patch Effects (EFX/Cho/Rev)",
        "07. Drums Editor (Rhythm Kits)",
        "08. Performance Editor (16 Part)",
        "09. System Editor & Scale Tune",
        "10. Footer & Tips Avanzados SOS"
    };

    static const char* kSectionsEn[] = {
        "01. Beginners & Hardware",
        "02. Top Control Bar",
        "03. Browser & Librarian",
        "04. Patch Editor: Tone & Env",
        "05. Patch Editor: Common & LFO",
        "06. Patch Effects (EFX/Cho/Rev)",
        "07. Drums Editor (Rhythm Kits)",
        "08. Performance Editor (16 Part)",
        "09. System Editor & Scale Tune",
        "10. Footer & SOS Power Tips"
    };

    const char* const* sections = spanish_ ? kSectionsEs : kSectionsEn;

    // Left Navigation Sidebar
    ImGui::BeginChild("##ManualTOC", ImVec2(238.0f, 0.0f), true);
    ui::SectionHeader(spanish_ ? "SECCIONES DEL MANUAL" : "MANUAL SECTIONS", IM_COL32(36, 98, 178, 255));
    ImGui::Spacing();
    for (int i = 0; i < 10; ++i) {
        ImGui::PushID(i);
        if (ui::RolandButton(sections[i], selectedSection_ == i, ImVec2(-1.0f, 28.0f), selectedSection_ == i ? 1 : 3)) {
            selectedSection_ = i;
        }
        ImGui::PopID();
    }
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::PushTextWrapPos(0.0f);
    ImGui::TextColored(ImVec4(0.62f, 0.66f, 0.72f, 1.0f), "%s",
                       spanish_ ? "Basado en el Owner's Manual oficial Roland JV-1010 (1999) y la serie de 4 partes de Nick Magnus (Sound On Sound)."
                                : "Based on the official Roland JV-1010 Owner's Manual (1999) and Nick Magnus's 4-part Sound On Sound series.");
    ImGui::PopTextWrapPos();
    ImGui::EndChild();

    ImGui::SameLine();

    // Right Content Pane
    ImGui::BeginChild("##ManualContent", ImVec2(0.0f, 0.0f), true);
    ui::SectionHeader(sections[selectedSection_], IM_COL32(195, 118, 24, 255));
    ImGui::Spacing();

    switch (selectedSection_) {
    // =========================================================================
    // 01. BEGINNERS & HARDWARE JV-1010
    // =========================================================================
    case 0:
        if (spanish_) {
            subTitle("Que es el Roland JV-1010 y por que necesitas esta app");
            pWrap("El Roland JV-1010 es un modulo sintetizador multitimbrico de 16 partes y 64 voces de polifonia (motor Sample & Synthesis derivado del JV-1080/2080) con la placa SR-JV80-09 'Session' integrada de fabrica y 1 bahia de expansion (EXP-B) para tarjetas SR-JV80. En total ofrece 1,023 patches internos (USER, Preset A-E y Session) y 40 algoritmos multi-efecto (EFX) mas Chorus y Reverb.");
            pWrap("En su panel fisico frontal solo tiene un visor LED de 3 digitos pensado para elegir sonidos, por lo que practicamente todos los parametros profundos (filtros, envolventes, matriz de modulacion, estructuras, EFX) solo pueden editarse por MIDI SysEx (Model ID 6AH). Esta app reemplaza al antiguo SoundDiver y te da control visual inmediato en tiempo real sobre cada byte del sintetizador.");

            subTitle("Jerarquia sonora para principiantes (de menor a mayor)");
            bulletItem("1. Wave (Forma de onda):", "Es la muestra PCM base guardada en ROM (ej. un piano, cuerda, onda saw o hit de bateria). Los grupos son INT-A (255 ondas), INT-B (193 ondas), XP-A Session (206 ondas) y XP-B (tarjeta instalada).");
            bulletItem("2. Tone (Tono):", "Unidad basica de sintesis. Toma 1 Wave y la pasa por Pitch, Filter (TVF), Amplifier (TVA), 2 LFOs y Control Matrix. Cada Patch combina hasta 4 Tones (T1..T4).");
            bulletItem("3. Patch (Sonido):", "Combina 1 a 4 Tones, define como se interconectan (Structures 1+2 y 3+4), agrega Portamento, Analog Feel y sus propios ajustes de EFX, Chorus y Reverb. Es el modo ideal para tocar un instrumento solo.");
            bulletItem("4. Rhythm Set (Kit de Bateria):", "Un mapa especial donde cada tecla MIDI (notas 35 a 98, B1..D7) dispara una onda de percusion independiente con su propio filtro, nivel, paneo, envolventes y Mute Group.");
            bulletItem("5. Performance (Multitimbrico 16 Partes):", "Funciona como una orquesta de 16 musicos (Part 1 a 16). Las Partes 1..9 y 11..16 cargan Patches, y la Part 10 esta reservada para el Rhythm Set. Puede funcionar en modo LAYER (apilar sonidos en el mismo canal) o SINGLE (cada parte responde a su canal MIDI 1..16 en un secuenciador/DAW).");
            bulletItem("6. GM Mode (General MIDI):", "Modo compatible con partituras/archivos MIDI estandar GM. Al activarlo inicializa Part 10 como GM Drum Set y el resto como Piano 1. Nota: el JV-1010 no soporta edicion SysEx 6AH en modo GM ni archivos Roland GS completos (salvo Scale Tune GS 42H).");

            subTitle("Conexiones y controles del panel fisico (Manual Original)");
            bulletItem("Panel trasero - Switch COMPUTER:", "MUY IMPORTANTE: si conectas el JV-1010 mediante cables MIDI DIN (IN/OUT/THRU) a una interfaz MIDI USB/audio, este switch trasero debe estar en posicion MIDI (con el equipo apagado antes de moverlo!). Las posiciones Mac / PC-1 / PC-2 solo son para el cable serial mini-DIN legacy.");
            bulletItem("Audio OUTPUT L(MONO) / R y PHONES:", "Conecta salida estereo L/R a tu interfaz/mixer/monitores (o solo L para mono). Encendido seguro: baja los volumenes, enciende primero el JV-1010 (aparece 'roland Jv-1010' y si hay expansion instalada parpadea 2 veces su numero de 2 digitos) y luego tus monitores. Al apagar: monitores primero, JV-1010 despues.");
            bulletItem("Panel frontal:", "VOLUME ajusta salida y al presionarlo dispara 'Phrase Preview'. PART elige canal de recepcion (en Patch) o Parte 1..16 (en Perform). VALUE cambia valores y al presionarlo cicla entre los modos PATCH / PERFORM / RHYTHM / GM. CATEGORY/BANK elige banco (letras blancas) o categoria (letras azules).");

            if (ImGui::BeginTable("##ErrTblEs", 2, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingStretchProp)) {
                ImGui::TableSetupColumn("Codigo LED", ImGuiTableColumnFlags_WidthFixed, 130.0f);
                ImGui::TableSetupColumn("Significado y solucion (Manual Original p.53)");
                ImGui::TableHeadersRow();
                const char* rows[][2] = {
                    {"coE", "MIDI Communication Error: cable MIDI suelto/cortado o dispositivo externo apagado."},
                    {"bFL", "MIDI Buffer Full: llegaron demasiados mensajes MIDI/SysEx juntos (la app ya usa pacing automatico)."},
                    {"cSE / rdE", "Check Sum / Receive Data Error: paquete SysEx corrupto o checksum invalido."},
                    {"noP", "No Patch: se pidio un Patch/Kit de XP-B sin tener tarjeta de expansion instalada."},
                    {"und", "User Memory Damaged: memoria USER danada; realizar Factory Reset (Edit -> PIANO 'Fct')."},
                    {"btL", "Battery Low: pila interna CR2032 agotada; respalda USER en .syx y reemplazala."}
                };
                for (auto& r : rows) {
                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0); ImGui::TextUnformatted(r[0]);
                    ImGui::TableSetColumnIndex(1); ImGui::TextWrapped("%s", r[1]);
                }
                ImGui::EndTable();
            }

            drawCallout("CONSEJO RAPIDO PARA PRINCIPIANTES",
                        "Como el JV-1010 tiene 128 slots USER reescribibles y bancos de fabrica protegidos (PR-A..E y Session), cualquier cambio que hagas en la app se envia al 'Buffer Temporal' del hardware. Si apagas el equipo sin presionar 'Write User', los cambios se pierden y vuelve el sonido original.",
                        IM_COL32(36, 148, 98, 255));
        } else {
            subTitle("What the Roland JV-1010 Is and Why This App Matters");
            pWrap("The Roland JV-1010 is a 16-part multitimbral, 64-voice Sample & Synthesis (S&S) sound module based on the JV-1080/2080 engine, with the SR-JV80-09 'Session' board built in plus 1 expansion slot (EXP-B) for SR-JV80 boards. It includes 1,023 onboard patches (USER, Preset A-E, Session) and a 40-algorithm Multi-Effects (EFX) processor plus dedicated Chorus and Reverb.");
            pWrap("Because the hardware front panel only provides a 3-digit LED display, deep sound design (filters, envelopes, modulation matrix, structures, EFX) is hidden behind MIDI System Exclusive (Model ID 6AH). This editor gives you direct real-time visual access to every internal parameter.");

            subTitle("Sound Hierarchy for Beginners (Bottom to Top)");
            bulletItem("1. Wave (Waveform):", "Raw PCM sample stored in ROM: INT-A (255), INT-B (193), XP-A Session (206), and optional XP-B expansion board.");
            bulletItem("2. Tone:", "Core synth voice taking 1 Wave through Pitch, TVF Filter, TVA Amplifier, 2 LFOs, and Control Matrix. A Patch stacks up to 4 Tones (T1..T4).");
            bulletItem("3. Patch:", "Combines 1-4 Tones, defines routing (Structures 1+2 and 3+4), Portamento, Analog Feel, and Patch EFX/Chorus/Reverb. Best for playing a single instrument.");
            bulletItem("4. Rhythm Set (Drum Kit):", "Maps MIDI keys 35..98 (B1..D7) to individual percussion waves with independent Pitch, Filter, Amp, Pan, Envelopes, and Mute Groups.");
            bulletItem("5. Performance (16-Part Multitimbral):", "An ensemble of 16 Parts (Parts 1..9 & 11..16 host Patches; Part 10 is fixed to Rhythm Set). Can run in LAYER (stacked keyboard) or SINGLE (separate MIDI channels 1..16 for DAW sequencing).");
            bulletItem("6. GM Mode (General MIDI):", "Initializes Part 10 to GM Drum Set and other parts to Piano 1 for standard GM playback. Note: SysEx 6AH editing is disabled while the unit is in GM mode.");

            subTitle("Hardware Connections & Panel Controls (Owner's Manual)");
            bulletItem("Rear COMPUTER Switch:", "CRITICAL: When using standard 5-pin DIN MIDI cables (IN/OUT/THRU) with a USB MIDI interface, set this switch to MIDI (always power off before changing!). Mac/PC-1/PC-2 are only for legacy serial cables.");
            bulletItem("Audio OUTPUT L(MONO)/R & PHONES:", "Power on JV-1010 first (watches boot message + 2-digit EXP board flash), then amp/monitors. Power off monitors first, then JV-1010.");
            bulletItem("Front Knobs:", "VOLUME adjusts level and triggers Phrase Preview when pushed. PART selects MIDI Rx Channel in Patch mode or Part 1..16 in Perform mode. VALUE changes numbers and cycles modes (PATCH/PERFORM/RHYTHM/GM) when pushed. CATEGORY/BANK selects banks (white text) or categories (blue text).");

            if (ImGui::BeginTable("##ErrTblEn", 2, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingStretchProp)) {
                ImGui::TableSetupColumn("LED Code", ImGuiTableColumnFlags_WidthFixed, 130.0f);
                ImGui::TableSetupColumn("Meaning & Action (Owner's Manual p.53)");
                ImGui::TableHeadersRow();
                const char* rows[][2] = {
                    {"coE", "MIDI Communication Error: loose/shorted MIDI cable or connected external unit powered off."},
                    {"bFL", "MIDI Buffer Full: MIDI flood (the app automatically paces SysEx queues to avoid this)."},
                    {"cSE / rdE", "Check Sum / Receive Data Error: malformed SysEx checksum or corrupted message."},
                    {"noP", "No Patch: XP-B bank selected with no SR-JV80 expansion board installed."},
                    {"und", "User Memory Damaged: perform Factory Reset (Edit mode -> PIANO 'Fct')."},
                    {"btL", "Battery Low: internal CR2032 memory battery depleted; back up USER bank and replace."}
                };
                for (auto& r : rows) {
                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0); ImGui::TextUnformatted(r[0]);
                    ImGui::TableSetColumnIndex(1); ImGui::TextWrapped("%s", r[1]);
                }
                ImGui::EndTable();
            }

            drawCallout("BEGINNER TIP: TEMPORARY BUFFER",
                        "All edits you make in the app modify the JV-1010's RAM Temporary Buffer so you hear changes immediately without overwriting presets. Use 'Write User' in the Browser (or Export .syx) before turning the hardware off!",
                        IM_COL32(36, 148, 98, 255));
        }
        break;

    // =========================================================================
    // 02. TOP CONTROL BAR
    // =========================================================================
    case 1:
        if (spanish_) {
            subTitle("Que hace la Barra Superior (Top Control Bar)");
            pWrap("La barra superior controla la conexion MIDI bidireccional, el enrutamiento con tu DAW, la importacion/exportacion de bancos y las herramientas globales de comparacion y visibilidad.");

            bulletItem("MIDI In / MIDI Out:", "Elige los puertos fisicos conectados al JV-1010. Al abrir ambos puertos, la app envia automaticamente peticiones RQ1 al hardware para sincronizar el Patch/Performance activo y reflejar en pantalla exactamente lo que suena en el equipo.");
            bulletItem("Route (Direct HW / DAW Bus / HW + DAW):", "Define hacia donde salen los mensajes MIDI y SysEx generados por la app. Direct HW envia directo al puerto MIDI fisico (ideal en Standalone); DAW Bus envia al bus del DAW en VST3; HW + DAW envia a ambos.");
            bulletItem("SysEx Queue / Barra de estado:", "Cuando ves 'SysEx Queue: N...' en ambar, significa que la app esta enviando paquetes DT1/RQ1 con pausas seguras (35-70 ms) para no saturar el buffer del JV-1010 ('bFL'). En verde indica la ultima accion completada.");
            bulletItem("Import .syx / .mid:", "Carga archivos SysEx o Standard MIDI File (.mid) de Roland JV-1010, JV-1080, JV-2080 o serie XP. Remapea direcciones compatibles y envia automaticamente los paquetes DT1 al sintetizador para que los escuches al instante.");
            bulletItem("Export .syx / .mid:", "Abre el menu de respaldo para exportar en formato .syx o .mid: Temp Patch, Temp Performance, los 128 User Patches, las 32 User Performances o el Full User Memory Backup completo.");
            bulletItem("Compare A/B:", "Alterna con un clic entre una copia original del Patch ('Snapshot Original') y tu edicion actual ('Edicion Actual'), enviando el estado correspondiente al hardware para evaluar si tu cambio mejoro el sonido.");
            bulletItem("Keyboard / MIDI Thru / Morph/Rnd:", "Keyboard muestra u oculta el teclado virtual inferior. MIDI Thru reenvia notas y controladores recibidos por MIDI In hacia MIDI Out (util para tocar el sinte desde un controlador conectado a la PC). Morph/Rnd muestra la barra de interpolacion A<->B y aleatorizacion en el footer.");
        } else {
            subTitle("What the Top Control Bar Does");
            pWrap("The top bar manages bidirectional MIDI ports, DAW vs. hardware routing, SysEx file I/O, A/B comparison, and dock visibility.");

            bulletItem("MIDI In / MIDI Out:", "Selects the physical MIDI ports connected to your JV-1010. As soon as both In and Out are open, the app automatically sends RQ1 requests to pull the current Temporary state from the hardware.");
            bulletItem("Route (Direct HW / DAW Bus / HW + DAW):", "Controls where generated MIDI/SysEx is routed: directly to the hardware port, to the host DAW bus (in VST3 mode), or both.");
            bulletItem("SysEx Queue / Status:", "Amber 'SysEx Queue: N...' shows paced transmission (35-70 ms intervals) to prevent hardware buffer overflow ('bFL'). Green displays the latest status.");
            bulletItem("Import .syx / .mid:", "Imports Roland JV-1010 / JV-1080 / JV-2080 / XP SysEx or MIDI files, remaps addresses if needed, and streams DT1 packets straight to the synth.");
            bulletItem("Export .syx / .mid:", "Exports Temp Patch, Temp Performance, User Patches 1-128, User Performances 1-32, or a Full User Memory Backup.");
            bulletItem("Compare A/B:", "Toggles between the original snapshot and your current edits, updating the hardware Temporary Buffer immediately.");
            bulletItem("Keyboard / MIDI Thru / Morph/Rnd:", "Toggles the Virtual Keyboard dock, enables MIDI Thru from MIDI In to MIDI Out, and shows/hides the Morph/Randomize toolbar.");
        }
        break;

    // =========================================================================
    // 03. BROWSER & LIBRARIAN
    // =========================================================================
    case 2:
        if (spanish_) {
            subTitle("Navegador de Bancos y Librero (Panel Izquierdo)");
            pWrap("Permite explorar los 1,023+ sonidos internos y de expansion, cambiar el modo del sintetizador automaticamente y administrar la memoria USER del JV-1010.");

            bulletItem("Patches / Perfs / Drums:", "Al elegir cualquiera de los tres, la app envia SoundMode al JV-1010 (1 = PATCH para Patches; 0 = PERFORM para Perfs y Drums) y abre la pestana del editor correspondiente.");
            bulletItem("Filtro de Banco (All Banks / USER / PR-A..E / XP-A / XP-B):", "Filtra por grupo de memoria. USER tiene 128 Patches, 32 Perfs y 2 Rhythm Sets editables. PR-A..E son bancos de fabrica de solo lectura (PR-D es General MIDI). XP-A es la expansion Session integrada. XP-B muestra los sonidos de la placa SR-JV80 instalada.");
            bulletItem("Filtro de Categoria y Buscador:", "Filtra Patches entre las 39 categorias de Roland (AC.PIANO, SYNTH PAD, BASS, etc.) o busca por nombre en tiempo real.");
            bulletItem("Clic en un sonido de la lista:", "Envia Bank Select (CC0 MSB + CC32 LSB) + Program Change al JV-1010, espera 140 ms a que el hardware cargue el sonido y luego pide sus parametros reales via RQ1 para poblar los editores.");
            bulletItem("Send PC:", "Reenvia Bank Select + Program Change para el item seleccionado.");
            bulletItem("Send SysEx:", "Si elegiste un item USER, envia su bloque DT1 completo desde la memoria de la app directo al buffer temporal del hardware (util para audicionar cambios locales sin escribir en memoria).");
            bulletItem("Write User:", "Guarda permanentemente el Patch, Performance o Rhythm Kit que estas editando en el slot USER seleccionado dentro de la memoria interna del JV-1010.");
            bulletItem("Sync RQ1 (visible en banco USER):", "Consulta al hardware el estado actual del buffer temporal (Common + 4 Tones o Common + 16 Parts) si editaste algo desde otro dispositivo.");
            bulletItem("Pull User Bank (visible en banco USER):", "Ofrece dos opciones: 'Pull User Names' (descarga rapida de 12 bytes por slot para ver los nombres reales guardados en tu JV-1010) y 'Pull Full User Bank' (descarga todos los parametros de los 128 User Patches, 32 Perfs y 2 Kits para respaldo total).");
            bulletItem("Selector EXP-B (abajo):", "Permite elegir manualmente que tarjeta SR-JV80-01..19 tenes instalada en el slot EXP-B (si la app no la auto-detecto) para listar correctamente sus ondas, patches y drum kits. La seleccion queda guardada entre sesiones.");
        } else {
            subTitle("Bank Browser & Librarian (Left Panel)");
            pWrap("Explores all 1,023+ factory and expansion sounds, switches the hardware Sound Mode automatically, and manages JV-1010 USER memory.");

            bulletItem("Patches / Perfs / Drums:", "Switches hardware SoundMode (1 = PATCH, 0 = PERFORM for Perfs and Drums) and opens the matching editor tab.");
            bulletItem("Bank Filter:", "USER (128 Patches, 32 Perfs, 2 Kits rewritable), PR-A..E (ROM presets; PR-D is GM), XP-A (onboard Session board), and XP-B (optional SR-JV80 card).");
            bulletItem("Category & Search:", "Filters Patches across all 39 Roland categories or searches by name.");
            bulletItem("Selecting a Row:", "Sends Bank Select (CC0/CC32) + Program Change, waits 140 ms for hardware mode/load settling, then queries the Temporary Buffer via RQ1.");
            bulletItem("Send PC / Send SysEx:", "Send PC re-triggers Bank Select + Program Change; Send SysEx pushes the stored USER patch/perf directly into the hardware Temporary Buffer via DT1.");
            bulletItem("Write User:", "Permanently writes the current Temporary Buffer into the selected USER slot inside the JV-1010's battery-backed RAM.");
            bulletItem("Sync RQ1 & Pull User Bank (USER bank):", "Sync RQ1 refreshes the active Temporary Buffer from hardware. Pull User Names quickly fetches 12-byte slot names; Pull Full User Bank downloads every parameter of all USER patches, performances, and kits.");
            bulletItem("EXP-B Selector:", "Persisted selector for your installed SR-JV80-01..19 expansion board so waveforms, patches, and drum kits match your hardware.");
        }
        break;

    // =========================================================================
    // 04. PATCH EDITOR LEFT (TONE, MATRIX, TVA, TVF, PITCH)
    // =========================================================================
    case 3:
        if (spanish_) {
            subTitle("Barra de Destino y Tones (T1..T4)");
            bulletItem("Patch Mode Temp vs Perf Part 1..16:", "Define si editas el Patch solitario (03 00 00 00) o el Patch asignado a una Parte dentro de la Performance activa (02 xx 00 00), sin pisar la memoria USER.");
            bulletItem("Name / Category / Tempo / Clk:", "Cambia el nombre de 12 caracteres, categoria, tempo propio del Patch (20-250 BPM) y si los delays/LFOs siguen el tempo del Patch o del System.");
            bulletItem("T1..T4, Link y Cp T1->T2:", "Cada boton pequeno arriba de T1..T4 enciende o apaga ese Tone en el JV-1010 (Tone Switch). T1..T4 elige cual estas viendo abajo. 'Link' replica cada movimiento a todos los Tones activos a la vez; 'Cp T1->T2' clona el Tone actual al siguiente.");

            subTitle("TONE 1..4 | WAVE, RANGE, OUTPUT & FXM");
            bulletItem("WaveGrp / Wave / Gain:", "Selecciona la forma de onda PCM del Tone activo y su ganancia previa (-6, 0, +6, +12 dB). Ojo: +12 dB puede saturar filtros o boosters.");
            bulletItem("Out (MIX / EFX / DIR1 / DIR2):", "Decide si este Tone va limpio a la salida principal (MIX) o entra al procesador multi-efecto (EFX). En el JV-1010 fisico DIR1/DIR2 no tienen salidas fisicas extra, asi que usa MIX o EFX.");
            bulletItem("Switches Vol / Bend / Hold1 / ReDmp:", "Activa o ignora CC#7 Volume, Pitch Bend, pedal Sustain (Hold1) y Redamper por cada Tone (ideal para que un pad ignore Pitch Bend mientras un lead si lo usa).");
            bulletItem("FXM (Switch / Color 1..4 / Depth 1..16):", "Frequency Cross Modulation: modula la frecuencia de la onda con alta velocidad creando texturas metalicas, campanas FM, distorsion o ruido tipo ring-mod.");
            bulletItem("Tone Delay (Mode + DlyT):", "Retrasa el disparo del Tone al pulsar la tecla (NORMAL, HOLD, PLAYMATE, CLK-SYNC, TAP-SYNC, KEY-OFF-N/D para disparar al soltar la tecla, TMP-SYNC).");
            bulletItem("KeyLo / KeyHi & VelLo / VelHi / XFade:", "Splits de teclado por nota (C-1..G9) y capas de velocidad (1..127) con fundido cruzado suave (XFade). Requiere 'VelSw' activo en Patch Common.");
            bulletItem("PanCtrl / Pan / PanKF / RndPan / AltPan / Sends:", "Controla el paneo fijo, seguimiento de teclado (PanKF), paneo aleatorio por nota (RndPan), paneo alternado L/R en cada nota (AltPan) y envios individuales a Mix/EFX, Chorus y Reverb.");

            subTitle("CONTROL MATRIX (C1 / C2 / C3)");
            pWrap("Permite enrutar en tiempo real 3 fuentes de modulacion (Ctrl 1 = Modulacion/Sys segun origen, Ctrl 2 Src y Ctrl 3 Src seleccionables entre Mod Wheel, Aftertouch, Velocity, Keyfollow, Breath, LFO1, LFO2, etc.) hacia hasta 4 destinos por controlador (12 ranuras C1-1..C3-4) con profundidad bipolar -63..+63: PCH (Pitch), CUT (Cutoff), RES (Resonance), LEV (Level), PAN, MIX, CHO, REV, PL1/2, FL1/2, AL1/2, pL1/2 (profundidades LFO) y L1R/L2R (velocidad de LFO1/2).");

            subTitle("AMPLIFIER (TVA & BIAS), FILTER (TVF) y PITCH & ENVELOPE");
            bulletItem("AMPLIFIER (TVA & BIAS):", "Controla el volumen en el tiempo. Level es el nivel del Tone; Bias Pos/Lvl/Dir atenua o resalta volumen segun si tocas notas graves o agudas respecto a Bias Pos. La grafica TVA Level Envelope (4 tiempos T1..T4, 3 niveles L1..L3, L4 siempre 0) moldea ataque, decaimiento, sustain y release arrastrando los puntos.");
            bulletItem("FILTER (TVF):", "Esculpe el brillo y cuerpo armonico. Tipos: OFF, LPF (pasa-bajos, el clasico calido), BPF (pasa-banda nasal), HPF (pasa-altos fino) y PKG (realce peaking). Cutoff fija la frecuencia de corte; Resonance enfatiza armonicos alrededor del corte; Cutoff KF abre o cierra el filtro segun la altura de la nota; Env Depth (-63..+63) controla cuanto mueve el filtro la envolvente TVF de 4 etapas (T1..T4, L1..L4).");
            bulletItem("PITCH & ENVELOPE:", "Ajusta afinacion por semitonos (Coarse -48..+48), centesimas (Fine -50..+50), aleatoriedad por nota (Rnd Pitch 0..30), seguimiento de teclado (Pitch KF: 10 = escala cromatica normal +100%, 0 = nota fija para percusiones/FX) y la envolvente bipolar de afinacion (-12..+12 semitonos).");
        } else {
            subTitle("Target Bar & Tone Selectors (T1..T4)");
            bulletItem("Patch Mode Temp vs Perf Part 1..16:", "Chooses whether you edit the standalone Patch (03 00 00 00) or an individual Part's Patch inside the active Performance (02 xx 00 00) without touching USER presets.");
            bulletItem("T1..T4, Link & Cp T1->T2:", "Small toggle switches enable/disable each Tone; T1..T4 selects the inspected Tone. 'Link' applies changes across all active Tones simultaneously; 'Cp T1->T2' copies the active Tone to the next.");

            subTitle("TONE 1..4 | WAVE, RANGE, OUTPUT & FXM");
            bulletItem("WaveGrp / Wave / Gain / Out:", "Selects PCM source waveform, pre-filter gain (-6..+12 dB), and output bus (MIX = bypass EFX, EFX = routed through Multi-Effects).");
            bulletItem("MIDI Switches (Vol/Bend/Hold1/ReDmp):", "Enables/ignores CC#7, Pitch Bend, Sustain pedal, and Redamper per Tone.");
            bulletItem("FXM (Switch/Color/Depth):", "Frequency Cross Modulation for metallic, bell-like, or gritty ring-modulated harmonics.");
            bulletItem("Tone Delay & Key/Vel Ranges:", "Delays Tone onset (including CLK-SYNC and KEY-OFF release triggers), sets note splits (KeyLo/KeyHi), and velocity layers with smooth XFade (requires VelSw ON in Patch Common).");

            subTitle("CONTROL MATRIX (C1 / C2 / C3)");
            pWrap("Maps 3 controller sources (Ctrl 2/3 Src selectable from Mod Wheel, Aftertouch, Velocity, Keyfollow, LFO1/2, Breath, etc.) to 12 destinations (C1-1..C3-4) with -63..+63 depth: PCH, CUT, RES, LEV, PAN, MIX, CHO, REV, and LFO depths/rates (PL1..L2R).");

            subTitle("AMPLIFIER (TVA & BIAS), FILTER (TVF) & PITCH ENVELOPE");
            bulletItem("TVA & Bias:", "Shapes Tone amplitude over 4 stages (T1..T4, L1..L3; drag points directly on the graph). Keyboard Bias adjusts level above/below Bias Position.");
            bulletItem("TVF Filter:", "Types OFF, LPF, BPF, HPF, PKG. Cutoff sets brightness, Resonance adds harmonic peak, Cutoff KF tracks keyboard pitch, and Filter Envelope shapes cutoff dynamically.");
            bulletItem("Pitch & Envelope:", "Coarse (-48..+48 st), Fine (-50..+50 cents), Random Pitch Depth, Pitch Keyfollow (10 = standard chromatic tracking), and bipolar 4-stage Pitch Envelope.");
        }
        break;

    // =========================================================================
    // 05. PATCH EDITOR RIGHT (COMMON, PORTAMENTO, STRUCTURES, LFOS)
    // =========================================================================
    case 4:
        if (spanish_) {
            subTitle("PATCH COMMON (Parametros globales del Patch)");
            bulletItem("Level / Pan / Oct / Bnd+ / Bnd-:", "Volumen general del Patch (0..127), paneo maestro, transposicion por octavas (-3..+3) y rango de Pitch Bend independiente hacia arriba (0..12 st) y hacia abajo (0..-48 st, ideal para dive-bombs).");
            bulletItem("Anlg (Analog Feel 0..127):", "Agrega leve deriva de afinacion aleatoria entre voces simulando osciladores analogicos antiguos.");
            bulletItem("KeyAsgn (POLY / SOLO) & Priority (LAST / LOUDEST):", "POLY permite acordes; SOLO vuelve monofonico al Patch (ideal para bajos y leads). Priority decide que nota se roba si excedes las 64 voces (LAST = conserva la mas nueva; LOUDEST = conserva las mas fuertes).");
            bulletItem("Stretch (OFF, 1..3):", "Afinacion estirada tipo piano acustico (agudos ligeramente mas altos y graves ligeramente mas bajos para mayor realismo en cuerdas/pianos).");
            bulletItem("Legato & VelSw:", "Legato (en modo SOLO) evita redisparar las envolventes cuando ligas notas. VelSw habilita los rangos VelLo/VelHi configurados en cada Tone.");

            subTitle("PORTAMENTO & PEAK + HOLD");
            bulletItem("PORTAMENTO:", "Porta On/Off activa el deslizamiento suave entre notas. Mode: NORMAL (desliza siempre) vs LEGATO (solo desliza al ligar notas). Type: RATE (velocidad proporcional al intervalo) vs TIME (tiempo constante sin importar la distancia). Start: PITCH vs NOTE.");
            bulletItem("PEAK + HOLD:", "Para EFX, Ctrl 1, Ctrl 2 y Ctrl 3 permite elegir OFF, HOLD (retiene el valor al soltar/sostener), PEAK o PEAK+HOLD (captura el pico maximo alcanzado, por ejemplo de velocidad o aftertouch).");

            subTitle("STRUCTURES 1+2 & 3+4 (Topologia y Booster)");
            pWrap("El diagrama muestra en tiempo real como se conectan las parejas de Tones (1 con 2, y 3 con 4) a traves de sus TVF, TVA y etapas opcionales de Booster (0, +6, +12, +18 dB) o Ring Modulator (estructuras 1 a 10):");
            bulletItem("Structure 1 (Paralelo):", "Cada Tone pasa por su propio TVF -> TVA de forma independiente y luego se suman. Es el modo estandar para apilar capas limpias.");
            bulletItem("Structures 2, 5..10 (Ring / Cross / Combinadas):", "Combinan salidas de WG1 con WG2 mediante Ring Modulation o rutas mixtas para sonidos metalicos, campanillas y texturas sinteticas complejas.");
            bulletItem("Structures 3 y 4 (Serie con Booster):", "WG1 y WG2 se mezclan o atraviesan filtros y un Booster (+0/+6/+12/+18 dB) en serie antes del segundo filtro/amplificador, generando saturacion armonica calida y gruesa.");

            subTitle("LFO 1 & LFO 2 (ACTIVE TONE)");
            pWrap("Tabla comparativa de los dos osciladores de baja frecuencia independientes de cada Tone:");
            bulletItem("Form:", "TRI (triangulo), SIN (senoidal), SAW (sierra), SQR (cuadrada), TRP (trapezoide), S&H (Sample & Hold escalonado), RND (aleatorio suave), CHS (caotico).");
            bulletItem("Offset / Key Trig / Rate / Delay / Fade / Extn Sync:", "Offset desplaza la polaridad (-100..+100). Key Trig reinicia el ciclo con cada nota. Delay retrasa el inicio; Fade Time/Mode hace entrar o salir gradualmente la modulacion. Extn Sync sincroniza con Clock MIDI o Tempo.");
            bulletItem("Profundidades directas:", "P-Depth (vibrato de Pitch), TVF Depth (wah/barrido de filtro), Pan Depth (auto-panning) y TVA Depth (tremolo de volumen).");
        } else {
            subTitle("PATCH COMMON");
            bulletItem("Level / Pan / Oct / Bnd+ / Bnd-:", "Overall Patch level, pan, octave transpose (-3..+3), and asymmetric Pitch Bend ranges (+12 / -48 semitones).");
            bulletItem("Anlg (Analog Feel):", "Adds gentle per-voice pitch drift simulating vintage analog oscillators.");
            bulletItem("KeyAsgn / Priority / Stretch / Legato / VelSw:", "POLY vs SOLO monophonic lead mode, voice-stealing priority (LAST/LOUDEST), acoustic piano Stretch Tuning (1..3), Solo Legato (no env re-trigger), and Velocity Range master switch (VelSw).");

            subTitle("PORTAMENTO & PEAK + HOLD");
            bulletItem("PORTAMENTO:", "Glide switch, Time (0..127), Mode (NORMAL vs LEGATO), Type (RATE vs constant TIME), and Start (PITCH vs NOTE).");
            bulletItem("PEAK + HOLD:", "Sets OFF, HOLD, PEAK, or PEAK+HOLD capture behavior for EFX and Matrix Controllers 1..3.");

            subTitle("STRUCTURES 1+2 & 3+4 (Topology & Booster)");
            pWrap("Live block diagram showing how Tone pairs 1&2 and 3&4 route through TVF filters, TVA amps, Ring Modulators, and Booster stages (0, +6, +12, +18 dB) across Structures 1..10:");
            bulletItem("Structure 1:", "Standard parallel routing (WG1->TVF->TVA + WG2->TVF->TVA).");
            bulletItem("Structures 3 & 4 (Booster):", "Routes waveforms through an internal gain/overdrive Booster stage into filters for thick harmonic saturation.");
            bulletItem("Structures 2, 5..10:", "Ring Modulation and hybrid serial/parallel filter paths for metallic bells, sweeps, and aggressive synth leads.");

            subTitle("LFO 1 & LFO 2 (ACTIVE TONE)");
            pWrap("Side-by-side table for both LFOs per Tone: Waveform (TRI, SIN, SAW, SQR, TRP, S&H, RND, CHS), Offset, Key Trig, Rate, Delay, Fade Time/Mode, External Sync (OFF/CLOCK/TAP), plus direct Pitch, TVF, Pan, and TVA modulation depths.");
        }
        break;

    // =========================================================================
    // 06. PATCH EFFECTS
    // =========================================================================
    case 5:
        if (spanish_) {
            subTitle("Switches Maestros y Enrutamiento rapido (Arriba)");
            bulletItem("Master EFX / Master Chorus / Master Reverb:", "Son interruptores globales guardados en System Common. Si estan apagados, NO escucharas ningun efecto aunque el Patch tenga EFX configurado.");
            bulletItem("Route Active Tones -> EFX:", "Boton utilitario de 1 clic que asigna Output Assign = EFX en todos los Tones activos del Patch actual y enciende Master EFX para que escuches inmediatamente el procesador multi-efecto.");

            subTitle("EFX (MULTI-EFFECTS - 40 Algoritmos)");
            bulletItem("Selector Patch Mode Effects vs Performance Effects:", "Elige si editas el bloque EFX del Patch actual o el bloque EFX compartido de la Performance.");
            bulletItem("EFX Type (01..40) y Visualizador:", "Cada algoritmo reorganiza dinamicamente las perillas/faders y muestra su curva o diagrama en vivo: EQ/Spectrum, Overdrive/Distortion, Phaser, Enhancer, Auto-Wah, Rotary, Compressor/Limiter, Hexa/Tremolo/Space-D/Stereo Chorus, Flangers, Delays multi-tap, Pitch Shifters, Reverbs y cadenas duales en Serie/Paralelo (tipos 26..40).");
            bulletItem("EFX Out (MIX / DIRECT-1 / DIRECT-2) y EFX SENDS:", "Define la salida del bloque EFX y cuanto envia a MixOut, Chorus (ChoSnd) y Reverb (RevSnd). A la derecha, Ctrl 1 y Ctrl 2 permiten modular parametros del EFX en tiempo real (ej. velocidad del Rotary o wah con Mod Wheel).");

            subTitle("CHORUS y REVERB / DELAY (Abajo)");
            bulletItem("CHORUS:", "Procesador estereo dedicado con salida enrutada a MIX, REV o MIX+REV (enviar Chorus hacia Reverb da mucha profundidad a pads y cuerdas). Ajusta C.Level, Rate, Depth, PreDly y Feedback.");
            bulletItem("REVERB / DELAY:", "8 tipos (ROOM1, ROOM2, STAGE1, STAGE2, HALL1, HALL2, DELAY, PAN-DLY), filtro pasa-bajos de amortiguacion HF Damp (200 Hz..8 kHz o BYPASS), R.Level, Reverb Time y Delay Feedback.");
        } else {
            subTitle("Master Switches & Quick Routing (Top)");
            bulletItem("Master EFX / Chorus / Reverb:", "Global System Common bypass switches. If any of these are OFF, you will not hear that effect regardless of Patch settings.");
            bulletItem("Route Active Tones -> EFX:", "One-click helper that sets Output Assign = EFX on all active Tones of the current Patch and switches Master EFX ON.");

            subTitle("EFX (MULTI-EFFECTS - 40 Algorithms)");
            bulletItem("Patch Mode vs Performance Effects:", "Switches between editing the standalone Patch EFX block or the shared Performance EFX block.");
            bulletItem("EFX Type (01..40) & Live Visualizer:", "Dynamically maps all 12 EFX parameters to faders/knobs/buttons with live curves and series/parallel block diagrams (Types 26..40).");
            bulletItem("EFX Sends & Control:", "Controls MixOut, Chorus Send, Reverb Send, and two real-time modulation slots (Ctrl 1 & Ctrl 2) to sweep EFX parameters via MIDI controllers.");

            subTitle("CHORUS & REVERB / DELAY (Bottom)");
            bulletItem("CHORUS:", "Dedicated stereo Chorus with Output routing (MIX / REV / MIX+REV), Level, Rate, Depth, Pre-Delay, and Feedback.");
            bulletItem("REVERB / DELAY:", "8 Reverb/Delay modes (ROOM1..HALL2, DELAY, PAN-DLY) with HF Damp (200 Hz..BYPASS), Level, Time, and Delay Feedback.");
        }
        break;

    // =========================================================================
    // 07. DRUMS EDITOR
    // =========================================================================
    case 6:
        if (spanish_) {
            subTitle("Como funciona el Drums Editor (Rhythm Kits)");
            pWrap("En el JV-1010 un Rhythm Set contiene hasta 64 sonidos de percusion independientes asignados a las teclas MIDI #35 (B1) hasta #98 (D7), operando siempre sobre el Canal MIDI 10 (Part 10).");

            bulletItem("Kit Name & Audition Key (Ch 10):", "Cambia el nombre (12 caracteres) del kit temporal en RAM y dispara la tecla seleccionada en el Canal 10 para probar tus cambios sin teclado externo.");
            bulletItem("Lista izquierda de teclas (#35..#98):", "Muestra numero de nota MIDI, nota musical y numero de onda asignada (`W:xxx`). Al hacer clic, selecciona esa tecla y la audiciona.");
            bulletItem("RHYTHM KIT NOTE INSPECTOR:", "Edita exclusivamente la tecla seleccionada: Tone On, WaveGrp/Wave/Gain, EnvMode (NO-SUS = reproduce la envolvente completa ideal para bombos/cajas aunque sueltes rapido la tecla; SUSTAIN = corta al recibir Note Off, util para platos o loops), Filter (OFF/LPF/BPF/HPF/PKG), Out (MIX/EFX) y switches Vol/Hold1.");
            bulletItem("MuteGrp (0..31):", "Grupo de exclusion mutua: asigna el mismo numero (ej. MuteGrp = 1) al Hi-Hat cerrado, pedal y abierto para que al golpear el cerrado se corte instantaneamente la cola del hi-hat abierto, igual que en una bateria real.");
            bulletItem("Knobs y 3 Envolventes por tecla:", "Ajusta Bend, Coarse (tono MIDI C-1..G9), Fine, RndPch, Cutoff/Reso/FltEnv, Level/Pan/RndPan/AltPan, envios MixEFX/Chorus/Reverb y edita graficamente Pitch Env, Filter Env y Level Env por cada instrumento del kit.");
        } else {
            subTitle("How the Drums Editor Works (Rhythm Kits)");
            pWrap("A JV-1010 Rhythm Set maps 64 independent drum instruments across MIDI notes #35 (B1) through #98 (D7) on MIDI Channel 10 (Part 10).");

            bulletItem("Kit Name & Audition Key (Ch 10):", "Renames the temporary Rhythm Set and auditions the selected drum note on Channel 10.");
            bulletItem("Note List (#35..#98):", "Shows MIDI key, musical note, and assigned waveform number; clicking selects and triggers the sound.");
            bulletItem("Inspector Controls:", "Tone On, WaveGrp/Wave/Gain, EnvMode (NO-SUS plays full one-shot percussion regardless of Note Off; SUSTAIN stops on Note Off), Filter Type, Output Assign (MIX/EFX), and Vol/Hold1 switches.");
            bulletItem("MuteGrp (0..31):", "Assign the same Mute Group (e.g., 1) to Closed Hi-Hat, Pedal Hi-Hat, and Open Hi-Hat so hitting a closed hat chokes the open hat naturally.");
            bulletItem("Per-Key Knobs & 3 Envelopes:", "Full Coarse/Fine/Random Pitch, Filter Cutoff/Reso, Level/Pan/Random/Alternate Pan, Effects Sends, plus draggable Pitch, Filter, and Level Envelope graphs for every key.");
        }
        break;

    // =========================================================================
    // 08. PERFORMANCE EDITOR
    // =========================================================================
    case 7:
        if (spanish_) {
            subTitle("Modo Multitimbrico: Performance Editor (16 Partes)");
            pWrap("Una Performance administra hasta 16 instrumentos simultaneos (Partes 1..9 y 11..16 son Patches; Part 10 es siempre el Rhythm Set). Es el modo que usas para secuenciar canciones en tu DAW o armar splits y capas complejas en vivo.");

            bulletItem("Cabecera de Performance:", "Perf Name, Tempo (20..250 BPM), Clock (PERFORM/SYSTEM), Mode (LAYER = todas las partes suenan juntas desde un teclado/canal; SINGLE = cada parte responde solo a su canal MIDI independiente para arreglos de 16 pistas), Key Range (activa splits por parte) y EFX Src (elige que Parte presta sus ajustes EFX al unico procesador multi-efecto compartido de la Performance).");
            bulletItem("Contador Voices (X/64):", "Suma la reserva de voces asignada entre las 16 partes. Si supera 64 se muestra en rojo como advertencia.");
            bulletItem("Canales del Mixer (PART 1..16 / P10 DRUMS):", "Cada tira controla On (Receive Switch), boton 'Edit' (salta directo al Patch Editor apuntando al buffer temporal de esa Parte para afinar su timbre en vivo sin tocar el banco USER!), canal MIDI (Ch 1..16), Grupo/Numero de Patch (#001..256), Output Assign (MIX / EFX / DIR1 / DIR2 / PATCH), fader Lvl, Pan, Mix, Cho, Rev, Coarse/Fine Tune, Voice Reserve (Res 0..64: garantiza voces minimas a partes clave para que no se corten en pasajes densos), Key Range (L: / H:) y filtros MIDI PC (Program Change), Vl (Volume) y Hd (Hold-1).");
        } else {
            subTitle("Multitimbral Mode: Performance Editor (16 Parts)");
            pWrap("A Performance coordinates 16 simultaneous Parts (Parts 1..9 & 11..16 host Patches; Part 10 hosts the Rhythm Set) for DAW multi-track playback or live splits and layers.");

            bulletItem("Performance Header:", "Perf Name, Tempo, Clock (PERFORM/SYSTEM), Mode (LAYER stacks active parts across the keyboard; SINGLE isolates each part to its own MIDI channel), Key Range switch, and EFX Src (selects which Part supplies the single shared EFX algorithm for the Performance).");
            bulletItem("Voices Counter (X/64):", "Displays total reserved voices across all 16 Parts (turns red if >64).");
            bulletItem("Channel Strips (PART 1..16 / P10 DRUMS):", "Each strip controls Receive Switch (On), 'Edit' (jumps to Patch Editor targeting that Part's temporary buffer without overwriting USER memory), MIDI Channel, Patch Bank/#, Output Assign (MIX/EFX/DIR1/DIR2/PATCH), Level, Pan, Sends, Coarse/Fine Tune, Voice Reserve (0..64), Key Range Lower/Upper, and MIDI Receive filters (PC, Vl, Hd).");
        }
        break;

    // =========================================================================
    // 09. SYSTEM EDITOR & SCALE TUNE
    // =========================================================================
    case 8:
        if (spanish_) {
            subTitle("Parametros Globales del Sintetizador (System Common)");
            bulletItem("Mode (PERFORMANCE / PATCH / GM):", "Cambia directamente el modo operativo del hardware. Elegir GM envia el mensaje Universal 'GM System On'.");
            bulletItem("Clock (INT / MIDI):", "Elige si el tempo maestro usa el reloj interno o se sincroniza con MIDI Clock (F8H) proveniente de tu DAW.");
            bulletItem("PerfCtrlCh (1..16 / OFF):", "Canal MIDI dedicado para cambiar Performances remotamente via Bank Select + Program Change. De fabrica viene en OFF; si lo activas en un canal (ej. 16), el cambio de Performance tiene prioridad sobre el Patch de esa parte.");
            bulletItem("PatchRxCh (1..16):", "Canal MIDI en el que escucha el JV-1010 cuando esta en modo PATCH.");
            bulletItem("VolSrc / AT Src / SysCtrl1-2:", "VolSrc elige si el volumen responde solo a CC#7 (VOLUME) o tambien a CC#11 Expression (VOL&EXP). AT Src define si acepta Aftertouch de Canal, Polifonico o ambos. SysCtrl1 y SysCtrl2 eligen que numeros MIDI CC alimentan las fuentes globales SYS-CTRL1 y SYS-CTRL2 de la Control Matrix.");
            bulletItem("MasterTune / SysTempo / Switches:", "MasterTune calibra la afinacion global (A4 = 427.4..452.6 Hz, defecto 440.0 Hz). Los switches ScaleTune, EFX, Chorus, Reverb, PatchRemain (evita que se corten notas sostenidas al cambiar Patch), Rx ProgChg y Rx BankSel afectan a todo el modulo.");

            subTitle("PHRASE PREVIEW CONFIG & SCALE TUNE (Microafinacion)");
            bulletItem("PHRASE PREVIEW CONFIG:", "Configura que toca el boton Phrase Preview (y al presionar VOLUME en el equipo): modo SINGLE, CHORD o PHRASE y hasta 4 pares Nota/Velocidad (N1..N4 / V1..V4).");
            bulletItem("SCALE TUNE (-64..+63 cents):", "Microafina cada una de las 12 notas (C..B) para Patch Mode o para cualquier Part 1..16 en Performance. Incluye presets del manual original: Equal Temp, Just (C) y Arabian. Si activas 'Send GS (42H)', ademas envia el mensaje exclusivo Roland GS Scale Tune.");
        } else {
            subTitle("Global Synthesizer Settings (System Common)");
            bulletItem("Mode (PERFORMANCE / PATCH / GM):", "Switches hardware Sound Mode directly (selecting GM sends Universal GM System On).");
            bulletItem("Clock / PerfCtrlCh / PatchRxCh:", "Selects INT or external MIDI Clock sync, sets Performance Control Channel (factory default OFF) for remote Performance changes, and sets Patch mode receive channel.");
            bulletItem("VolSrc / AT Src / SysCtrl1-2:", "Selects CC#7 only vs CC#7+CC#11 (VOL&EXP), Channel/Poly Aftertouch routing, and global CC numbers assigned to SYS-CTRL1 and SYS-CTRL2.");
            bulletItem("MasterTune / SysTempo / Global Switches:", "Calibrates A4 tuning (427.4..452.6 Hz), global tempo, ScaleTune master switch, Master EFX/Chorus/Reverb switches, PatchRemain (prevents cutoff when switching patches), and Rx Program Change / Bank Select.");

            subTitle("PHRASE PREVIEW CONFIG & SCALE TUNE");
            bulletItem("PHRASE PREVIEW CONFIG:", "Configures PreviewMode (SINGLE, CHORD, PHRASE) and 4 Note/Velocity pairs used by Phrase Preview.");
            bulletItem("SCALE TUNE (-64..+63 cents):", "Micro-tunes each semitone C..B per octave for Patch Mode or Parts 1..16, with factory Equal Temp, Just (C), and Arabian presets plus optional Roland GS (42H) SysEx transmission.");
        }
        break;

    // =========================================================================
    // 10. FOOTER & SOUND ON SOUND POWER-USER TIPS (PARTS 1-4)
    // =========================================================================
    case 9:
    default:
        if (spanish_) {
            subTitle("Footer Dock: Morph A<->B, Randomizer y Teclado Virtual");
            bulletItem("Set A / Set B / Morph A<->B:", "Captura dos Patches distintos como Snapshots A y B y desliza 'Morph A<->B' para interpolar sus parametros de sintesis en tiempo real enviando SysEx continuo al JV-1010.");
            bulletItem("Randomize Patch:", "Genera variaciones creativas instantaneas sobre el Patch actual (usa Compare A/B arriba si queres volver atras).");
            bulletItem("KbdCh / Chord / Arp / Phrase Preview / Panic / Vol / Mod:", "KbdCh ajusta el canal MIDI del teclado virtual (y actualiza PatchRxCh en modo Patch). Chord dispara triadas mayores al tocar una tecla; Arp ejecuta un arpegio continuo; Panic envia All Notes Off en los 16 canales si alguna nota queda colgada; Vol y Mod envian Patch Level / CC#7 y Mod Wheel CC#1.");

            subTitle("Tips Avanzados (Sound On Sound - Nick Magnus, Partes 1 a 4)");

            drawCallout("SOS PARTE 1: AUTOMATIZACION CON CC#11 (EXPRESSION) Y FXM ALEATORIO",
                        "1) En arreglos multitimbricos desde tu DAW, deja Part Level / CC#7 fijo para la mezcla y automatiza dinamicas musicales (crescendos, fades) con MIDI CC#11 (Expression), asegurandote en System Editor de tener VolSrc = VOL&EXP.\n"
                        "2) Para dar vida agresiva o texturas metalicas organicas a ondas estaticas: activa FXM en un Tone (Color 1..4, Depth 4..12) y en Control Matrix asigna LFO1 o LFO2 con forma RND (Key Trig = OFF) modulando Pitch o Filter para romper la rigidez digital de las muestras PCM.",
                        IM_COL32(36, 98, 178, 255));

            drawCallout("SOS PARTE 2: DISCIPLINA DE BACKUPS Y HEADROOM INTERNO",
                        "1) Antes de programar sesiones largas o pisar slots USER con 'Write User', usa Browser -> Pull User Bank -> Pull Full User Bank y Top Bar -> Export -> Full User Memory Backup (.syx). Guarda backups por proyecto en carpetas separadas.\n"
                        "2) Headroom interno: sumar 4 Tones con Wave Gain = +12 dB y Booster = +18 dB puede saturar internamente el bus de efectos y la salida digital/analogica en acordes densos de 64 voces. Manten Wave Gain en 0 dB como base y equilibra Tone Level antes de subir Patch Level.",
                        IM_COL32(195, 118, 24, 255));

            drawCallout("SOS PARTE 3: EDITA EN PERFORMANCE SIN MIEDO, PANNING Y EFECTOS",
                        "1) 'Tweak with Confidence': en Performance Editor pulsa 'Edit' en cualquier Parte (1..9, 11..16). Editaras el Patch en su buffer temporal exclusivo de Performance sin modificar el Patch original en memoria USER ni estropear otras canciones.\n"
                        "2) Maximizando 1 solo EFX compartido: asigna EFX Src a la Parte que realmente necesita un inserto caracteristico (ej. Guitarra Overdrive o Organo Rotary) y usa los envios Chorus/Reverb por Parte para vestir al resto.\n"
                        "3) Si una Parte no panea bien en Performance: verifica Output Assign (si esta en EFX con un algoritmo mono o en PATCH, puede ignorar Part Pan), revisa Pan Control Switch en los Tones y que Patch Pan / Tone Pan no esten fijados extremos.\n"
                        "4) Automatizando sonidos con Overdrive/Distortion: bajar Tone Level o CC#11 antes de una distorsion limpia el timbre. Si queres bajar volumen manteniendo la saturacion intacta, ajusta el nivel post-EFX o automatiza en mezcla.",
                        IM_COL32(32, 138, 86, 255));

            drawCallout("SOS PARTE 4: ANALOGUE FEEL REALISTA, STRUCTURES CON BOOSTER Y MATRIZ CREATIVA",
                        "1) 'Analogue Feel' superior: el parametro Analog Feel en Patch Common agrega deriva sutil; para lograr una emulacion analogica aun mas viva en pads/cuerdas, programa en cada Tone un LFO lento distinto en forma RND (Key Trig = OFF) modulando sutilmente Pitch (+-1..3) y Cutoff (+-4..8), mas Random Pitch Depth en 1..3. Evita excederte o sonara desafinado.\n"
                        "2) Soldar ondas con Structures 3 y 4 + Booster: elige Structure 3 o 4 en 'STRUCTURES 1+2 & 3+4' y sube Booster a +6, +12 o +18 dB. La etapa de distorsion entre filtros genera intermodulacion que 'suelda' dos formas de onda simples en un bajo o lead analogico enorme.\n"
                        "3) Abuso creativo de Matriz, Envolventes y Delay: encadena LFO1 -> LFO2 Rate (destino L2R) para vibratos no repetitivos; usa V-On Time para ataques mas rapidos al tocar fuerte; activa Peak+Hold para sostener modulaciones maximas; y usa Tone Delay Mode = KEY-OFF-N/D en el Tone 4 para generar ruidos mecanicos o colas al soltar la tecla.",
                        IM_COL32(198, 42, 42, 255));
        } else {
            subTitle("Footer Dock: Morph A<->B, Randomizer & Virtual Keyboard");
            bulletItem("Set A / Set B / Morph A<->B:", "Captures two Patch snapshots and continuously morphs synthesis parameters between them in real time.");
            bulletItem("Randomize Patch:", "Creates instant sound-design variations in the Temporary Buffer (use Compare A/B to revert).");
            bulletItem("KbdCh / Chord / Arp / Phrase Preview / Panic / Vol / Mod:", "Sets virtual keyboard MIDI channel, plays major triads (Chord) or an 8-step arp (Arp), triggers Phrase Preview, sends All Notes Off on all 16 channels (Panic), and adjusts Patch Level/CC#7 and Mod Wheel CC#1.");

            subTitle("Power-User Tips (Sound On Sound - Nick Magnus, Parts 1-4)");

            drawCallout("SOS PART 1: CC#11 EXPRESSION AUTOMATION & RANDOM FXM",
                        "1) In multitimbral DAW projects, keep Part Level / CC#7 for static mix balance and automate musical dynamics using MIDI CC#11 (Expression) with VolSrc = VOL&EXP in System Editor.\n"
                        "2) Enable FXM (Color 1..4, Depth) and modulate Pitch/Cutoff/FXM textures via Control Matrix using an LFO set to RND (Key Trig = OFF) for evolving metallic or grainy motion.",
                        IM_COL32(36, 98, 178, 255));

            drawCallout("SOS PART 2: BACKUP DISCIPLINE & INTERNAL HEADROOM",
                        "1) Always back up before overwriting USER slots: run Browser -> Pull Full User Bank and Top Bar -> Export -> Full User Memory Backup (.syx).\n"
                        "2) Internal Headroom: combining 4 Tones at Wave Gain +12 dB with +18 dB Boosters in dense 64-voice chords can clip internal buses. Start with Wave Gain at 0 dB and balance Tone Levels cleanly.",
                        IM_COL32(195, 118, 24, 255));

            drawCallout("SOS PART 3: TWEAK WITH CONFIDENCE, MULTITIMBRAL EFX & PANNING",
                        "1) 'Tweak with Confidence': click 'Edit' on any Part strip in Performance Editor to modify that Part's Patch inside its exclusive Performance Temporary Buffer without altering USER memory.\n"
                        "2) Stretching One EFX: assign EFX Src to the lead/featured Part (e.g., Rotary organ or Overdrive guitar) and use shared Chorus/Reverb sends for the remaining parts.\n"
                        "3) Fixing Panning Quirks: if a Part ignores Part Pan, check Output Assign (mono/centered EFX or PATCH routing overrides Part Pan) and Tone Pan Control Switch.\n"
                        "4) Automating Overdriven Sounds: lowering pre-EFX level cleans up distortion; adjust post-EFX or output level to preserve grit while fading.",
                        IM_COL32(32, 138, 86, 255));

            drawCallout("SOS PART 4: ANALOGUE FEEL, BOOSTER STRUCTURES & MATRIX TRICKS",
                        "1) Realistic Analogue Feel: combine Patch Common 'Analog Feel' with slow, per-Tone RND LFOs (Key Trig = OFF) gently modulating Pitch (+-1..3) and Cutoff, plus Random Pitch Depth (1..3), avoiding excessive values that sound out of tune.\n"
                        "2) Welding Waveforms with Booster Structures (3 & 4): routing Tones in series through a Booster (+6/+12/+18 dB) introduces warm intermodulation that fuses waveforms into fat analog basses and leads.\n"
                        "3) Creative Matrix & Envelopes: route LFO1 -> LFO2 Rate (L2R) for non-periodic motion, map V-On Time for faster attacks on hard hits, use Peak+Hold to latch modulation peaks, and set Tone 4 Delay Mode to KEY-OFF-N/D for key-release noises.",
                        IM_COL32(198, 42, 42, 255));
        }
        break;
    }

    ImGui::EndChild();
}

} // namespace jv1010
