cmake_minimum_required(VERSION 2.8.3)
project(mantis)
set(CMAKE_CXX_FLAGS "-std=c++11 ${CMAKE_CXX_FLAGS}")

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

find_package(catkin REQUIRED COMPONENTS
  cv_bridge
  image_transport
  roscpp
  sensor_msgs
  geometry_msgs
  std_msgs
  dynamic_reconfigure
  cmake_modules
  tf
  message_generation
)

find_package(OpenCV REQUIRED)

find_package(Eigen3 REQUIRED)

find_package(GLUT REQUIRED)
find_package(GLEW REQUIRED)
find_package(OpenGL REQUIRED)

include_directories(
	${OpenCV_INCLUDE_DIRS}
	include
	include/mantis
	${catkin_INCLUDE_DIRS}
	${OpenGL_INCLUDE_DIRS}
 	${GLUT_INCLUDE_DIRS}
 	${GLEW_INCLUDE_DIRS}
	${Eigen_INCLUDE_DIRS}
)


# add the service files
add_service_files(
	FILES
	mantisService.srv
)

generate_messages(
	DEPENDENCIES
	std_msgs
	geometry_msgs
	sensor_msgs
	
)

catkin_package(
   INCLUDE_DIRS include
#  LIBRARIES pauvsi_vio
# CATKIN_DEPENDS cv_bridge image_transport roscpp sensor_msgs std_msgs tf
#  DEPENDS system_lib
   DEPENDS opencv
)

add_library(markovPlane include/mantis3/Markov.h)
set_target_properties(markovPlane PROPERTIES LINKER_LANGUAGE CXX)
target_link_libraries(markovPlane ${catkin_LIBRARIES} ${OpenCV_LIBRARIES})

add_library(mantisGridRenderer include/mantis3/GridRenderer.cpp)
target_link_libraries(mantisGridRenderer ${catkin_LIBRARIES} ${OpenCV_LIBRARIES} ${OPENGL_LIBRARIES} ${GLUT_LIBRARY} ${GLEW_LIBRARY})

add_library(clusterer include/mantis3/PoseClusterer.cpp)
target_link_libraries(clusterer ${catkin_LIBRARIES} ${OpenCV_LIBRARIES})

add_library(rpoly include/mantis3/RobustPlanarPose/Rpoly.cpp)
target_link_libraries(rpoly ${catkin_LIBRARIES} ${OpenCV_LIBRARIES})

add_library(rpp include/mantis3/RobustPlanarPose/RPP.cpp)
target_link_libraries(rpp ${catkin_LIBRARIES} ${OpenCV_LIBRARIES} rpoly)

add_library(poseEstimation include/mantis3/CoPlanarPoseEstimator.cpp)
target_link_libraries(poseEstimation ${catkin_LIBRARIES} ${OpenCV_LIBRARIES} rpp)

add_executable(mantis_test test/unit_test.cpp)
target_link_libraries(mantis_test ${catkin_LIBRARIES} ${OpenCV_LIBRARIES} ${Eigen_LIBRARIES} poseEstimation clusterer mantisGridRenderer)

add_executable(ogl_test test/opengl_test.cpp)
target_link_libraries(ogl_test ${catkin_LIBRARIES} ${OpenCV_LIBRARIES} ${Eigen_LIBRARIES} ${OPENGL_LIBRARIES} ${GLUT_LIBRARY} ${GLEW_LIBRARY})

add_executable(mantis3 src/mantis3.cpp)
target_link_libraries(mantis3 ${catkin_LIBRARIES} ${OpenCV_LIBRARIES} ${Eigen_LIBRARIES} poseEstimation clusterer mantisGridRenderer markovPlane)

#add_executable(mantis2 src/mantis2.cpp)
#target_link_libraries(mantis2 ${catkin_LIBRARIES} ${OpenCV_LIBRARIES} ${Eigen_LIBRARIES})

#add_library(montecarlo include/mantis/MonteCarlo.cpp)
#set_target_properties(montecarlo PROPERTIES LINKER_LANGUAGE CXX)

#add_executable(mantis_client src/mantis_client.cpp)
#add_executable(mantis_server src/mantis_server.cpp)

#target_link_libraries(montecarlo ${catkin_LIBRARIES} ${OpenCV_LIBRARIES})
#target_link_libraries(mantis_client ${catkin_LIBRARIES} ${OpenCV_LIBRARIES} ${Eigen_LIBRARIES})
#target_link_libraries(mantis_server ${catkin_LIBRARIES} ${OpenCV_LIBRARIES} ${Eigen_LIBRARIES} montecarlo)

#add_dependencies(mantis_client ${PROJECT_NAME}_generate_messages_cpp)
#add_dependencies(mantis_server ${PROJECT_NAME}_generate_messages_cpp)
