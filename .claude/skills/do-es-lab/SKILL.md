---
name: do-es-lab
description: Complete an embedded systems lab assignment — produces working PlatformIO code, a LaTeX report, and a Wokwi simulation following all project conventions. Invoke with the lab conditions as arguments, or paste them after invoking.
argument-hint: "<lab conditions or lab number>"
disable-model-invocation: true
---

Complete the following embedded systems lab assignment, following ALL rules in this skill strictly.

**Lab conditions / task statement:**
$ARGUMENTS

---

## ⚠️ CRITICAL RULES (READ FIRST)

When the user denies a tool call (e.g. dismisses a Bash command), IMMEDIATELY STOP and ask what to do next using the AskUserQuestion tool. Do not continue.

"Denied" means the user explicitly rejected the tool execution. Treat the command as not executed — never assume it ran.

For every CLI command:
1. Execute it autonomously using the Bash tool — no need to announce it or wait for permission.
2. If the user **denies** the tool call → stop immediately, do not assume the command ran, and use the AskUserQuestion tool with these options:
   - Run the command now and I'll retry
   - Skip this command and continue with an alternative approach
   - Perform this step manually yourself
3. Wait for the user's choice before proceeding. Never continue without an explicit answer.

Violations:
- Assuming denied commands were executed ❌
- Continuing dependent tasks without the output ❌
- Asking in plain text instead of AskUserQuestion ❌
- Moving forward without the user's explicit choice ❌

---

## Documentation Style

All **lab reports** should prioritize readability and clarity. Use paragraphs for conceptual explanations, workflows, and detailed descriptions. Use bulleted or numbered lists only when strictly necessary (e.g., for step-by-step procedures, simple enumerations, or dense reference material). Avoid over-listing; convert multi-item lists into prose when they form a coherent narrative.

---

## Command Execution Rules

See the CRITICAL RULES section above for the full denied-command policy. Additional rules:

### CLI Failure Handling

If a CLI command fails, attempt to resolve it autonomously with safe, non-destructive steps. Limit troubleshooting to at most **3** follow-up attempts; after each, briefly state what changed and why. If it still fails, stop and use AskUserQuestion to determine the next step. Never run destructive commands (delete/format/flash erase) without explicit user approval.

### LaTeX Compilation Failure Handling

If `latexmk` fails, check the `.log` file for the error (search for lines starting with `!` or containing `Error`). Common causes: missing `\end{...}`, unescaped special characters (`#`, `%`, `&`, `_` in text), or undefined control sequences. Fix the offending `.tex` file and retry. The same 3-attempt limit applies — if still broken, use AskUserQuestion.

---

## Autonomy vs. Manual Actions

- Be as autonomous as possible: implement, refactor, run builds, run tests, and generate report content automatically.
- If a step requires a human-only action (e.g., taking screenshots, photographing hardware setup, pressing buttons, rewiring, reconnecting the board), do NOT block progress waiting for the user.

### Autonomous Tasks (Always Complete Without Asking)

- **Code implementation & compiling**: Write all code, build with PlatformIO, verify both lab environments compile successfully.
- **Test execution**: Run all unit tests and integration tests.
- **Diagram generation**: Diagrams are written directly in LaTeX using TikZ (flowcharts, FSM, block diagrams) and CircuiTikZ (electrical schematics) — no external tools or export step needed.
- **Report compilation**: Compile LaTeX to PDF using `latexmk -pdf -interaction=nonstopmode main.tex` from inside the report folder. This always uses the pdflatex engine.
- **Git workflow (commits only)**: Commit with clear messages after each completed task. Group logically (e.g., one commit for code, one for documentation). **Do NOT push automatically** — push only after explicit user confirmation.
- **Wokwi simulation setup**: Create `diagram.json` and `wokwi.toml` with correct pin mapping.

### Manual Tasks (Ask User & Insert Placeholders)

The following require human actions and should only be requested (with exact specifications):

- **Wokwi/hardware screenshots**: UI interactions, button presses, selecting menu options, capturing specific states.
- **Oscilloscope/logic analyzer captures**: Physical measurement device interactions.
- **Hardware setup photos**: Breadboard assembly, PCB integration, physical wiring.
- **Device flashing/rewiring**: Physically connecting/disconnecting hardware.

### When a manual action is needed

