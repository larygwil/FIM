# $LastChangedDate: 2018-01-09 23:51:46 +0100 (Tue, 09 Jan 2018) $

HTMLDOCS=FIM.html fim.man.html  fimgs.man.html fimrc.man.html

CLEANFILES= doctags *.o  tags $(HTMLDOCS)

EXTRA_DIST = FIM.TXT fim.man doctags.c fim-stylesheet.css ./vim2html.pl fimgs.man fimrc.man fim.man.in fimgs.man.in fimrc.man.in
#MAN2HTML=man2html
MAN2HTML=$(MAN) --html=cat
MAN2HTMLS = sed 's/&minus;/-/g'

man1_MANS = fim.man fimgs.man
man5_MANS = fimrc.man

# html stuff is installed in the documentation directories, too
htmldir = $(docdir)
dist_html_DATA = $(HTMLDOCS)
dist_doc_DATA = FIM.TXT

all:	$(HTMLDOCS) $(EXTRA_DIST)

FIM.html fim-stylesheet.css: doctags FIM.TXT
	./doctags $(srcdir)/FIM.TXT > tags ; $(srcdir)/vim2html.pl tags $(srcdir)/FIM.TXT

fim.man:
	make doc/fim.man -C $(top_srcdir)/
	
fim.man.html: fim.man
	$(MAN2HTML) ./$< | $(MAN2HTMLS) >  $@ || $(ECHO) "$(ERRMSG) $(MAN2HTML) $< >  $@"

fimrc.man.html: fimrc.man
	$(MAN2HTML) ./$< | $(MAN2HTMLS) >  $@ || $(ECHO) "$(ERRMSG) $(MAN2HTML) $< >  $@"

ERRMSG="No $(MAN2HTML) at build time : please install $(MAN2HTML) and reissue :"

fimgs.man.html: fimgs.man
	$(MAN2HTML) ./$< | $(MAN2HTMLS) >  $@ || $(ECHO) "$(ERRMSG) $(MAN2HTML) $< >  $@"

doctags: doctags.c
	        $(CC) -o $@ $<

#clean:
#	        $(RM) -f  doctags.o doctags FIM.html tags fim-stylesheet.css fim.man.html fimgs.man.html

