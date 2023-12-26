include_directories(/usr/include/**)
# Add the source files for the utilities library
set(source_files
    model/utilities/sumo_xml_parser_custom.cc
    model/application/stopWarningClient.cc
    model/application/stopWarningServer.cc
    model/helper/stopWarningClient-helper.cc
    model/helper/stopWarningServer-helper.cc
    model/Facilities/caBasicService_custom.cc
    model/Facilities/denBasicService_custom.cc
    model/Facilities/asn_utils.cc
    model/Facilities/denData.cc
    model/Facilities/ITSSOriginatingTableEntry.cc
    model/Facilities/ITSSReceivingTableEntry.cc
    model/utilities/utils.cc
    model/plexe/plexe.cc
    model/plexe/plexe_utils.cc
)

set(header_files
    model/utilities/sumo_xml_parser_custom.h
    model/application/stopWarningClient.h
    model/application/stopWarningServer.h
    model/helper/stopWarningClient-helper.h
    model/helper/stopWarningServer-helper.h
    model/Facilities/caBasicService_custom.h
    model/Facilities/denBasicService_custom.h
    model/Facilities/asn_utils.h
    model/Facilities/denData.h
    model/Facilities/ITSSOriginatingTableEntry.h
    model/Facilities/ITSSReceivingTableEntry.h
    model/utilities/utils.h
    model/plexe/plexe.h
    model/plexe/plexe_utils.h
)

set(test_sources
)

build_lib(
  LIBNAME custom
  SOURCE_FILES ${source_files}
  HEADER_FILES ${header_files}
  LIBRARIES_TO_LINK
    ${libcore}
    ${libfd-net-device}
    ${libwifi}
    ${libapplications}
    ${libmobility}
    ${libautomotive}
    ${libconfig-store}
    ${libstats}
    ${libgps-tc}
    ${libnr}
    ${libinternet}
    ${libvehicle-visualizer}
    ${libtraci}
    ${libxml2}
  TEST_SOURCES ${test_sources}
)

# 定义源文件列表
set(SOURCE_FILES
    "${CMAKE_SOURCE_DIR}/src/custom/model/utilities/sumo_xml_parser_custom.h"
    "${CMAKE_SOURCE_DIR}/src/custom/model/application/stopWarningClient.h"
    "${CMAKE_SOURCE_DIR}/src/custom/model/application/stopWarningServer.h"
    "${CMAKE_SOURCE_DIR}/src/custom/model/helper/stopWarningClient-helper.h"
    "${CMAKE_SOURCE_DIR}/src/custom/model/helper/stopWarningServer-helper.h"
    "${CMAKE_SOURCE_DIR}/src/custom/model/Facilities/caBasicService_custom.h"
    "${CMAKE_SOURCE_DIR}/src/custom/model/Facilities/denBasicService_custom.h"
    "${CMAKE_SOURCE_DIR}/src/custom/model/Facilities/asn_utils.h"
    "${CMAKE_SOURCE_DIR}/src/custom/model/Facilities/denData.h"
    "${CMAKE_SOURCE_DIR}/src/custom/model/Facilities/ITSSOriginatingTableEntry.h"
    "${CMAKE_SOURCE_DIR}/src/custom/model/Facilities/ITSSReceivingTableEntry.h"
    "${CMAKE_SOURCE_DIR}/src/custom/model/utilities/utils.h"
    "${CMAKE_SOURCE_DIR}/src/custom/model/plexe/plexe.h"
    "${CMAKE_SOURCE_DIR}/src/custom/model/plexe/plexe_utils.h"
)

# 定义符号链接目标列表
set(SYMLINK_TARGETS
    "${CMAKE_BINARY_DIR}/include/ns3/sumo_xml_parser_custom.h"
    "${CMAKE_BINARY_DIR}/include/ns3/stopWarningClient.h"
    "${CMAKE_BINARY_DIR}/include/ns3/stopWarningServer.h"
    "${CMAKE_BINARY_DIR}/include/ns3/stopWarningClient-helper.h"
    "${CMAKE_BINARY_DIR}/include/ns3/stopWarningServer-helper.h"
    "${CMAKE_BINARY_DIR}/include/ns3/caBasicService_custom.h"
    "${CMAKE_BINARY_DIR}/include/ns3/denBasicService_custom.h"
    "${CMAKE_BINARY_DIR}/include/ns3/asn_utils.h"
    "${CMAKE_BINARY_DIR}/include/ns3/denData.h"
    "${CMAKE_BINARY_DIR}/include/ns3/ITSSOriginatingTableEntry.h"
    "${CMAKE_BINARY_DIR}/include/ns3/ITSSReceivingTableEntry.h"
    "${CMAKE_BINARY_DIR}/include/ns3/utils.h"
    "${CMAKE_BINARY_DIR}/include/ns3/plexe.h"
    "${CMAKE_BINARY_DIR}/include/ns3/plexe_utils.h"
)

# 定义一个函数来创建符号链接
function(create_symlink source dest)
    execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink ${source} ${dest})
endfunction(create_symlink)

# 循环遍历文件列表并创建符号链接
list(LENGTH SOURCE_FILES num_files)
math(EXPR num_files "${num_files} - 1")
foreach(i RANGE ${num_files})
    list(GET SOURCE_FILES ${i} source_file)
    list(GET SYMLINK_TARGETS ${i} symlink_target)
    create_symlink(${source_file} ${symlink_target})
endforeach()
