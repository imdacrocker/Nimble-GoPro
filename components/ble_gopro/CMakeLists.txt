
idf_component_register(SRCS "ble_gopro.c" "gap.c" "camera.c" "helpers.c"
                       INCLUDE_DIRS "include"
                       REQUIRES "nvs_flash" "bt" "json")
