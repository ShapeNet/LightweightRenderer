include_directories(
	${CMAKE_SOURCE_DIR}/src/util
	${OPENGL_INCLUDE_PATH}
	${GLUT_INCLUDE_DIR}
	${GLEW_INCLUDE_DIR}
)

add_library (util
	readtex.c
	shaderutil.c
	showbuffer.c
	trackball.c
)

if (BUILD_SHARED_LIBS)
	install (TARGETS util DESTINATION ${LIBDIR})
endif (BUILD_SHARED_LIBS)
