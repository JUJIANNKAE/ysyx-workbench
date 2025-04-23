DUT = alu
CSRC += ./csrc/tb_$(DUT).cpp
VSRC += ./vsrc/$(DUT).sv

all: sim wave

sim:
	$(call git_commit, "sim RTL") # DO NOT REMOVE THIS LINE!!!
	verilator -Wall --trace -cc $(VSRC) --exe $(CSRC) --Mdir build
	make -C build -f V$(DUT).mk V$(DUT)
	./build/V$(DUT)

wave:
	gtkwave build/waveform.vcd &

clean::
	@rm -rf build

include ../Makefile