- Ask the user to perform the manual action.
- Immediately continue by assuming the user will provide/perform it.
- Insert a **live, uncommented** `\begin{figure}...\end{figure}` block in the report at the correct location. **Never comment out figure blocks.** The figure must be unconditionally included so it appears in the final PDF when the image is placed.
- To allow the report to compile before the image exists, wrap `\includegraphics` in `\IfFileExists`:

```latex
\begin{figure}[H]
\centering
\IfFileExists{resources/ObtainedResults/screenshot.png}{%
    \includegraphics[width=0.75\textwidth]{resources/ObtainedResults/screenshot.png}%
}{%
    \fbox{\parbox{0.73\textwidth}{\centering\color{gray}\small
        [Screenshot pending — save to resources/ObtainedResults/screenshot.png]}}%
}
\caption{Description of what the screenshot shows}
\label{fig:screenshot_label}
\end{figure}
```

- Specify precisely:
  - what the screenshot/photo must show
  - which window/screen/menu should be visible
  - where to place the image in the report/repo
  - how to name the file (consistent naming convention)
  - any caption text to include

---

## Project Context

- Repository name: `es-labs`
- PlatformIO project folder: `labs/` (this is the active PlatformIO project)
- Development environment: VS Code + PlatformIO, Wokwi VS Code plugin for simulation, LaTeX for writing lab reports.

---

## Per-Lab Deliverables

For every lab, produce ALL of the following:

1. **Working embedded code** (PlatformIO project code)
2. **A lab report** (LaTeX source compiled to PDF using `latexmk`)
3. **A Wokwi simulation setup** (circuit + config matching the code)

All three must match each other: same features, same behavior, same pin mapping, same module structure.

---

## Repository Structure Convention

- Keep all embedded code inside the PlatformIO project folder: `labs/`
- Organize each lab as its own module while sharing reusable code via libraries.

Recommended structure:

```
labs/
  src/
    main.cpp
  lab/
    lab1_1/
      lab1_1_main.cpp
    lab1_2/
      lab1_2_main.cpp
    lab2_1/
      lab2_1_main.cpp
  lib/
    drivers/
    hal/
    utils/
    protocols/
  include/   (if needed)
  platformio.ini
```

---

## Lab Isolation (No Overwrites)

- Labs MUST NOT overwrite each other.
- Each lab must have its own entry-point file (a lab-specific main file).
- The repository must preserve all previous labs in a runnable state.

### Lab Entry-Point Rules

- Each lab must have a dedicated file, for example:
  - `labs/lab/lab1_1/lab1_1_main.cpp`
  - `labs/lab/lab1_2/lab1_2_main.cpp`

- `labs/src/main.cpp` must only select and run ONE lab at a time.
  - It calls into the chosen lab entry point using `#ifdef` guards: `#ifdef LAB2_1`, `#elif defined(LAB1_2)`, etc.
  - Each PlatformIO environment in `platformio.ini` sets the corresponding flag via `build_flags = -DLAB2_1`.
  - Switching labs means changing the active environment, not deleting or modifying code.

### Reuse Rules

- Lab entry points should only contain lab-specific orchestration logic.
- Any reusable code must be extracted into libraries under `labs/lib/`.
- Before implementing any new functionality, **always check `labs/lib/` first**. If a library component already exists (e.g., `LcdDisplay`, `KeypadInput`, `Led`, `StdioSerial`, `TaskScheduler`), use it directly — do not reimplement it.
- **No low-level code anywhere outside `labs/lib/`.** Direct register manipulation, raw `digitalWrite`/`analogRead` calls, protocol handling (UART, I2C, SPI), and hardware initialization must all live in library modules under `labs/lib/` — never inline in the lab entry point, `main.cpp`, or any lab task/module file.

---

## Modularity Rules

- Code must be modular and readable.
- Separate hardware drivers (GPIO, UART, PWM, ADC, timers, etc.) from application logic.
- Prefer small focused modules over one large `main.cpp`.
- If functionality is reusable across labs, put it in `labs/lib/` as a library:
  - Provide a clear API in a header file.
  - Hide implementation details in `.cpp`.
- Keep naming consistent and explicit (no vague module names like `stuff.cpp`).

---

## Hardware Accuracy & Pin Mapping

### Hardware Accuracy Rules

- MUST NOT assume hardware features that are not explicitly stated.
- If the MCU/board/pinout is unknown, ask the user before choosing pins or peripherals.
- MUST NOT invent: sensor specifications, voltage levels, wiring details, or board capabilities.

### Pin Mapping Rules

