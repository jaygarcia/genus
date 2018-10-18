# ESP-IDF Makefile for game project


PROJECT_NAME=Genus
<<<<<<< HEAD
EXTRA_COMPONENT_DIRS=${PROJECT_PATH}/creative-engine ${PROJECT_PATH}/src ${PROJECT_PATH}/src/GameState
=======

# COMPONENT_PATH=./src



EXTRA_COMPONENT_DIRS=creative-engine src 

>>>>>>> c19557ee3040a46009a7d79262f6bb43e415b984
include $(IDF_PATH)/make/project.mk

# Temporary until RCOMP is folded into the music/sfx workflow
gen_sound_headers: gen_music_headers gen_sfx_headers FORCE

gen_music_headers:
	echo "Generating music headers"
	cd resources/music && ./gen_header.sh

gen_sfx_headers:
	echo "Generating sound effects headers"
	cd resources/sound_effects && ./gen_header.sh


rcomp: FORCE
	echo "Building rcomp"
	cd ./creative-engine/tools && make

resources: rcomp FORCE
	echo "Compiling resources"
	cd src && ../creative-engine/tools/rcomp Resources.r

docs:
	(cd doxygen && ./build.sh)

clean-docs:
	rm -rf docs
	
FORCE:


