/********************************************************************
 * This file includes functions to build fabric dependent bitstream
 *******************************************************************/
#include <time.h>
#include <string>
#include <algorithm>

#include "vtr_assert.h"
#include "util.h"
#include "fpga_x2p_naming.h"

#include "bitstream_manager_utils.h"
#include "build_fabric_bitstream.h"

/********************************************************************
 * This function will walk through all the configurable children under a module
 * in a recursive way, following a Depth-First Search (DFS) strategy
 * For each configuration child, we use its instance name as a key to spot the 
 * configuration bits in bitstream manager.
 * Note that it is guarentee that the instance name in module manager is 
 * consistent with the block names in bitstream manager
 * We use this link to reorganize the bitstream in the sequence of memories as we stored
 * in the configurable_children) and configurable_child_instances() of each module of module manager 
 *******************************************************************/
static 
void rec_build_module_fabric_dependent_bitstream(const BitstreamManager& bitstream_manager,
                                                 const ConfigBlockId& parent_block,
                                                 const ModuleManager& module_manager,
                                                 const ModuleId& parent_module,
                                                 std::vector<ConfigBitId>& fabric_bitstream) {

  /* Depth-first search: if we have any children in the parent_block, 
   * we dive to the next level first! 
   */
  if (0 < bitstream_manager.block_children(parent_block).size()) {
    for (size_t child_id = 0; child_id < module_manager.configurable_children(parent_module).size(); ++child_id) {
      ModuleId child_module = module_manager.configurable_children(parent_module)[child_id]; 
      size_t child_instance = module_manager.configurable_child_instances(parent_module)[child_id]; 
      /* Get the instance name and ensure it is not empty */
      std::string instance_name = module_manager.instance_name(parent_module, child_module, child_instance);
       
      /* Find the child block that matches the instance name! */ 
      ConfigBlockId child_block = bitstream_manager.find_child_block(parent_block, instance_name); 
      /* We must have one valid block id! */
      if (true != bitstream_manager.valid_block_id(child_block))
      VTR_ASSERT(true == bitstream_manager.valid_block_id(child_block));

      /* Go recursively */
      rec_build_module_fabric_dependent_bitstream(bitstream_manager, child_block,
                                                  module_manager, child_module,
                                                  fabric_bitstream);
    }
    /* Ensure that there should be no configuration bits in the parent block */
    VTR_ASSERT(0 == bitstream_manager.block_bits(parent_block).size());
  }

  /* Note that, reach here, it means that this is a leaf node. 
   * We add the configuration bits to the fabric_bitstream,
   * And then, we can return
   */
  for (const ConfigBitId& config_bit : bitstream_manager.block_bits(parent_block)) {
    fabric_bitstream.push_back(config_bit);
  }
}

/********************************************************************
 * A top-level function re-organizes the bitstream for a specific 
 * FPGA fabric, where configuration bits are organized in the sequence
 * that can be directly loaded to the FPGA configuration protocol.
 * Support:
 * 1. Configuration chain
 * 2. Memory decoders
 * This function does NOT modify the bitstream database
 * Instead, it builds a vector of ids for configuration bits in bitstream manager
 *
 * This function can be called ONLY after the function build_device_bitstream() 
 * Note that this function does NOT decode bitstreams from circuit implementation
 * It was done in the function build_device_bitstream() 
 *******************************************************************/
std::vector<ConfigBitId> build_fabric_dependent_bitstream(const BitstreamManager& bitstream_manager,
                                                          const ModuleManager& module_manager) {
  std::vector<ConfigBitId> fabric_bitstream; 

  vpr_printf(TIO_MESSAGE_INFO, "\nBuilding fabric dependent bitstream...\n");

  /* Start time count */
  clock_t t_start = clock();

  /* Get the top module name in module manager, which is our starting point */
  std::string top_module_name = generate_fpga_top_module_name();
  ModuleId top_module = module_manager.find_module(top_module_name);
  VTR_ASSERT(true == module_manager.valid_module_id(top_module));

  /* Find the top block in bitstream manager, which has not parents */
  std::vector<ConfigBlockId> top_block = find_bitstream_manager_top_blocks(bitstream_manager);
  /* Make sure we have only 1 top block and its name matches the top module */
  VTR_ASSERT(1 == top_block.size());
  VTR_ASSERT(0 == top_module_name.compare(bitstream_manager.block_name(top_block[0])));

  rec_build_module_fabric_dependent_bitstream(bitstream_manager, top_block[0],
                                              module_manager, top_module, 
                                              fabric_bitstream);

  /* Time-consuming sanity check: Uncomment these codes only for debugging!!!
   * Check which configuration bits are not touched 
   */
  /*
  for (const ConfigBitId& config_bit : bitstream_manager.bits()) {
    std::vector<ConfigBitId>::iterator it = std::find(fabric_bitstream.begin(), fabric_bitstream.end(), config_bit);
    if (it == fabric_bitstream.end()) {
      std::vector<ConfigBlockId> block_hierarchy = find_bitstream_manager_block_hierarchy(bitstream_manager, bitstream_manager.bit_parent_block(config_bit)); 
      std::string block_hierarchy_name;
      for (const ConfigBlockId& temp_block : block_hierarchy) {
        block_hierarchy_name += std::string("/") + bitstream_manager.block_name(temp_block);
      }
      vpr_printf(TIO_MESSAGE_INFO, 
                 "bit (parent_block = %s) is not touched!\n", 
                 block_hierarchy_name.c_str());
    }
  }
   */

  /* Ensure our fabric bitstream is in the same size as device bistream */
  VTR_ASSERT(bitstream_manager.bits().size() == fabric_bitstream.size());

  /* End time count */
  clock_t t_end = clock();

  float run_time_sec = (float)(t_end - t_start) / CLOCKS_PER_SEC;
  vpr_printf(TIO_MESSAGE_INFO, 
             "Building fabric dependent bitstream took %g seconds\n", 
             run_time_sec);  
  
  return fabric_bitstream;
}
