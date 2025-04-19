# Get the main component directory from the build properties
# Assuming the user places param_table.inc in their 'main' directory
idf_build_get_property(project_dir PROJECT_DIR)
set(main_comp_dir ${project_dir}/main)

idf_component_register(
            SRCS
                nvs_config.c
                secure_level.c
            INCLUDE_DIRS
                .
            PRIV_INCLUDE_DIRS 
                ${main_comp_dir} 
            REQUIRES
                nvs_flash
           )
