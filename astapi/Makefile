#----------------------------------------------------------------------------
#       Macros
#----------------------------------------------------------------------------
PROJECT       = Asterisk api
BIN           = $(BTARGETDIR)libastapi.so
OBJS          = $(OBJSDIR)Smt_XML.o $(OBJSDIR)AstApi.o $(OBJSDIR)TBaseDevice.o $(OBJSDIR)TAsteriskDevice.o
BTARGETDIR    = ../../lib/
OBJSDIR       = ../../objs/AstApi/
INCLUDEDIR    = -I"$(ACE_ROOT)" -I"./" -I"../smtlib/include/" -I"../smtlib/inc/" -I"../smtlib/inc/libxml2-2.6.31/include/" -I"../ctiapi/include/"
LIBDIR        = -L"$(ACE_ROOT)/lib" -L"../smtlib/lib/" -L"../../lib/"
CPPFLAGS      = -fPIC -O -D_REENTRANT $(INCLUDEDIR)
LDFLAGS       = $(LIBDIR)
LDLIBS        = -lACE -lSmtLib -lxml2 -liconv -ldl -lpthread -lcti
COMPILE.cc    = g++ $(CPPFLAGS) -c -fpermissive
LINK.cc       = g++ -shared $(CPPFLAGS) $(LDFLAGS)

#
#----------------------------------------------------------------------------
#       Local targets
#----------------------------------------------------------------------------

$(BIN): $(OBJS)
	@test -d "$(BTARGETDIR)" || mkdir -p "$(BTARGETDIR)"
	$(LINK.cc) $(OBJS) $(LDLIBS) -o "$@"

$(OBJSDIR)Smt_XML.o: Smt_XML.cpp
	@test -d "$(OBJSDIR)" || mkdir -p "$(OBJSDIR)"	
	$(COMPILE.cc) -o "$@" Smt_XML.cpp

$(OBJSDIR)AstApi.o: AstApi.cpp
	$(COMPILE.cc) -o "$@" AstApi.cpp
		
$(OBJSDIR)TBaseDevice.o: TBaseDevice.cpp
	$(COMPILE.cc) -o "$@" TBaseDevice.cpp
	
$(OBJSDIR)TAsteriskDevice.o: TAsteriskDevice.cpp
	$(COMPILE.cc) -o "$@" TAsteriskDevice.cpp
	
clean:
	-rm -rf $(OBJSDIR)

realclean: clean
	-rm -rf $(BIN)

