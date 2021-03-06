#!/bin/bash
# Example of how to run vpr

# Set variables
# For FPGA-Verilog ONLY 
benchmark="test_modes"
OpenFPGA_path="OPENFPGAPATHKEYWORD"
verilog_output_dirname="${benchmark}_Verilog"
verilog_output_dirpath="$vpr_path"
tech_file="${OpenFPGA_path}/fpga_flow/tech/PTM_45nm/45nm.xml"
# VPR critical inputs
template_arch_xml_file="${OpenFPGA_path}/fpga_flow/arch/template/k6_N10_sram_chain_HC_template.xml"
arch_xml_file="${OpenFPGA_path}/fpga_flow/arch/generated/k6_N10_sram_chain_HC.xml"
blif_file="${OpenFPGA_path}/fpga_flow/benchmarks/Blif/Test_Modes/$benchmark.blif" 
act_file="${OpenFPGA_path}/fpga_flow/benchmarks/Blif/Test_Modes/$benchmark.act "
verilog_reference="${OpenFPGA_path}/fpga_flow/benchmarks/Verilog/Test_Modes/$benchmark.v"
vpr_route_chan_width="200"
fpga_flow_script="${OpenFPGA_path}/fpga_flow/scripts"
ff_path="$vpr_path/VerilogNetlists/ff.v"
new_ff_path="$verilog_output_dirpath/$verilog_output_dirname/SRC/ff.v"
ff_keyword="GENERATED_DIR_KEYWORD"
ff_include_path="$verilog_output_dirpath/$verilog_output_dirname"
arch_ff_keyword="FFPATHKEYWORD"
tb_formal_ext="_formal_random_top_tb.v"
formal_postfix="_top_formal_verification"

# Remove previous designs
rm -rf $verilog_output_dirpath/$verilog_output_dirname

mkdir ${OpenFPGA_path}/fpga_flow/arch/generated

cd $fpga_flow_scripts
perl rewrite_path_in_file.pl -i $template_arch_xml_file -o $arch_xml_file
perl rewrite_path_in_file.pl -i $arch_xml_file -k $arch_ff_keyword $new_ff_path

# Move to vpr folder
cd $vpr_path

# Run VPR  
./vpr $arch_xml_file $blif_file --full_stats --activity_file $act_file --fpga_verilog --fpga_verilog_dir $verilog_output_dirpath/$verilog_output_dirname --fpga_x2p_rename_illegal_port --fpga_bitstream_generator --fpga_verilog_print_top_testbench --fpga_verilog_include_timing --fpga_verilog_include_signal_init --fpga_verilog_print_formal_verification_top_netlist --fpga_verilog_print_autocheck_top_testbench $verilog_reference --route_chan_width $vpr_route_chan_width --fpga_verilog_include_icarus_simulator --fpga_x2p_compact_routing_hierarchy --fpga_verilog_explicit_mapping --nodisp

cd $fpga_flow_scripts
perl rewrite_path_in_file.pl -i $ff_path -o $new_ff_path -k $ff_keyword $ff_include_path
cd -


