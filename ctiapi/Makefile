#----------------------------------------------------------------------------
#       Macros
#----------------------------------------------------------------------------
PROJECT       = CMUAPI
BIN           = $(BTARGETDIR)libcti.so
OBJS          = $(OBJSDIR)TConfig.o $(OBJSDIR)TUser.o $(OBJSDIR)cmuapi.o 
BTARGETDIR    = ../../lib/
OBJSDIR       = ../../objs/ctiapi/
INCLUDEDIR    = -I"$(ACE_ROOT)" -I"./" -I"../smtlib/include/"
LIBDIR        = -L"$(ACE_ROOT)/lib" -L"../smtlib/lib/" 
CPPFLAGS      = -fPIC -O -D_REENTRANT $(INCLUDEDIR)
LDFLAGS       = $(LIBDIR)
LDLIBS        = -lACE -lSmtLib -ldl -lpthread
COMPILE.cc    = g++ $(CPPFLAGS) -c -fpermissive
LINK.cc       = g++ -shared $(CPPFLAGS) $(LDFLAGS)

#
#----------------------------------------------------------------------------
#       Local targets
#----------------------------------------------------------------------------

$(BIN): $(OBJS)
	@test -d "$(BTARGETDIR)" || mkdir -p "$(BTARGETDIR)"
	$(LINK.cc) $(OBJS) $(LDLIBS) -o "$@"
#	@cp $BIN ../TelService/CMU/lib/

$(OBJSDIR)TConfig.o: TConfig.cpp
	@test -d "$(OBJSDIR)" || mkdir -p "$(OBJSDIR)"	
	$(COMPILE.cc) -o "$@" TConfig.cpp

$(OBJSDIR)TUser.o: TUser.cpp
	$(COMPILE.cc) -o "$@" TUser.cpp
	
$(OBJSDIR)cmuapi.o: cmuapi.cpp
	$(COMPILE.cc) -o "$@" cmuapi.cpp
	
clean:
	-rm -rf $(OBJSDIR)

realclean: clean
	-rm -rf $(BIN)

