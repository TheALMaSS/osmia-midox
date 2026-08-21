# MIDox Step 2: generate and return the Doxygen documentation

## What the supplied files do

`docs/index.md` is the editable front-page source. Doxygen converts it to
`doxygen_output/html/index.html`; do not edit the generated HTML directly. All source links on the
page use fully qualified Doxygen identifiers rather than generated filenames.

`Doxyfile` documents the four Osmia implementation files and enables the source browser, class and
collaboration diagrams, call graphs, tree navigation and search. It writes warnings to
`doxygen_output/doxygen_warnings.log`.

## Windows run

1. Place the documented package in the complete ALMaSS working environment if that is where Doxygen
   and Graphviz are installed.
2. Open a command prompt in the repository root.
3. Run `run_doxygen.bat`.
4. Open `doxygen_output\html\index.html` and inspect the front page, navigation, representative class
   pages, source links and diagrams.
5. Return `Osmia_Doxygen_Return_Package.zip` without changing its contents.

The script first removes the previous generated `doxygen_output` directory so obsolete HTML pages
cannot contaminate a rerun. It then records Doxygen and Graphviz versions, builds the HTML, repairs a known Doxygen 1.18
alphabet-index linking defect, checks every local file and anchor, and packages the HTML and reports.
The repair affects only Doxygen-generated index links; source-documentation links are checked
separately before the build. Warnings do not prevent the initial HTML from being generated because
they must be reviewed and corrected iteratively.

## Linux or macOS run

Run `sh run_doxygen.sh` from the repository root and return the same ZIP, or the complete
`doxygen_output` directory if `zip` is unavailable.

## Review boundary

The first successful generation does not complete Step 2. The returned package is checked for
warnings, unresolved links, missing pages, graph rendering and agreement between the front page and
the technical reference. Corrected inputs are then supplied for another run if necessary.
