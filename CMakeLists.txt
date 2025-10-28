cmake_minimum_required(VERSION 3.21)

project(f-crm
    VERSION 0.3.0
    DESCRIPTION "CRM for Independent Contractors and Freelancers"
    LANGUAGES CXX)

add_definitions(-DF_CRM_VERSION=\"${CMAKE_PROJECT_VERSION}\")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 6.10 REQUIRED COMPONENTS Core Gui Widgets Sql)

qt_standard_project_setup()


file(GLOB_RECURSE APP_SOURCES
    CONFIGURE_DEPENDS
    src/*.cpp
    src/*.cxx
    *.cpp
)
file(GLOB_RECURSE APP_HEADERS
    CONFIGURE_DEPENDS
    src/*.h
    src/*.hpp
    include/**/*.h
    include/**/*.hpp
)
file(GLOB_RECURSE APP_FORMS
    CONFIGURE_DEPENDS
    src/*.ui
    forms/*.ui
)

qt_add_executable(f-crm
    WIN32
    MACOSX_BUNDLE
    ${APP_SOURCES}
    ${APP_HEADERS}
    ${APP_FORMS}
)

target_include_directories(f-crm
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}      # project root
)

set(UI_FORMS
    ui/mainwindow.ui
    ui/channeldialog.ui
    ui/persondialog.ui
    ui/intentdialog.ui
    ui/actiondialog.ui
    ui/actionexecutedialog.ui
    ui/documentdialog.ui
    ui/settingsdialog.ui
    ui/favoritesdialog.ui
    ui/aboutdialog.ui
)

target_sources(f-crm PRIVATE ${UI_FORMS})

set_target_properties(f-crm PROPERTIES
    AUTOUIC_SEARCH_PATHS "${CMAKE_CURRENT_SOURCE_DIR}/ui"
)

qt_add_resources(f-crm app_resources
    PREFIX "/"
    FILES
        res/icons/github.svg
        res/icons/status_shut_down.svg
        res/icons/state_succeeded.svg
        res/icons/person.svg
        res/icons/gender_male.svg
        res/icons/edit_document.svg
        res/icons/action_blocked.svg
        res/icons/status_watching.svg
        res/icons/updated_person.svg
        res/icons/4star.svg
        res/icons/status_prospect.svg
        res/icons/delete_action.svg
        res/icons/0star.svg
        res/icons/external-link.svg
        res/icons/check.svg
        res/icons/favourite.svg
        res/icons/addcompany.svg
        res/icons/2star.svg
        res/icons/addperson.svg
        res/icons/1star.svg
        res/icons/gender_unknown.svg
        res/icons/move_up.svg
        res/icons/log_general.svg
        res/icons/nofavourite.svg
        res/icons/delete.svg
        res/icons/clear_filter.svg
        res/icons/f-crm.svg
        res/icons/state_progress.svg
        res/icons/outgoing.svg
        res/icons/offer.svg
        res/icons/edit_intent.svg
        res/icons/addchannel.svg
        res/icons/updated_company.svg
        res/icons/intent.svg
        res/icons/gender_female.svg
        res/icons/incoming.svg
        res/icons/move_down.svg
        res/icons/state_terminated.svg
        res/icons/delete_document.svg
        res/icons/action_type_meeting.svg
        res/icons/mail.svg
        res/icons/ch_type_web.svg
        res/icons/status_lost.svg
        res/icons/mobile.svg
        res/icons/action_cancelled.svg
        res/icons/not_favourite.svg
        res/icons/action_waiting.svg
        res/icons/status_banned.svg
        res/icons/3star.svg
        res/icons/action_on_hold.svg
        res/icons/note.svg
        res/icons/ch_type_other.svg
        res/icons/facebook.svg
        res/icons/add_intent.svg
        res/icons/clipboard.svg
        res/icons/company.svg
        res/icons/skype.svg
        res/icons/delete_intent.svg
        res/icons/edit.svg
        res/icons/phone.svg
        res/icons/state_failed.svg
        res/icons/document.svg
        res/icons/action_type_channel.svg
        res/icons/request.svg
        res/icons/action_type_task.svg
        res/icons/action_failed.svg
        res/icons/users.svg
        res/icons/intent_type_manual.svg
        res/icons/open.svg
        res/icons/status_customer.svg
        res/icons/5star.svg
        res/icons/linkedin.svg
        res/icons/internal.svg
        res/icons/file.svg
        res/icons/proposal.svg
        res/icons/add_document.svg
        res/icons/action_ready.svg
        res/icons/add_action.svg
        res/icons/star.svg
        res/icons/home.svg
        res/icons/execute_action.svg
        res/icons/state_defined.svg
        res/icons/edit_action.svg
        res/icons/action_done.svg
        res/icons/research.svg
        res/icons/status_candidate.svg
        res/icons/activity.svg
        res/icons/reddit.svg
)

target_link_libraries(f-crm PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Sql
)

set_target_properties(f-crm PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER "eu.lastviking.f-crm"
    MACOSX_BUNDLE_BUNDLE_NAME   "f-crm"
    MACOSX_BUNDLE_INFO_STRING   "CRM for Independent Contractors and Freelancers"
    MACOSX_BUNDLE_COPYRIGHT     "Copyright (c) 2018–2025 Jarle (jgaa) Aase"
)

# On Windows, set some RC metadata (Visual Studio generators)
if (WIN32)
    set_property(TARGET f-crm PROPERTY VS_GLOBAL_CompanyName "The Last Viking LTD")
    set_property(TARGET f-crm PROPERTY VS_GLOBAL_ProductName "f-crm")
endif()

install(TARGETS f-crm
    BUNDLE DESTINATION .
    RUNTIME DESTINATION bin
)

