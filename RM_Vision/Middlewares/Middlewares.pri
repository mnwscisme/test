!isEmpty(MIDDLEWARES_PRI_INCLUDED):error("Middlewares.pri already included")
MIDDLEWARES_PRI_INCLUDED = 1

# Include Middlewares files
MIDDLEWARES_FILES += \
    QtSingleApplication \
    Common \
    Communication \
    Camera \
    Opencv \
    Detect

for(l, MIDDLEWARES_FILES) {
    include(./$$l/$${l}.pri)
}

