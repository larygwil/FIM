# $Id: Makefile.am 1401 2017-07-16 19:50:17Z dezperado $

subdir = src
AUTOMAKE_OPTIONS = no-dependencies dejagnu
#AUTOMAKE_OPTIONS+= nostdinc
SUBDIRS = 
check_PROGRAMS = fim
bin_PROGRAMS = fim
bin_SCRIPTS = fimgs
doc_DATA = fimrc

LFLAGS = -+ --nounistd
YFLAGS = -v -d

#lex.yy.cc : lex.lex
lex.yy.cc: lex.lex yacc.tab.cpp
	$(LEX) $(LFLAGS) $<
	@$(ECHO) "Applying a horrible hack to avoid the flex non-throwing isatty() redeclaration in 'extern "C" int isatty (int );'"
	$(SED) -i 's/^.*extern.*isatty.*int.*$$//g' $@

yacc.tab.hpp: yacc.tab.cpp
yacc.tab.cpp: yacc.ypp lex.lex
	LC_ALL=C $(YACC) $(YFLAGS) $< -o $@

FIM_LIB_OBJECTS = yacc.tab.o lex.yy.o

conf.h:       fimrc
	$(ECHO) 'const fim_char_t * const FIM_DEFAULT_CONFIG_FILE_CONTENTS =' > $@
	$(SED)  's/\\/\\\\/g;s/"/\\\"/g;s/^/"/g;s/$$/\\n"/g;' $< | $(GREP) -v '^.###' >> $@
	$(ECHO) '"";' >> $@

grammar.h: yacc.tab.cpp yacc.output
	$(ECHO) 'const fim_char_t * const FIM_DEFAULT_GRAMMAR_FILE_CONTENTS =' > $@
	$(AWK) -f $(top_srcdir)/scripts/maintenance/yacc2grammar.awk yacc.output | $(CPP) -include $(top_srcdir)/scripts/maintenance/yacc2grammar.h  | $(GREP) -v '^#' | $(SED) 's/"/\\\"/g;s/^/"/g;s/$$/\\n"/g;' >> $@
	$(ECHO) '"";' >> $@

examples.h: ../scripts/example/oneline.fim
	$(ECHO) 'const fim_char_t * const FIM_DEFAULT_EXAMPLE_FILE_CONTENTS =' > $@
	$(SED) 's/"/\\\"/g;s/^/"/g;s/$$/\\n"/g;' $< >> $@
	$(ECHO) '"";' >> $@

CommandConsole-help.o: CommandConsole-help.cpp help-acm.cpp help.cpp examples.h grammar.h
if FIM_WANT_HARDCODED_CONSOLEFONT
FontServer.o: default_font_byte_array.h
endif
SDLDevice.o: default_icon_byte_array.h

# in the following : for some freebsd oddity, i was forced to replace ..0-9_]\+ with ..0-9_]*
version.h:	../config.h
	$(GREP) define $< | $(GREP) FIM | LC_ALL=C $(SORT) | $(SED) 's/^#//g;s/define //g;s/^\([A-Za-z0-9_]*\).*$$/+\1  /g;s/^/\"/g;s/$$/\"/g' > $@
	$(ECHO) "\"\n\"" >> $@
	$(GREP) define $< | $(GREP) FIM.*'"' | LC_ALL=C $(SORT) | $(SED) 's/^#//g;s/define //g;s/^\([A-Za-z0-9_]*\) "\(.*\)"$$/"\1 = \\"\2\\"\\n"/g'   >> $@
	$(GREP) undef $< | $(GREP) FIM | LC_ALL=C $(SORT) | $(SED) 's/^#//g;s/.*undef //g;s/^\([A-Za-z0-9_]*\).\+$$/-\1  /g;s/^/\"/g;s/$$/\"/g' >> $@
	$(ECHO)  '"\n'CXXFLAGS=$(CXXFLAGS)'\n"' >> $@

help.cpp: fim.h
	$(ECHO) -e "#ifdef FIM_WANT_INLINE_HELP\n" > $@
	$(GREP)   '^#define FIM_VID' $< | $(SED) 's/^#define //g;s/\(^[A-Z0-9_]\+\)\s\+\("[^ ]*"\)\s*\/\*\(.*\)\*\/$$/fim_var_help_db[\1]=\3;/g' >> $@
	$(ECHO) '#endif /* FIM_WANT_INLINE_HELP */' >> $@

