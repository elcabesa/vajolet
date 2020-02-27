################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

EXE = Vajolet

RM := rm -rf

# All of the sources participating in the build are defined here

C_UPPER_SRCS := 
CXX_SRCS := 
C++_SRCS := 
OBJ_SRCS := 
CC_SRCS := 
ASM_SRCS := 
CPP_SRCS := \
./src/utility/benchmark.cpp \
./src/misc/bitops.cpp \
./src/book/book.cpp \
./src/uci/command.cpp \
./src/misc/data.cpp \
./src/position/endgame.cpp \
./src/position/eval.cpp \
./src/search/game.cpp \
./src/position/hashKey.cpp \
./src/misc/vajo_io.cpp \
./src/libchess.cpp \
./src/movegen/magicmoves.cpp \
./src/movegen/move.cpp \
./src/movegen/movegen.cpp \
./src/movegen/movepicker.cpp \
./src/parameters.cpp \
./src/utility/perft.cpp \
./src/book/polyglotKey.cpp \
./src/position/position.cpp \
./src/search/rootMovesToBeSearched.cpp \
./src/search/search.cpp \
./src/search/searchData.cpp \
./src/search/searchLogger.cpp \
./src/search/see.cpp \
./src/search/thread.cpp \
./src/search/timeManagement.cpp \
./src/search/transposition.cpp \
./src/uci/uciOption.cpp \
./src/uci/uciOutput.cpp \
./src/uci/uciParameters.cpp \
./src/vajolet.cpp \
./src/syzygy/syzygy.cpp \
./src/syzygy/tbCommonData.cpp \
./src/syzygy/tbfile.cpp \
./src/syzygy/tbpairs.cpp \
./src/syzygy/tbtable.cpp \
./src/syzygy/tbtableDTZ.cpp \
./src/syzygy/tbtables.cpp \
./src/syzygy/tbtableWDL.cpp \
./src/syzygy/tbvalidater.cpp 

C_SRCS := 
S_UPPER_SRCS := 
O_SRCS := 
CC_DEPS := 
C++_DEPS := 
EXECUTABLES := 
OBJS :=  \
./src/utility/benchmark.o \
./src/misc/bitops.o \
./src/book/book.o \
./src/uci/command.o \
./src/misc/data.o \
./src/position/endgame.o \
./src/position/eval.o \
./src/search/game.o \
./src/position/hashKey.o \
./src/misc/vajo_io.o \
./src/libchess.o \
./src/movegen/magicmoves.o \
./src/movegen/move.o \
./src/movegen/movegen.o \
./src/movegen/movepicker.o \
./src/parameters.o \
./src/utility/perft.o \
./src/book/polyglotKey.o \
./src/position/position.o \
./src/search/rootMovesToBeSearched.o \
./src/search/search.o \
./src/search/searchData.o \
./src/search/searchLogger.o \
./src/search/see.o \
./src/search/thread.o \
./src/search/timeManagement.o \
./src/search/transposition.o \
./src/uci/uciOption.o \
./src/uci/uciOutput.o \
./src/uci/uciParameters.o \
./src/vajolet.o \
./src/syzygy/syzygy.o \
./src/syzygy/tbCommonData.o \
./src/syzygy/tbfile.o \
./src/syzygy/tbpairs.o \
./src/syzygy/tbtable.o \
./src/syzygy/tbtableDTZ.o \
./src/syzygy/tbtables.o \
./src/syzygy/tbtableWDL.o \
./src/syzygy/tbvalidater.o 
C_UPPER_DEPS := 
CXX_DEPS := 
CPP_DEPS := \
./src/utility/benchmark.d \
./src/misc/bitops.d \
./src/book/book.d \
./src/uci/command.d \
./src/misc/data.d \
./src/position/endgame.d \
./src/position/eval.d \
./src/search/game.d \
./src/position/hashKey.d \
./src/misc/vajo_io.d \
./src/libchess.d \
./src/movegen/magicmoves.d \
./src/movegen/move.d \
./src/movegen/movegen.d \
./src/movegen/movepicker.d \
./src/parameters.d \
./src/utility/perft.d \
./src/book/polyglotKey.d \
./src/position/position.d \
./src/search/rootMovesToBeSearched.d \
./src/search/search.d \
./src/search/searchData.d \
./src/search/searchLogger.d \
./src/search/see.d \
./src/search/thread.d \
./src/search/timeManagement.d \
./src/search/transposition.d \
./src/uci/uciOption.d \
./src/uci/uciOutput.d \
./src/uci/uciParameters.d \
./src/vajolet.d \
./src/syzygy/syzygy.d \
./src/syzygy/tbCommonData.d \
./src/syzygy/tbfile.d \
./src/syzygy/tbpairs.d \
./src/syzygy/tbtable.d \
./src/syzygy/tbtableDTZ.d \
./src/syzygy/tbtables.d \
./src/syzygy/tbtableWDL.d \
./src/syzygy/tbvalidater.d 
C_DEPS := 

# Each subdirectory must supply rules for building sources it contributes
%.o: ./%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -std=c++1z -O3 -DNDEBUG -msse4.2 -m64 -mpopcnt -pedantic -Wall -Wextra -c -fmessage-length=0 -Isrc -Isrc/book -Isrc/misc -Isrc/movegen -Isrc/position -Isrc/search -Isrc/uci -Isrc/utility -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

# Every subdirectory with source files must be described here
SUBDIRS := \
. \
syzygy \

USER_OBJS :=

LIBS :=



ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(CC_DEPS)),)
-include $(CC_DEPS)
endif
ifneq ($(strip $(C++_DEPS)),)
-include $(C++_DEPS)
endif
ifneq ($(strip $(C_UPPER_DEPS)),)
-include $(C_UPPER_DEPS)
endif
ifneq ($(strip $(CXX_DEPS)),)
-include $(CXX_DEPS)
endif
ifneq ($(strip $(CPP_DEPS)),)
-include $(CPP_DEPS)
endif
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 

# All Target
all: vajolet

# Tool invocations
vajolet: $(OBJS) $(USER_OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: MinGW C++ Linker'
	g++ -s -Wl,--whole-archive -lpthread -Wl,--no-whole-archive -static -o $(EXE) $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(CC_DEPS)$(C++_DEPS)$(EXECUTABLES)$(OBJS)$(C_UPPER_DEPS)$(CXX_DEPS)$(CPP_DEPS)$(C_DEPS) $(EXE)
	-@echo ' '

.PHONY: all clean dependents
.SECONDARY:

-include ../makefile.targets
