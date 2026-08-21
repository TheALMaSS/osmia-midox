# ALMaSS *Osmia bicornis* MIDox

This repository documents the reviewed, post-calibration C++ implementation of the ALMaSS *Osmia bicornis* model. It is an implementation record, not a calibration repository and not a stand-alone ALMaSS executable.

The release contains:

- annotated *Osmia* source and the support headers needed for Doxygen;
- a verified Doxygen site in `doxygen_output/html/`;
- the narrative MIDox manuscript in `manuscript/`;
- the source-verified 100-parameter appendix in `supplement/`;
- figure sources and process evidence.

The source code is authoritative for implemented behaviour. The Formal Model is the conceptual comparator: Ziółkowska et al. (2023), <https://doi.org/10.3897/fmj.4.102102>. Calibration information is used only to record the provenance of fitted declaration defaults.

## Documentation status

The amended returned Doxygen build contains 104 HTML pages. The acceptance check examined 16,538 local references and found no broken file references, broken anchors, missing expected documentation terms or non-empty Doxygen warning lines.

Browse locally by opening `doxygen_output/html/index.html`. After Step 4, the public site will be https://thealmass.github.io/osmia-midox/.

## Repository layout

| Path | Contents |
|:---|:---|
| `src/` | Annotated *Osmia* code and Doxygen support headers |
| `docs/` | Doxygen front-page source and stylesheet |
| `doxygen_output/html/` | Accepted, generated documentation site |
| `manuscript/` | MIDox paper source and Figure 1 |
| `supplement/` | Source-verified parameter appendix |
| `figures/` | PNG, SVG and Graphviz figure sources |
| `tools/` | Source-reference, repair and generated-link checks |
| `evidence/` | Version, reconciliation and acceptance records |

## Rechecking the accepted site

```bash
python3 tools/verify_doxygen_output.py doxygen_output
```

The expected result is `RESULT: PASS`.

## Regenerating Doxygen

The accepted site was generated with Doxygen 1.18.0 and Graphviz 16.0.0. On Windows, run `run_doxygen.bat`; on Linux or macOS, run `./run_doxygen.sh`. The scripts carry out the source-reference check, generation, repair of Doxygen's generated index links and final local-link check.

Regeneration is not required merely to publish the accepted site. If the annotated source or `docs/index.md` changes, regenerate and replace the accepted output only after the new site passes the checks.

## Version boundary

The source package records upstream archive revision `ae3857cc39dc4003742de9e4c9efe6ac70771b25`. The publication tag, repository URL, Pages URL and Zenodo DOI remain placeholders until Step 4.

## Licence

BSD 3-Clause. See `LICENSE`.
