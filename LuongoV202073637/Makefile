
texfile = LuongoV202073637 Redaccion nueva
texdep = LuongoV202073637\ Redaccion\ nueva.tex
bibfile = Referencias.bib

documento: $(texdep) $(bibfile)
	latex "$(texfile).tex"
	bibtex "$(texfile).aux"
	latex "$(texfile).tex"
	latex "$(texfile).tex"
	dvipdf "$(texfile).dvi"

clean:
	rm -f *~
	rm -f "$(texfile).aux"
	rm -f "$(texfile).bbl"
	rm -f "$(texfile).blg"
	rm -f "$(texfile).dvi"
	rm -f "$(texfile).log"
	rm -f "$(texfile).out"
	rm -f "$(texfile).spl"
	rm -f "$(texfile).pdf"
	rm -f "$(texfile).tex.backup"
	rm -f $(bibfile).backup
