file(GLOB_RECURSE srcs "*.c")

idf_component_register(
    SRCS "${srcs}"
    INCLUDE_DIRS "." "ble_demo"
    PRIV_REQUIRES svc
)
