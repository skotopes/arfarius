BUILD_DIR := ./build

OS := $(shell uname -s)
ifeq ($(OS), Linux)
NPROCS := $(shell grep -c ^processor /proc/cpuinfo)
else ifeq ($(OS), Darwin)
NPROCS := $(shell sysctl -n hw.ncpu)
endif

ifneq (, $(shell which ninja))
GENERATOR = -G Ninja
endif

CMAKE_BUILD_TYPE := "Release"
DOCKER_PLATFORM := "linux/arm64"

all: always $(BUILD_DIR)/Makefile 
	cmake --build $(BUILD_DIR) -j $(NPROCS)

$(BUILD_DIR)/Makefile: $(BUILD_DIR)/build_type
	cd $(BUILD_DIR) && cmake .. -D CMAKE_BUILD_TYPE=$(CMAKE_BUILD_TYPE) $(GENERATOR)

$(BUILD_DIR)/build_type:

.PHONY: always
always:
	mkdir -p $(BUILD_DIR)
	grep $(CMAKE_BUILD_TYPE) $(BUILD_DIR)/build_type || echo $(CMAKE_BUILD_TYPE) > $(BUILD_DIR)/build_type

.PHONY: clean
clean:
	rm -rf $(BUILD_DIR)

.PHONY: macdeploy 
macdeploy: all
	rm -rf $(BUILD_DIR)/disk_image || true
	mkdir $(BUILD_DIR)/disk_image
	ln -s /Applications $(BUILD_DIR)/disk_image/Applications
	rsync -av $(BUILD_DIR)/Arfarius.app/ $(BUILD_DIR)/disk_image/Arfarius.app/
	macdeployqt $(BUILD_DIR)/disk_image/Arfarius.app -verbose=1
	@if [ -n "$$SIGNING_KEY" ]; then \
		xattr -cr $(BUILD_DIR)/disk_image/Arfarius.app; \
		codesign --force -s "$(SIGNING_KEY)" --deep -v $(BUILD_DIR)/disk_image/Arfarius.app; \
	fi
	hdiutil create -volname Arfarius -srcfolder $(BUILD_DIR)/disk_image -ov -format UDZO $(BUILD_DIR)/Arfarius.dmg
	open $(BUILD_DIR)

