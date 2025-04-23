DUT = alu
CSRC += ./csrc/tb_$(DUT).cpp
VSRC += ./vsrc/$(DUT).sv

all:
	@echo "Write this Makefile by your self."

sim:
	$(call git_commit, "sim RTL") # DO NOT REMOVE THIS LINE!!!
	verilator -Wall --trace -cc $(VSRC) --top_module $(DUT) --exe $(CSRC) --Mdir build
	make -C build -f V$(DUT).mk V$(DUT)

include ../Makefile