help-acm.cpp: fim.h
	$(ECHO) -e "//#ifdef FIM_WANT_INLINE_HELP\n" > $@
	$(ECHO) -e '#define FIM_AUTOCOMMANDS_LIST \' >> $@
	$(GREP)   '^#define FIM_ACM' $< | $(SED) 's/^#define //g;s/\(^[A-Z0-9_]\+\)\s\+\("[^ ]*"\)\s*\/\*\(.*\)\*\/$$/\1", "/g'|tr '\n' ' ' >> $@
	$(ECHO)  ' '>> $@
	$(ECHO) '//#endif' >> $@

b2ba: b2ba.c
	$(CC) -o $@ $<

if FIM_WANT_CUSTOM_HARDCODED_CONSOLEFONT
default_font_byte_array.h: $(FIM_CUSTOM_HARDCODED_CONSOLEFONT) b2ba
	./b2ba < $< > $@
else
default_font_byte_array.h:   ../var/fonts/Lat15-Terminus16.psf b2ba
	./b2ba < $< > $@
endif

# the following sources are built at make time
# old bison versions could have problems with this (e.g.: generating yacc.tab.cpp.h instead of yacc.tab.hpp)
BUILT_SOURCES = lex.yy.cc yacc.tab.cpp conf.h yacc.tab.hpp 

# this hook triggers on 'make dist'
dist-hook:
	-for file in $(BUILT_SOURCES) ; do $(RM) -f $(distdir)/$$file ; done

EXTRA_fim_SOURCES = 
# The following files are marked as source files by configure.ac
EXTRA_fim_SOURCES += FbiStuffGif.cpp
EXTRA_fim_SOURCES += FbiStuffJasPer.cpp
EXTRA_fim_SOURCES += FbiStuffJpeg.cpp
EXTRA_fim_SOURCES += FbiStuffTiff.cpp
EXTRA_fim_SOURCES += FbiStuffPng.cpp
EXTRA_fim_SOURCES += FbiStuffDjvu.cpp
EXTRA_fim_SOURCES += FbiStuffMagick.cpp
EXTRA_fim_SOURCES += FbiStuffPdf.cpp
EXTRA_fim_SOURCES += FbiStuffPs.cpp
EXTRA_fim_SOURCES += Imlib2Device.cpp
EXTRA_fim_SOURCES += FbiStuffXyz.cpp
EXTRA_fim_SOURCES += FbiStuffPcx.cpp
EXTRA_fim_SOURCES += FbiStuffBmp.cpp

CLEANFILES=$(BUILT_SOURCES) yacc.output b2ba

