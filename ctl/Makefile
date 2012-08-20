#----------------------------------------------------------------------------
#       Macros
#----------------------------------------------------------------------------
PROJECT       = ctl
BIN           = $(BTARGETDIR)ctl.exe
OBJS          = $(OBJSDIR)TConfig.o $(OBJSDIR)Smt_XML.o $(OBJSDIR)TASTConnector.o $(OBJSDIR)TConnID.o $(OBJSDIR)TCall.o $(OBJSDIR)TCallState.o $(OBJSDIR)TDevice.o $(OBJSDIR)TDeviceState.o $(OBJSDIR)TConnection.o $(OBJSDIR)TConnectionState.o $(OBJSDIR)CMUService_linux.o
BTARGETDIR    = ../../bin/
OBJSDIR       = ../../objs/ctl/
INCLUDEDIR    = -I"$(ACE_ROOT)" -I"./" -I"../smtlib/include/" -I"../smtlib/inc/libxml2-2.6.31/include/" -I"../smtlib/inc/libiconv-1.12/include/"
LIBDIR        = -L"$(ACE_ROOT)/lib" -L"../smtlib/lib/" 
CPPFLAGS      = -fPIC -O -D_REENTRANT $(INCLUDEDIR) -g
LDFLAGS       = $(LIBDIR)
LDLIBS        = -lACE -lSmtLib -lxml2 -liconv -ldl -lpthread
COMPILE.cc    = g++ $(CPPFLAGS) -c -fpermissive
LINK.cc       = g++ $(CPPFLAGS) $(LDFLAGS)

#
#----------------------------------------------------------------------------
#       Local targets
#----------------------------------------------------------------------------

$(BIN): $(OBJS)
	@test -d "$(BTARGETDIR)" || mkdir -p "$(BTARGETDIR)"
	$(LINK.cc) $(OBJS) $(LDLIBS) -o "$@"

$(OBJSDIR)TConfig.o: TConfig.cpp
	@test -d "$(OBJSDIR)" || mkdir -p "$(OBJSDIR)"	
	$(COMPILE.cc) -o "$@" TConfig.cpp

$(OBJSDIR)Smt_XML.o: Smt_XML.cpp
	$(COMPILE.cc) -o "$@" Smt_XML.cpp
	
$(OBJSDIR)TASTConnector.o: TASTConnector.cpp
	$(COMPILE.cc) -o "$@" TASTConnector.cpp
	
$(OBJSDIR)TConnID.o: TConnID.cpp
	$(COMPILE.cc) -o "$@" TConnID.cpp
	
$(OBJSDIR)TCall.o: TCall.cpp
	$(COMPILE.cc) -o "$@" TCall.cpp

$(OBJSDIR)TCallState.o: TCallState.cpp
	$(COMPILE.cc) -o "$@" TCallState.cpp

$(OBJSDIR)TDevice.o: TDevice.cpp
	$(COMPILE.cc) -o "$@" TDevice.cpp

$(OBJSDIR)TDeviceState.o: TDeviceState.cpp
	$(COMPILE.cc) -o "$@" TDeviceState.cpp
	
$(OBJSDIR)TConnection.o: TConnection.cpp
	$(COMPILE.cc) -o "$@" TConnection.cpp
	
$(OBJSDIR)TConnectionState.o: TConnectionState.cpp
	$(COMPILE.cc) -o "$@" TConnectionState.cpp

$(OBJSDIR)CMUService_linux.o: CMUService_linux.cpp
	$(COMPILE.cc) -o "$@" CMUService_linux.cpp
	
clean:
	-rm -rf $(OBJSDIR)

realclean: clean
	-rm -rf $(BIN)

