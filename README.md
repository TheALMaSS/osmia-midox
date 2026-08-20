# ALMaSS *Osmia bicornis* Model — MIDox Documentation

<!-- TODO after the first release: paste the Zenodo badge here.
     [![DOI](https://zenodo.org/badge/DOI/10.5281/zenodo.XXXXXXX.svg)](https://doi.org/10.5281/zenodo.XXXXXXX) -->

[![Deploy MIDox Documentation](https://github.com/TheALMaSS/osmia-midox/actions/workflows/deploy-docs.yml/badge.svg)](https://github.com/TheALMaSS/osmia-midox/actions/workflows/deploy-docs.yml)

Implementation documentation for the agent-based population model of the red
mason bee, *Osmia bicornis* L., in the ALMaSS framework, following the MIDox
(Model Implementation Documentation with Doxygen) standard.

**📖 Browse the documentation: <https://thealmass.github.io/osmia-midox/>**

## What this is

MIDox addresses the gap between a *formal model* — the mathematical
specification of what a model does — and the *implementation* that actually
runs. The formal specification for this model is published separately
([Ziółkowska et al. 2023](https://doi.org/10.3897/fmj.4.102102)); this
repository documents the code that implements it.

The documentation has two halves that are read together:

- **Narrative documentation** — purpose, architecture, scheduling, the
  biological rationale for each design decision, calibration, inputs, outputs,
  limitations, and complete parameter tables. This is [`docs/index.md`](docs/index.md),
  and it is the front page of the generated site.
- **Doxygen technical documentation** — every class, method and configuration
  variable, cross-linked to the annotated source, with inheritance,
  collaboration and call graphs. Generated from the comments in the C++ sources.

Each half links into the other: a parameter in the tables links to its
declaration in the code, and a class in the code links back to the section of
the narrative that explains why it behaves as it does.

## Repository layout

```
osmia-midox/
├── README.md
├── LICENSE                         3-Clause BSD, as upstream ALMaSS
├── NOTICE.md                       provenance: which files came from upstream, and at which commit
├── CHANGELOG.md
├── CITATION.cff                    citation metadata (GitHub reads this)
├── .zenodo.json                    archival metadata (Zenodo reads this, not the .cff)
├── Doxyfile                        builds the documentation
├── docs/                           narrative MIDox documentation
│   ├── index.md                        the documentation, and the site's front page
│   └── OsmiaFig2_Architecture.{png,svg,dot}   Figure 1, with its Graphviz source
├── src/                            source code
│   ├── Osmia/                          the model: life stages, scheduling, nests, parasitoids
│   │   ├── Osmia.{h,cpp}                   egg → larva → prepupa → pupa → cocooned adult → adult
│   │   └── Osmia_Population_Manager.{h,cpp}  scheduling, nest management, parasitoid sub-populations
│   ├── BatchALMaSS/                    ALMaSS base classes and support headers (verbatim upstream)
│   ├── Landscape/                      landscape enums (verbatim upstream, include-only)
│   └── ALMaSSDefines.h                 framework-wide compile-time definitions (verbatim upstream)
└── examples/                       example input
    ├── Osmia_Calibrated.cfg            calibrated parameterisation
    └── Osmia_FormalModel.cfg           Formal Model parameterisation
```

**Why `src/` keeps subdirectories.** The *Osmia* sources include their
dependencies by relative path — `#include "../BatchALMaSS/PopulationManager.h"`,
`#include "../ALMaSSDefines.h"`. From `src/Osmia/`, `..` is `src/`, so these
resolve as they do upstream. Flattening every source file into `src/` directly
would break them, and it would also make the files diff-incomparable with
upstream ALMaSS.

This is a documentation repository, not a place to develop the model. See
[`NOTICE.md`](NOTICE.md) for exactly which files came from upstream and at which
commit. Model development happens at
[TheALMaSS/ALMaSS_all](https://github.com/TheALMaSS/ALMaSS_all).

## Building the documentation locally

Requires [Doxygen](https://www.doxygen.nl/) 1.9.8 or later and
[Graphviz](https://graphviz.org/).

```bash
# Debian/Ubuntu
sudo apt-get install doxygen graphviz
# macOS
brew install doxygen graphviz

git clone https://github.com/TheALMaSS/osmia-midox.git
cd osmia-midox
doxygen Doxyfile
open doxygen_output/html/index.html      # xdg-open on Linux
```

The build takes about eight seconds and produces roughly 20 MB across ~1,850
files, including 751 Graphviz diagrams. **It should complete with no warnings** —
`doxygen_output/doxygen_warnings.log` is expected to be empty, and the CI
workflow fails the build if it is not. To build without diagrams, set
`HAVE_DOT = NO`.

## Running the model

The sources here are for reading, not for compiling on their own — the model is
one component of ALMaSS and needs the full framework, the landscape simulation
and its input data. To run it, build ALMaSS from
[TheALMaSS/ALMaSS_all](https://github.com/TheALMaSS/ALMaSS_all) and use one of
the configuration files in [`examples/`](examples/). The narrative
documentation's *Inputs* section describes the landscape, resource and weather
data the model expects.

## Citation

Cite the archived version, not the repository, so the reader gets the exact
state you used:

<!-- TODO after the first release: replace with the real DOI and year. -->

> Topping, C. J., Ziółkowska, E., & Duan, X. (2026). *ALMaSS Osmia bicornis
> Model — MIDox Implementation Documentation* (v1.0.0) [Software]. Zenodo.
> https://doi.org/10.5281/zenodo.XXXXXXX

[`CITATION.cff`](CITATION.cff) carries the same information in machine-readable
form, and GitHub's *Cite this repository* button reads it.

Note that Zenodo itself reads [`.zenodo.json`](.zenodo.json), **not**
`CITATION.cff` — when both files exist, Zenodo ignores the `.cff` entirely. Any
change to authors, version or DOI has to be made in both.

## Related work

| | |
|---|---|
| Formal model | Ziółkowska et al. (2023), [10.3897/fmj.4.102102](https://doi.org/10.3897/fmj.4.102102) |
| ALMaSS framework | [projects.au.dk/almass](https://projects.au.dk/almass) · [TheALMaSS/ALMaSS_all](https://github.com/TheALMaSS/ALMaSS_all) |
| Other ALMaSS model archives | [Zenodo ALMaSS community](https://zenodo.org/communities/almass/) |

## Licence

3-Clause BSD Licence, © 2017 Christopher John Topping — the same licence as
upstream ALMaSS. See [`LICENSE`](LICENSE).

## Acknowledgements

<!-- TODO: funding sources and grant numbers. -->
