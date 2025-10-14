set(QTCLIENT_SOURCES
    main.cpp
    core/ApplicationContext.cpp
    network/NetworkClient.cpp
    utils/JsonUtils.cpp
    ui/LoginDialog.cpp
    ui/MainWindow.cpp
    ui/tabs/DashboardTab.cpp
    ui/tabs/ProductsTab.cpp
    ui/tabs/CartTab.cpp
    ui/tabs/OrdersTab.cpp
    ui/tabs/AdminTab.cpp
    ui/tabs/NotificationsTab.cpp
    ui/tabs/CouponsTab.cpp
    ui/dialogs/AddressDialog.cpp
    ui/dialogs/PaymentDialog.cpp
)

add_executable(emshop_qtclient
    ${QTCLIENT_SOURCES}
)

target_include_directories(emshop_qtclient
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(emshop_qtclient
    PRIVATE
        ${QT_LIBS}
)

if (MSVC)
    target_compile_options(emshop_qtclient PRIVATE /Zc:__cplusplus /permissive-)
endif()

# Output binary next to source tree build directory for convenience
set_target_properties(emshop_qtclient PROPERTIES
    OUTPUT_NAME "emshop_qtclient"
)
