# Embedded Systems Lab Report Template

This template provides a standardized LaTeX structure for Embedded Systems laboratory reports at UTM (Technical University of Moldova).

## Quick Start

1. Copy the entire `template/` folder and rename it to your lab (e.g., `lab_1_2/`):
   ```bash
   cp -r template lab_1_2
   ```

2. Open `title_page.tex` and fill in the placeholders:
   - `{{STUDENT_NAME}}` — your full name (e.g., `Maxim Alexei`)
   - `{{STUDENT_GROUP}}` — your group (e.g., `FAF-232`)
   - `{{LAB_WORK_NUMBER}}` — lab number (e.g., `Laboratory Work n.1.2`)
   - `{{LAB_WORK_TOPIC}}` — lab topic (e.g., `Interaction with the User: STDIO - Serial Interface`)
   - `{{INSTRUCTOR_NAME}}` — instructor name (e.g., `Martîniuc Alexei`)
   - `{{INSTRUCTOR_TITLE}}` — instructor title (e.g., `univ. asist.`)
   - `{{YEAR}}` — current year (e.g., `2026`)

3. Fill in each section file following the `% TODO` comments:
   - `domain_analysis.tex` — objectives, problem, technologies, components
   - `design.tex` — architecture, block diagrams, schematics, code modules
   - `obtained_results.tex` — build, simulation, testing, hardware validation
   - `conclusion.tex` — achievements, limitations, improvements, reflections
   - `bibliography.tex` — references
   - `appendix.tex` — full source code listings
   - `note_regarding_usage_of_ai.tex` — AI tool usage disclosure

4. Place your images in the corresponding `resources/` subfolders:
   - `resources/DomainAnalysis/UsedTechnologies/`
   - `resources/DomainAnalysis/HardwareComponents/`
   - `resources/DomainAnalysis/SoftwareComponents/`
   - `resources/Design/SystemArchitectureDiagrams/`
   - `resources/Design/BlockDiagrams/`
   - `resources/Design/ElectricalSchematics/`
   - `resources/Design/ModularImplementation/`
   - `resources/ObtainedResults/`
   - `resources/ObtainedResults/HardwareResults/`

5. Build the PDF:
   ```bash
   latexmk -pdf main.tex
   ```
   Build artifacts go to `builds/` (configured in `.latexmkrc`).

## File Structure

```
template/
├── .latexmkrc                  # Latexmk build configuration
├── main.tex                    # Main document entry point
├── settings.tex                # Packages, formatting, macros (DO NOT MODIFY)
├── title_page.tex              # Title page with placeholders
├── domain_analysis.tex         # Section 1: Domain Analysis
├── design.tex                  # Section 2: Design
├── obtained_results.tex        # Section 3: Obtained Results
├── conclusion.tex              # Section 4: Conclusion
├── note_regarding_usage_of_ai.tex  # Section 5: AI Usage Disclosure
├── bibliography.tex            # Section 6: Bibliography
├── appendix.tex                # Section 7: Appendix (source code)
├── README.md                   # This file
└── resources/
    ├── utm-logo.png            # UTM logo for title page
    ├── Design/
    │   ├── BlockDiagrams/
    │   ├── ElectricalSchematics/
    │   ├── ModularImplementation/
    │   └── SystemArchitectureDiagrams/
    ├── DomainAnalysis/
    │   ├── HardwareComponents/
    │   ├── SoftwareComponents/
    │   └── UsedTechnologies/
    └── ObtainedResults/
        └── HardwareResults/
```

## Useful LaTeX Commands (from settings.tex)

- `\citeimg{fig:label}` — reference a figure (renders as italic *Figure X.Y*)
- `\citelst{lst:label}` — reference a code listing (renders as italic *Listing X.Y*)
- Figures and listings are numbered per section (e.g., 1.1, 1.2, 2.1)
- Code listings use the `lstlisting` environment with predefined styling
- JSON syntax highlighting is available via `language=json`
