MULTIPAGESAMPLE=sample

all: $(MULTIPAGESAMPLE).pdf $(MULTIPAGESAMPLE).dvi $(MULTIPAGESAMPLE).ps $(MULTIPAGESAMPLE).djvu

$(MULTIPAGESAMPLE).pdf: $(MULTIPAGESAMPLE).dvi

$(MULTIPAGESAMPLE).ps: $(MULTIPAGESAMPLE).dvi
	dvips $<

$(MULTIPAGESAMPLE).dvi: $(MULTIPAGESAMPLE).tex
	tex $<

$(MULTIPAGESAMPLE).djvu: $(MULTIPAGESAMPLE).pdf
	pdf2djvu $< > $@

$(MULTIPAGESAMPLE).pdf: $(MULTIPAGESAMPLE).dvi
	dvipdf $<
clean:
	rm -f *.log *.dvi *.pdf *.djvu *.ps
