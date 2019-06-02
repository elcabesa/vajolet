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
./src/benchmark.cpp \
./src/bitops.cpp \
./src/book.cpp \
./src/command.cpp \
./src/data.cpp \
./src/endgame.cpp \
./src/eval.cpp \
./src/game.cpp \
./src/hashKey.cpp \
./src/vajo_io.cpp \
./src/libchess.cpp \
./src/magicmoves.cpp \
./src/Move.cpp \
./src/movegen.cpp \
./src/movepicker.cpp \
./src/parameters.cpp \
./src/perft.cpp \
./src/polyglotKey.cpp \
./src/position.cpp \
./src/search.cpp \
./src/searchData.cpp \
./src/see.cpp \
./src/thread.cpp \
./src/timeManagement.cpp \
./src/transposition.cpp \
./src/uciParameters.cpp \
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
./src/benchmark.o \
./src/bitops.o \
./src/book.o \
./src/command.o \
./src/data.o \
./src/endgame.o \
./src/eval.o \
./src/game.o \
./src/hashKey.o \
./src/vajo_io.o \
./src/libchess.o \
./src/magicmoves.o \
./src/Move.o \
./src/movegen.o \
./src/movepicker.o \
./src/parameters.o \
./src/perft.o \
./src/polyglotKey.o \
./src/position.o \
./src/search.o \
./src/searchData.o \
./src/see.o \
./src/thread.o \
./src/timeManagement.o \
./src/transposition.o \
./src/uciParameters.o \
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
./src/benchmark.d \
./src/bitops.d \
./src/book.d \
./src/command.d \
./src/data.d \
./src/endgame.d \
./src/eval.d \
./src/game.d \
./src/hashKey.d \
./src/vajo_io.d \
./src/libchess.d \
./src/magicmoves.d \
./src/Move.d \
./src/movegen.d \
./src/movepicker.d \
./src/parameters.d \
./src/perft.d \
./src/polyglotKey.d \
./src/position.d \
./src/search.d \
./src/searchData.d \
./src/see.d \
./src/thread.d \
./src/timeManagement.d \
./src/transposition.d \
./src/uciParameters.cd \
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
	g++ -std=c++1z -O3 -DNDEBUG -mbmi -mbmi2 -msse4.2 -mpopcnt -pedantic -Wall -Wextra -c -fmessage-length=0 -Isrc -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
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
	g++ -s -pthread -o $(EXE) $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(CC_DEPS)$(C++_DEPS)$(EXECUTABLES)$(OBJS)$(C_UPPER_DEPS)$(CXX_DEPS)$(CPP_DEPS)$(C_DEPS) vajolet2.exe
	-@echo ' '

.PHONY: all clean dependents
.SECONDARY:

-include ../makefile.targets
