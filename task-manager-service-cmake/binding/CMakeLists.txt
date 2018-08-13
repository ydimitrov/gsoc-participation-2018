# Add target to project dependency list
PROJECT_TARGET_ADD(taskmanager)

    # Define project Targets
    file(GLOB sourcelist "*.c")

    # Define project Targets
    ADD_LIBRARY(${TARGET_NAME} MODULE ${sourcelist})

    # Binder exposes a unique public entry point
    SET_TARGET_PROPERTIES(${TARGET_NAME} PROPERTIES
	PREFIX "afb-"
        LABELS "BINDING"
	LINK_FLAGS  ${BINDINGS_LINK_FLAG}
        OUTPUT_NAME ${TARGET_NAME}
    )

    TARGET_LINK_LIBRARIES(${TARGET_NAME}
	    ${link_libraries}
    )
