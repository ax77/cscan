################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../c_array.c \
../core_buf.c \
../core_file.c \
../core_list.c \
../core_mem.c \
../core_strutil.c \
../hashmap.c \
../ident_hash.c \
../main.c \
../token.c 

OBJS += \
./c_array.o \
./core_buf.o \
./core_file.o \
./core_list.o \
./core_mem.o \
./core_strutil.o \
./hashmap.o \
./ident_hash.o \
./main.o \
./token.o 

C_DEPS += \
./c_array.d \
./core_buf.d \
./core_file.d \
./core_list.d \
./core_mem.d \
./core_strutil.d \
./hashmap.d \
./ident_hash.d \
./main.d \
./token.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cygwin C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


