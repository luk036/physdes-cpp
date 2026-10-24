# Build rpolygon_decomposition.pdf from the Markdown source. Run from paper/.
#
# This follows the canonical IEEEtran sibling pipeline
# (D:\github\py\physdes-py\paper\Makefile):
#  - title/author/abstract live in the .md YAML front matter, so body headings
#    start at `##` and --shift-heading-level-by=-1 maps them to \section;
#  - latex.yaml sets documentclass IEEEtran (journal) and loads the float,
#    algorithm and table packages;
#  - the paper uses raw figure*/table*/algorithm* floats, because pandoc's pipe
#    tables become longtable and are illegal in two-column mode;
#  - pdflatex cannot embed SVG, so the Markdown points at the PDF twins in
#    figures/ (do not use ImageMagick; it rasterizes);
#  - references are authored manually, so no --citeproc/.bib is needed.

PANDOC ?= pandoc
MD      = rpolygon_decomposition.md
OUT     = rpolygon_decomposition.pdf
FIGS    = $(wildcard figures/*.pdf)

PDFFLAGS = -s -t latex -N --pdf-engine=pdflatex \
           --pdf-engine-opt=-interaction=nonstopmode \
           --shift-heading-level-by=-1 \
           --metadata-file=latex.yaml

.PHONY: paper pdf figures clean

paper: pdf

pdf: $(OUT)

$(OUT): $(MD) latex.yaml $(FIGS)
	$(PANDOC) $(PDFFLAGS) $(MD) -o $(OUT)

figures: $(FIGS)

figures/%.pdf: figures/%.svg
	rsvg-convert -f pdf -o $@ $<

clean:
	-@rm -f $(OUT) rpolygon_decomposition.tex
