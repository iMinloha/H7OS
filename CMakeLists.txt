# H7OS CMake 构建系统
# 通过 BOARD 变量切换目标板子, 自动加载对应的 HAL 层代码
# 用法: cmake -B build -DBOARD=FK743M2-IIT6 -G "MinGW Makefiles"
set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_VERSION 1)
cmake_minimum_required(VERSION 4.1)

# ============================================================
#  板子选择 —— 只需修改此变量即可切换目标板
# ============================================================
if(NOT DEFINED BOARD)
    set(BOARD "FK743M2-IIT6" CACHE STRING "Target board name (folder under HAL/)")
endif()
message(STATUS "Target Board: ${BOARD}")
message(STATUS "HAL Path:    HAL/${BOARD}")
message(STATUS "Linker:      HAL/${BOARD}/STM32H743IITX_FLASH.ld")

# ============================================================
#  交叉编译工具链
# ============================================================
set(CMAKE_C_COMPILER arm-none-eabi-gcc)
set(CMAKE_CXX_COMPILER arm-none-eabi-g++)
set(CMAKE_ASM_COMPILER  arm-none-eabi-gcc)
set(CMAKE_AR arm-none-eabi-ar)
set(CMAKE_OBJCOPY arm-none-eabi-objcopy)
set(CMAKE_OBJDUMP arm-none-eabi-objdump)
set(SIZE arm-none-eabi-size)
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

# project settings
project(H7OS C CXX ASM)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_C_STANDARD 11)

#Uncomment for hardware floating point
add_compile_definitions(ARM_MATH_CM4;ARM_MATH_MATRIX_CHECK;ARM_MATH_ROUNDING)
add_compile_options(-mfloat-abi=hard -mfpu=fpv4-sp-d16)
add_link_options(-mfloat-abi=hard -mfpu=fpv4-sp-d16)

add_compile_options(-mcpu=cortex-m7 -mthumb -mthumb-interwork)
add_compile_options(-ffunction-sections -fdata-sections -fno-common -fmessage-length=0)

# Enable assembler files preprocessing
add_compile_options($<$<COMPILE_LANGUAGE:ASM>:-x$<SEMICOLON>assembler-with-cpp>)

# ============================================================
#  优化级别
# ============================================================
if ("${CMAKE_BUILD_TYPE}" STREQUAL "Release")
    message(STATUS "Maximum optimization for speed")
    add_compile_options(-Ofast)
elseif ("${CMAKE_BUILD_TYPE}" STREQUAL "RelWithDebInfo")
    message(STATUS "Maximum optimization for speed, debug info included")
    add_compile_options(-Ofast -g)
elseif ("${CMAKE_BUILD_TYPE}" STREQUAL "MinSizeRel")
    message(STATUS "Maximum optimization for size")
    add_compile_options(-Os)
else ()
    message(STATUS "Minimal optimization, debug info included")
    add_compile_options(-Og -g)
endif ()

# ============================================================
#  板级编译宏
# ============================================================
add_definitions(-DDEBUG -DUSE_HAL_DRIVER -DSTM32H743xx)
# 将 BOARD 名中的连字符替换为下划线, 生成合法的 C 宏 (如 BOARD_FK743M2_IIT6)
string(REPLACE "-" "_" BOARD_MACRO ${BOARD})
add_definitions(-DBOARD_${BOARD_MACRO})   # 驱动 board_select.h 选择板级配置

# ============================================================
#  头文件路径
# ============================================================
# HAL 层 —— 板级 CubeMX 生成代码
include_directories(HAL/${BOARD}/Inc)
include_directories(HAL/${BOARD}/BSP)

# HAL BSP 子目录 (cpu, gpio, usart, pwm, adc, i2c, lcd, touch, ...)
file(GLOB BSP_SUBDIRS LIST_DIRECTORIES true "HAL/${BOARD}/BSP/*")
foreach(DIR ${BSP_SUBDIRS})
    if(IS_DIRECTORY ${DIR})
        include_directories(${DIR})
    endif()
endforeach()

# Platform 层 —— 统一接口 + 板级配置
include_directories(Platform)
include_directories(Platform/Board)

# 应用层头文件
include_directories(Core/Inc)
include_directories(Applications/inc)
include_directories(RAMFS/)
include_directories(RAMFS/init.d)
include_directories(Command/)

# OS 内核
include_directories(Drivers/Kernel/inc)

# STM32 HAL 库 & CMSIS (板级自包含, CubeMX 可覆盖)
include_directories(HAL/${BOARD}/Drivers/STM32H7xx_HAL_Driver/Inc)
include_directories(HAL/${BOARD}/Drivers/STM32H7xx_HAL_Driver/Inc/Legacy)
include_directories(HAL/${BOARD}/Drivers/CMSIS/Device/ST/STM32H7xx/Include)
include_directories(HAL/${BOARD}/Drivers/CMSIS/Include)

# FreeRTOS
include_directories(Middlewares/Third_Party/FreeRTOS/Source/include)
include_directories(Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS)
include_directories(Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F)

# 中间件
include_directories(Utilities/JPEG)
include_directories(FATFS/Target)
include_directories(FATFS/App)
include_directories(Middlewares/Third_Party/FatFs/src)

# USB
include_directories(USB_DEVICE/App)
include_directories(USB_DEVICE/Target)
include_directories(Middlewares/ST/STM32_USB_Device_Library/Core/Inc)
include_directories(Middlewares/ST/STM32_USB_Device_Library/Class/CDC/Inc)

# ============================================================
#  源文件
# ============================================================
# HAL 层 —— 板级 CubeMX 代码 + BSP 驱动
file(GLOB_RECURSE HAL_SOURCES "HAL/${BOARD}/*.*")
# 排除 CubeMX 生成的 main.c (真正的入口是 Core/Src/main.c)
list(FILTER HAL_SOURCES EXCLUDE REGEX ".*/Src/main\\.c$")
# 排除 Drivers/ 目录 (CMSIS & HAL_Driver 由 DRIVER_SOURCES 单独管理)
list(FILTER HAL_SOURCES EXCLUDE REGEX ".*/Drivers/.*")

# Platform 层 —— 统一接口
file(GLOB_RECURSE PLATFORM_SOURCES "Platform/*.*")

# 应用层 (main.c, freertos.c, syscalls, sysmem)
file(GLOB_RECURSE CORE_SOURCES "Core/*.*")

# 中间件 & 驱动
file(GLOB_RECURSE DRIVER_SOURCES
    "HAL/${BOARD}/Drivers/STM32H7xx_HAL_Driver/*.*"
    "HAL/${BOARD}/Drivers/CMSIS/*.*"
    "Drivers/Kernel/*.*"
)
file(GLOB_RECURSE MIDDLEWARE_SOURCES
    "Middlewares/*.*"
    "FATFS/*.*"
    "USB_DEVICE/*.*"
    "Utilities/*.*"
)

# OS 层
file(GLOB_RECURSE APP_SOURCES "Applications/*.*" "Command/*.*" "RAMFS/*.*")

# 汇总
set(SOURCES
    ${HAL_SOURCES}
    ${PLATFORM_SOURCES}
    ${CORE_SOURCES}
    ${DRIVER_SOURCES}
    ${MIDDLEWARE_SOURCES}
    ${APP_SOURCES}
)

# ============================================================
#  链接器脚本 (来自板级 HAL 目录)
# ============================================================
set(LINKER_SCRIPT ${CMAKE_SOURCE_DIR}/HAL/${BOARD}/STM32H743IITX_FLASH.ld)

add_link_options(-Wl,-gc-sections,--print-memory-usage,-Map=${PROJECT_BINARY_DIR}/${PROJECT_NAME}.map)
add_link_options(-mcpu=cortex-m7 -mthumb -mthumb-interwork)
add_link_options(-T ${LINKER_SCRIPT})

add_executable(${PROJECT_NAME}.elf ${SOURCES} ${LINKER_SCRIPT})

# ============================================================
#  后处理 —— 生成 .hex 和 .bin
# ============================================================
set(HEX_FILE ${PROJECT_BINARY_DIR}/${PROJECT_NAME}.hex)
set(BIN_FILE ${PROJECT_BINARY_DIR}/${PROJECT_NAME}.bin)

add_custom_command(TARGET ${PROJECT_NAME}.elf POST_BUILD
        COMMAND ${CMAKE_OBJCOPY} -Oihex $<TARGET_FILE:${PROJECT_NAME}.elf> ${HEX_FILE}
        COMMAND ${CMAKE_OBJCOPY} -Obinary $<TARGET_FILE:${PROJECT_NAME}.elf> ${BIN_FILE}
        COMMENT "Building ${HEX_FILE}\nBuilding ${BIN_FILE}")
