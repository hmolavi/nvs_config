# Get the main component directory from the build properties
# Assuming the user places param_table.inc in their 'main' directory
idf_build_get_property(project_dir PROJECT_DIR)
set(main_comp_dir ${project_dir}/main)

set(NVS_CONFIG_SRCS
    src/nvs_config.c
    src/secure_level.c)

set(NVS_CONFIG_PRIV_REQUIRES
    esp_timer)

if(CONFIG_NVS_CONFIG_CONSOLE_ENABLED)
    list(APPEND NVS_CONFIG_SRCS src/nvs_config_console.c)
    list(APPEND NVS_CONFIG_PRIV_REQUIRES console)
endif()

idf_component_register(
            SRCS
                ${NVS_CONFIG_SRCS}
            INCLUDE_DIRS
                include
            PRIV_INCLUDE_DIRS
                src
                ${main_comp_dir}
            REQUIRES
                nvs_flash
            PRIV_REQUIRES
                ${NVS_CONFIG_PRIV_REQUIRES}
           )
