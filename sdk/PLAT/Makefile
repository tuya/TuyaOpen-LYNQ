.PHONY:gccall
gccall:
	(cd project/$(TARGET)/$(CORE)/apps/$(PROJECT)/GCC && $(MAKE) V=$(V) -f Makefile all)

.PHONY:clean-gccall
clean-gccall:
	(cd project/$(TARGET)/$(CORE)/apps/$(PROJECT)/GCC && $(MAKE) -f Makefile cleanall)

.PHONY:clean-gcc
clean-gcc:
	(cd project/$(TARGET)/$(CORE)/apps/$(PROJECT)/GCC && $(MAKE) -f Makefile clean)

.PHONY:keilall
keilall:
	(cd project/$(TARGET)/$(CORE)/apps/$(PROJECT)/ARMCC && $(MAKE) V=$(V) -f Makefile all)

.PHONY:clean-keilall
clean-keilall:
	(cd project/$(TARGET)/$(CORE)/apps/$(PROJECT)/ARMCC && $(MAKE) -f Makefile cleanall)
	
.PHONY:clean-keil
clean-keil:
	(cd project/$(TARGET)/$(CORE)/apps/$(PROJECT)/ARMCC && $(MAKE) -f Makefile clean)

.PHONY:size
size:
	(cd project/$(TARGET)/$(CORE)/apps/$(PROJECT)/$(TOOLCHAIN_NAME) && $(MAKE) -f Makefile size)
.PHONY: build-unilog
build-unilog: 
	(cd project/$(TARGET)/$(CORE)/apps/$(PROJECT)/$(TOOLCHAIN_NAME) && $(MAKE) -f Makefile unilog)