- Always define pin mappings in a single place (constants or a config header).
- Use the exact same pin mapping in:
  - PlatformIO code
  - Wokwi `diagram.json`
  - Report wiring/schematic section

---

## Simulation (Wokwi)

- Each lab must include a Wokwi simulation setup.
- Each lab simulation MUST be in its own folder.
- The simulation must match the implementation exactly: same pin mapping, same components, same behavior.

### Wokwi Folder Convention

- Each lab must have its own Wokwi folder:
  - `labs/wokwi/lab1.1/diagram.json`
  - `labs/wokwi/lab1.2/diagram.json`
  - `labs/wokwi/lab2.1/diagram.json`

- The simulation should be reproducible by opening the lab in VS Code and starting Wokwi.
- If a lab cannot be fully simulated: clearly state what is not supported and why, and provide a workaround.

### Wokwi Circuit Screenshot (Required Manual Action)

Every lab report's Design section must include a screenshot of the running Wokwi simulation. This is a manual action — Claude cannot take the screenshot autonomously.

After setting up the Wokwi simulation, always ask the user to:

1. Open the lab's `wokwi.toml` in VS Code and start the Wokwi simulation.
2. Capture a screenshot showing the full circuit with all components visible and the simulation actively running (LEDs lit, LCD showing text, not just the static wiring diagram).
3. Save the file as `reports/<lab>/resources/Design/ElectricalSchematics/wokwi_circuit.png`.

In the report, add a **live, uncommented** `\begin{figure}...\end{figure}` block using `\IfFileExists` so the report compiles cleanly before the image is provided. Never comment out the figure block.

---

## Reports (LaTeX)

### Report Template

- All lab reports must be based on the existing LaTeX template located in: `template/`
- When creating a new report folder (e.g. `reports/lab1.1/`), copy/adapt the template instead of starting from scratch.

### Report Folder Convention

- Each lab has its own LaTeX report folder: `reports/lab1.1/`, `reports/lab1.2/`, etc.

Each report folder should contain:
- `main.tex`
- `resources/` (organized subdirectories for screenshots, photos, and other external images)
- optional extra `.tex` sections if needed

### Report Context Efficiency

> ⛔ **HARD RULE — Enforced without exception:**
> When writing or updating any lab report, you are **only permitted to read** two sources for report context:
> 1. `template/` — primary reference for ALL formatting, structure, package setup, and diagram style.
> 2. `reports/lab1.1/` — secondary reference **only** for gauging prose depth (Domain Analysis, Design, Results).
>
> **Reading any other completed lab report (`reports/lab1.2/`, `reports/lab2.1/`, `reports/lab2.2/`, etc.) is strictly forbidden.** Do not open, read, grep, or search those folders when creating or editing a report.
>
> If you find yourself about to read a file from `reports/lab1.2/` or later, **stop and redirect to `template/` instead.**

**Important about Lab 1.1:** Lab 1.1 used Mermaid diagrams (`.mmd` files) which are now obsolete. All post-1.1 labs use TikZ / CircuiTikZ drawn directly in LaTeX. Never copy diagram code from Lab 1.1.

**Sole exception — images only:** You may search completed lab folders exclusively to locate reusable image files (`.png`, `.jpg`, `.pdf`) for copying into the new lab's `resources/` folder. Only inspect `resources/` subdirectories, never `.tex` files.

### Report Language

- All reports must be written in English.
- Only the lab task statement may remain Romanian if provided by the teacher. Place it in the Domain Analysis section as a quoted block, then follow with the English analysis.

### Image Organization in Reports

```
reports/<lab>/resources/
├── DomainAnalysis/
│   ├── UsedTechnologies/
│   ├── HardwareComponents/
│   └── SoftwareComponents/
├── Design/
│   ├── SystemArchitectureDiagrams/
│   ├── BlockDiagrams/
│   ├── ElectricalSchematics/
│   └── ModularImplementation/
└── ObtainedResults/
```

### Domain Analysis Images (Reusable Template Content)

**HardwareComponents/** — For every lab, always ask the user to provide a photo or product image for each hardware component introduced. Insert a live `\IfFileExists` figure placeholder immediately and ask the user to supply the image.

Always request photos for:
- **MCU / development board** (e.g., Arduino Mega 2560): full-board product photo. Filename: `arduino_mega_2560.png`. If this image already exists in a previous lab, copy it — never reference across labs.
- **Each peripheral introduced in the lab**: push button, LED, LCD module, keypad, sensor, etc.

**SoftwareComponents/** — For each software tool used (PlatformIO IDE, Wokwi simulator), ask the user to provide a screenshot showing the tool in use for this lab. These are manual actions — insert a live `\IfFileExists` figure placeholder.

- `platformio.png` — show the project open in VS Code with the PlatformIO sidebar visible.
- `wokwi.png` — show the Wokwi simulator running the lab's circuit.
- If identical screenshots were used in a previous lab, copy them into the current lab's folder — never reference across labs.

**All images in HardwareComponents and SoftwareComponents MUST be referenced in the report.** If an image exists but is not referenced with a `\begin{figure}...\end{figure}` block, either remove or include it.

**Reusing images:** Each lab report must use only its own local `resources/` folder — never reference images with paths like `../lab1.1/resources/...`. Copy reusable images into the current lab's resources folder.

### Image Naming Convention

- Use snake_case filenames.
- Recommended patterns:
  - **Domain Analysis**: `uart_frame.png`, `i2c_protocol.png`, `arduino_mega_2560.png`, `lcd_1602_i2c.png`
  - **Design**: `system_structural_diagram.png`, `fsm_state_diagram.png`, `app_flowchart.png`, `electric_schema.png`
  - **Obtained Results**: `build_results.png`, `simulation_start.png`, `test_<scenario>.png`, `test_report.png`

Include the lab number only for lab-specific screenshots; omit it for reusable technology/component diagrams.

### Diagrams in Reports (LaTeX)

All diagrams must be drawn directly in LaTeX — no external tools, no PNG exports. Use **TikZ** (flowcharts, FSM state diagrams, block/architecture diagrams) and **CircuiTikZ** (electrical schematics). Both are loaded in `settings.tex`.

#### TikZ Flowcharts

```latex
\tikzset{
    startstop/.style={rectangle, rounded corners, minimum width=2.5cm,
        minimum height=0.7cm, text centered, align=center, draw=black, fill=gray!15},
    process/.style={rectangle, minimum width=2.5cm,
        minimum height=0.7cm, text centered, align=center, draw=black, fill=blue!10},
    decision/.style={diamond, aspect=2.5, minimum width=2cm,
        minimum height=0.7cm, text centered, align=center, draw=black, fill=orange!15},
    arrow/.style={thick,->,>=Stealth}
}

\begin{figure}[H]
\centering
\begin{tikzpicture}
    \node (start) [startstop] at (0,   0.0)  {Start};
    \node (proc1) [process]   at (0,  -1.8)  {Process 1};
    \node (dec1)  [decision]  at (0,  -4.3)  {Condition?};
    \node (proc2) [process]   at (0,  -6.8)  {Process 2};
    \node (stop)  [startstop] at (0,  -8.6)  {End};
    \draw [arrow] (start) -- (proc1);
    \draw [arrow] (proc1) -- (dec1);
    \draw [arrow] (dec1)  -- node[anchor=east]{Yes} (proc2);
    \draw [arrow] (proc2) -- (stop);
\end{tikzpicture}
\caption{Flowchart of the application logic}
\label{fig:app_flowchart}
\end{figure}
```

#### TikZ FSM State Diagrams

```latex
\begin{figure}[H]
\centering
\begin{tikzpicture}[>=Stealth, auto, node distance=2.8cm,
        every state/.style={draw, circle, minimum size=1.2cm}]
    \node[state, initial]  (idle) {IDLE};
    \node[state] (menu)    [right of=idle]  {MENU};
    \node[state] (confirm) [right of=menu]  {CONFIRM};
    \node[state, accepting] (result) [below of=menu] {RESULT};
    \path[->]
        (idle)    edge              node {*}    (menu)
        (menu)    edge              node {0}    (confirm)
        (confirm) edge              node {\#}   (result)
        (result)  edge [bend right] node {auto} (idle);
\end{tikzpicture}
\caption{FSM state diagram}
\label{fig:fsm_state_diagram}
\end{figure}
```

#### TikZ Architecture / Block Diagrams

```latex
\begin{figure}[H]
\centering
\begin{tikzpicture}[
    box/.style={draw, rectangle, minimum width=5cm, minimum height=0.8cm,
                text centered, fill=blue!10},
    node distance=0.5cm]
    \node[box] (app)  {Application Layer (APP)};
    \node[box] (srv)  [below=of app]  {Service Layer (SRV)};
    \node[box] (ecal) [below=of srv]  {MCU Abstraction (ECAL)};
    \node[box] (mcal) [below=of ecal] {Driver Layer (MCAL)};
    \node[box] (hw)   [below=of mcal] {Hardware (HW)};
    \foreach \a/\b in {app/srv, srv/ecal, ecal/mcal, mcal/hw}
        \draw[->] (\a) -- (\b);
\end{tikzpicture}
\caption{Layered software architecture}
\label{fig:layered_architecture}
\end{figure}
```

#### CircuiTikZ Electrical Schematics

```latex
\begin{figure}[H]
\centering
\begin{circuitikz}
    \draw (0,0) node[anchor=east]{Pin 7}
        to[R, l=220\,\si{\ohm}] (2,0)
        to[leD*, color=red]     (4,0)
        node[ground] {};
    \draw (0,-1.5) node[anchor=east]{SDA (Pin 20)}
        to[short] (4,-1.5) node[anchor=west]{LCD SDA};
\end{circuitikz}
\caption{Circuit schematic}
\label{fig:circuit_schematic}
\end{figure}
```

#### Diagram Complexity Limits (CRITICAL)

- **Flowcharts**: maximum 8 decision nodes. Split larger ones into overview + sub-diagrams.
- **FSM state diagrams**: maximum 8 states in one `tikzpicture`. Split into logical groups if more.
- **Architecture / structural diagrams**: keep readable at `\textwidth`; simplify by grouping subsystems if needed.

#### Node Overlap and Arrow Routing Prevention (CRITICAL)

**Rule 1 — Always use explicit `at (x,y)` coordinates.** Never use `node distance` with `below of=` / `right of=` for flowcharts or structural diagrams. Place every node with an explicit coordinate.

**Rule 2 — Minimum vertical gap between nodes:**
- **1.8 cm** between two process/startstop nodes
- **2.5 cm** between two decision (diamond) nodes or between a decision and any neighbour
- **2.0 cm** gap around trapezoid (I/O) nodes

**Rule 3 — Right-column nodes at same y as left-column decisions.** Return arrows MUST NOT use `|-` routing through that y-level.

**Rule 4 — Route return arrows BETWEEN columns, not around the outside.**

```latex
% RIGHT: route via midpoint between columns
\draw [arrow] (rightNode.west) -- (2.5, -6.2) -- (2.5, -8.7) -- (nextDecision);

% WRONG: |- routing passes through right-column node
% \draw [arrow] (rightNode.east) -- ++(1.0,0) |- (nextDecision.east);
```

Use slightly different x-values (e.g. 2.5, 3.0, 3.5) for multiple return arrows to prevent shared segments.

**Rule 5 — Back-edges in loops must avoid the loop body.** Route via the LEFT side:

```latex
\draw [arrow] (inc.west) -- ++(-2.0,0) |- (loopHead.west);
```

**Rule 6 — Use `calc` library for sublayer positioning.**

**Before finalising any diagram**: mentally trace each node's bounding box and every arrow segment. Confirm: no two boxes overlap, and no arrow segment passes through any node body.

---

## Required Report Structure (Teacher Requirements)

All lab reports must cover: **technology analysis → design → implementation → testing/results**.

### 1. State-of-the-Art / Domain Analysis

Include:
- A description of the technologies used and the application context.
- Hardware and software components used, explaining the role of each.
- System architecture explanation and justification of the chosen solution.
- A relevant case study showing where this solution applies in real life.

### 2. Design

Include:
- An architectural sketch/diagram showing how components connect, with role of each component and interface.
- Behavior/algorithm diagrams (Flowchart, FSM, etc.) with explanations.
- Electrical schematic showing circuit wiring and peripheral integration.
- A Wokwi simulation circuit screenshot (required manual action — ask user, insert `\IfFileExists` placeholder pointing to `resources/Design/ElectricalSchematics/wokwi_circuit.png`).
- A well-organized project structure where each module has a clear role reflecting the architecture diagram.
- Modular implementation rules:
  - each component represented in `.h` and `.cpp` files
  - `.h` files define the interface and explain available methods
  - `.cpp` files contain implementation and explanation of critical code sections
  - include functional block diagrams for each module implemented

### 3. Results

Include:
- Relevant screenshots of system interaction and processed data.
- Photos of the physical hardware setup (if real hardware is used).
- Serial monitor logs demonstrating data exchange and behavior.
- A demo video if required (link to the video).
- Simulation results (Wokwi), if the system was tested in simulation.

**Minimum screenshot coverage** — at least the following must be provided for every lab:

1. **PlatformIO build output**: screenshot of terminal showing successful build (`[SUCCESS]`, RAM/Flash usage). Save as `resources/ObtainedResults/build_results.png`.
2. **Simulation startup**: Wokwi circuit with initial state visible in the serial terminal. Save as `resources/ObtainedResults/simulation_start.png`.
3. **Primary behaviour test**: at least one screenshot per distinct test case documented in the report. Name them descriptively: `test_<scenario>.png`.
4. **Periodic or summary output**: screenshot of serial monitor output showing aggregated results or stats (if applicable). Save as `resources/ObtainedResults/test_report.png`.

Before finalising the report, ask the user for all missing screenshots. Insert `\IfFileExists`-wrapped figure blocks for each — never leave screenshot sections without a figure block.

### 4. Conclusions

Include:
- Performance analysis, limitations, and improvement ideas.
- Conclusions based on the obtained results.
- Real-world impact of the used technology.

### 5. Note About AI Tools

If AI tools were used, explicitly mention it in the report.

Recommended text: "During the writing of this report, the author used [AI TOOL NAME] to generate/consolidate content. The results were reviewed, validated, and adjusted according to the lab requirements."

### 6. Bibliography

- Include at least 3 references.
- References must be from: course materials, official documentation, technical literature.

### 7. Appendix — Source Code

- The full project source code organized into `.cpp` and `.h` modules.
- Code must be documented with explanatory comments per module.
- Include a link to the GitHub repository.

---

## Git / GitHub Workflow

### When to Commit and Push

**After each completed task**, you MUST:

1. **Commit the changes** with a clear message (code, documentation, or both)
2. **Stop and ask the user** if they want to push to GitHub
3. **Only push** if the user explicitly says yes; otherwise, leave changes committed locally

Do not push without user confirmation. Multiple commits can accumulate locally before pushing.

### Commit Strategy

**Code Implementation / Fixes**
- Format: `Lab X.Y: <functionality description>`
- Example: `Lab 1.2: fix Wokwi keypad layout`

**Documentation and Diagrams**
- Format: `Lab X.Y: <documentation update>`
- Example: `Lab 1.2: add report (LaTeX) + Domain Analysis images`

**Multiple Changes in One Prompt**
- Create separate commits for independent changes (code vs. docs), or combine if logically related.

### What to Include in Commits

**Always include:**
- Embedded C/C++ code (all `.cpp`, `.h` files)
- PlatformIO configuration (`platformio.ini`)
- Wokwi simulation files (`diagram.json`, `wokwi.toml`)
- LaTeX report source (`.tex` files)
- All images and diagrams in `resources/` folders
- Compiled PDF of the report (`main.pdf`)

**Never commit:**
- Build artifacts (`.pio/`, `build/`, `.vscode/`)
- Object files (`*.o`, `*.a`)
- LaTeX temporary files (`*.aux`, `*.fdb_latexmk`, `*.fls`, `*.log`, `*.out`, `*.synctex.gz`, `*.toc`, `*.blg`, `*.bbl`, `*.xdv`)

### Commit Message Format

```
Lab X.Y: <component/feature> — <brief description>

Optional body:
- Key design decisions
- Dependency updates
- Integration with previous lab components
```

**Examples:**
- `Lab 1.1: STDIO serial LED control`
- `Lab 1.2: LCD + Keypad lock system with 10-state FSM`
- `Lab 1.2: add report (LaTeX) + Domain Analysis images`

### Push to GitHub

Always ask before pushing. After committing changes locally:

1. Ask the user: "Changes committed locally to main. Would you like me to push to GitHub?"
2. **Only if user says yes**, run: `git push origin main`
3. If user says no, leave the commits local.

### Build Status and Quality Checks

**Before pushing:**
1. Verify the code compiles successfully for all lab environments.
2. Verify the LaTeX report compiles without errors (missing screenshot images are acceptable — they render as grey placeholder boxes via `\IfFileExists`).
3. Ensure Wokwi simulation loads correctly.

Never push broken builds or LaTeX compile errors to the main branch.

### Lab Milestones and Tagging

After each lab is fully complete and pushed, optionally create a Git tag:

```bash
git tag lab1.1
git push origin --tags
```

### Repository Configuration

- **Public visibility**: repository should be set to public.
- **Repository link in reports**: each report's appendix must include: `GitHub Repository: https://github.com/<owner>/es-labs`
