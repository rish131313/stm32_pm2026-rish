cmake_minimum_required(VERSION 3.16)

# Отключаем проверки компилятора, чтобы не падало на микроконтроллере
set(CMAKE_TRY_COMPILE_TARGET_TYPE "STATIC_LIBRARY")
set(CMAKE_C_COMPILER_WORKS 1)
set(CMAKE_CXX_COMPILER_WORKS 1)

project(stm32_template C CXX ASM)

set(TARGET_NAME ${PROJECT_NAME})

# Если CMake Tools не передал путь к objcopy, ставим дефолтный для Ubuntu
if(NOT OBJCOPY)
    set(OBJCOPY arm-none-eabi-objcopy)
endif()
set(STFLASH st-flash)

# Папки для сборки
set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/build)
set(CMAKE_EXECUTABLE_SUFFIX ".elf")

# Флаги компилятора для STM32F103 (Cortex-M3)
set(CMAKE_C_FLAGS "-mcpu=cortex-m3 -mthumb -std=c99")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fdata-sections -ffunction-sections -mlittle-endian -g3")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fno-move-loop-invariants -fsigned-char -ffreestanding")

# Флаги линковщика (указание файла разметки памяти)
set(LINKER_FLAGS "-nostdlib -nostartfiles")
set(LINKER_FLAGS "${LINKER_FLAGS} -Xlinker -Map -Xlinker ${TARGET_NAME}.map")
set(LINKER_FLAGS "${LINKER_FLAGS} -Xlinker -T ${CMAKE_CURRENT_SOURCE_DIR}/stm32f103c8.ld")
set(LINKER_FLAGS "${LINKER_FLAGS} -Wl,--gc-sections")
set(CMAKE_C_LINK_FLAGS ${LINKER_FLAGS})

message(STATUS "Build directory: ${EXECUTABLE_OUTPUT_PATH}")

# Дефайны для STM32F10x
add_definitions(-DSTM32F103C8)
add_definitions(-DSTM32F10X_MD)

include_directories(cmsis)
include_directories(stm32)

# Исходники
set(SOURCE_FILES
    stm32/startup/startup_stm32f10x_md.c
    stm32/system_stm32f10x.c
    syscalls/syscalls.c
    lab_03/main.c)

# Копируем .gdbinit в папку сборки
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/.gdbinit)
    file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/.gdbinit DESTINATION ${EXECUTABLE_OUTPUT_PATH})
endif()

# Создаем исполняемый файл
add_executable(${TARGET_NAME} ${SOURCE_FILES})

# Пути к выходным файлам
set(BIN_TARGET "${EXECUTABLE_OUTPUT_PATH}/${TARGET_NAME}.bin")
set(HEX_TARGET "${EXECUTABLE_OUTPUT_PATH}/${TARGET_NAME}.hex")
set(ELF_TARGET "${EXECUTABLE_OUTPUT_PATH}/${TARGET_NAME}${CMAKE_EXECUTABLE_SUFFIX}")

# Команды генерации прошивок .bin и .hex
add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
    COMMAND ${OBJCOPY} -O binary ${ELF_TARGET} ${BIN_TARGET}
    COMMENT "Generating .bin firmware...")

add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
    COMMAND ${OBJCOPY} -O ihex ${ELF_TARGET} ${HEX_TARGET}
    COMMENT "Generating .hex firmware...")

# Таргет для прошивки через st-flash
add_custom_target(flash
    COMMAND ${STFLASH} write ${BIN_TARGET} 0x8000000
    DEPENDS ${TARGET_NAME}
    COMMENT "Flashing MCU via st-flash...")