#CPP_FLAGS="$CPP_FLAGS -D X_DISPLAY_MISSING"
#AM_CPPFLAGS=
fim_DEPENDENCIES = @FIM_LIB_OBJECTS@ @LIBOBJS@ $(INTLDEPS)
fim_LDADD = @FIM_LIB_OBJECTS@ @LIBOBJS@ $(FIMLIBS)
#fim_LDADD = @FIM_LIB_OBJECTS@ @LIBOBJS@ $(FIMLIBS) $(LEXLIB)
fim_SOURCES  = $(BUILT_SOURCES) \
	$(srcdir)/fim.cpp \
	$(srcdir)/fim.h \
	$(srcdir)/fim_types.h \
	$(srcdir)/fim_limits.h \
	$(srcdir)/fim_plugin.h \
	$(srcdir)/fim_plugin.cpp \
	$(srcdir)/fim_wrappers.h \
	$(srcdir)/default_font_byte_array.h \
	$(srcdir)/default_icon_byte_array.h \
	$(srcdir)/AADevice.h \
	$(srcdir)/Benchmarkable.h \
	$(srcdir)/CACADevice.h \
	$(srcdir)/SDLDevice.h \
	$(srcdir)/Arg.h \
	$(srcdir)/Browser.h \
	$(srcdir)/Cache.h \
	$(srcdir)/Command.h \
	$(srcdir)/CommandConsole.h \
	$(srcdir)/DebugConsole.h \
	$(srcdir)/DisplayDevice.h \
	$(srcdir)/DummyDisplayDevice.h \
	$(srcdir)/FbiStuff.h \
	$(srcdir)/FbiStuffList.h \
	$(srcdir)/FbiStuffFbtools.h \
	$(srcdir)/FbiStuffLoader.h \
	$(srcdir)/FontServer.h \
	$(srcdir)/FramebufferDevice.h \
	$(srcdir)/Imlib2Device.h \
	$(srcdir)/Image.h \
	$(srcdir)/Namespace.h \
	$(srcdir)/Var.h \
	$(srcdir)/Viewport.h \
	$(srcdir)/FimWindow.h \
	$(srcdir)/common.h \
	$(srcdir)/fim_interpreter.h \
	$(srcdir)/conf.h \
	$(srcdir)/help.cpp \
	$(srcdir)/help-acm.cpp \
	$(srcdir)/version.h \
	$(srcdir)/grammar.h \
	$(srcdir)/examples.h \
	$(srcdir)/fim.h \
	$(srcdir)/fim_stream.h \
	$(srcdir)/fim_stream.cpp \
	$(srcdir)/fimgs \
	$(srcdir)/lex.h \
	$(srcdir)/readline.h \
	$(srcdir)/readline.cpp \
	$(srcdir)/fim_string.h \
	$(srcdir)/AADevice.cpp \
	$(srcdir)/CACADevice.cpp \
	$(srcdir)/SDLDevice.cpp \
	$(srcdir)/Arg.cpp \
	$(srcdir)/Browser.cpp \
	$(srcdir)/Cache.cpp \
	$(srcdir)/Command.cpp \
	$(srcdir)/CommandConsole.cpp \
	$(srcdir)/CommandConsole-cmd.cpp \
	$(srcdir)/CommandConsole-var.cpp \
	$(srcdir)/CommandConsole-init.cpp \
	$(srcdir)/CommandConsole-help.cpp \
	$(srcdir)/DebugConsole.cpp \
	$(srcdir)/DisplayDevice.cpp \
	$(srcdir)/FbiStuff.cpp \
	$(srcdir)/FbiStuffFbtools.cpp \
	$(srcdir)/FbiStuffLoader.cpp \
	$(srcdir)/FbiStuffPpm.cpp \
	$(srcdir)/FbiStuffBit1.cpp \
	$(srcdir)/FbiStuffBit24.cpp \
	$(srcdir)/FbiStuffText.cpp \
	$(srcdir)/FbiStuffMatrixMarket.cpp \
	$(srcdir)/FontServer.cpp \
	$(srcdir)/FramebufferDevice.cpp \
	$(srcdir)/Image.cpp \
	$(srcdir)/Namespace.cpp \
	$(srcdir)/Var.cpp \
	$(srcdir)/Viewport.cpp \
	$(srcdir)/FimWindow.cpp \
	$(srcdir)/common.cpp \
	$(srcdir)/fim_interpreter.cpp \
	$(srcdir)/fim_string.cpp

Command.o: Command.h
Viewport.o: Viewport.h
Browser.o: Browser.h
Image.o: Image.h
CommandConsole.o: defaultConfiguration.o
CommandConsole-init.o: conf.h defaultConfiguration.o
help.o: conf.h defaultConfiguration.o
fim.o: version.h help.cpp grammar.h examples.h
Var.o: help.cpp
fim.cpp: version.h help.cpp grammar.h examples.h
lex.yy.o: lex.yy.cc

# regex.c is work in progress and awaits for integration
EXTRA_DIST = lex.lex yacc.ypp fimrc defaultConfiguration.cpp fimgs \
	testdir/*ok testdir/*in testdir/Makefile \
	testsuite/*/* \
	regex.c b2ba.c

RUNTESTDEFAULTFLAGS = --tool ./fim FIM=$$srcdir/./fim  --srcdir $$srcdir/testsuite
#RUNTESTDEFAULTFLAGS = --all --tool ./fim FIM=$$srcdir/./fim --tool_opts=$$srcdir/../media/fim.png --srcdir $$srcdir/testsuite


